//
// Created by Administrator on 2026/2/27.
//
module Generator;

import std;
import Token;
import keyword;
import Parser;
import aux;

size_t GenClass::exprCnt = 1;

using astClass = mlc::parser::AbstractSyntaxTree;


GenClass::exprResult GenClass::ExpressionExpand(const sPtr<ast::Expression> &_expression,
                                                const sPtr<type::CompileType> &_type) {
    // 1. 获取表达式的 LLVM 类型 (调用你之前写的 TypeToLLVM)
    const auto type = TypeToLLVM(_expression->GetType());
    if (auto *const constPtr = _expression->GetConstValue()) {
        return {type, constPtr->Value, ""};
    }
    if (auto *const enumPtr = _expression->GetEnumValue()) {
        return {type, std::to_string(enumPtr->Index), ""};
    }
    if (auto *const varPtrT = _expression->GetVariable()) {
        const auto varPtr = *varPtrT;
        std::string varAddr = std::format("%{}", varPtr->Name);
        std::string llvmType = TypeToLLVM(varPtr->VarType);

        // 💡 物理分流：判断是否为“复合类型”（数组、指针、结构体等）
        auto isComplex = [](const sPtr<type::CompileType> &cType) {
            const auto isArray = std::get_if<type::ArrayType>(&*cType) != nullptr;
            const auto isStruct = std::get_if<type::StructDefinition>(&*cType) != nullptr;
            if (const auto *const ptr = std::get_if<type::PointerType>(&*cType)) {
                const auto baseType = ptr->BaseType;
                return std::get_if<type::ArrayType>(&*baseType) || std::get_if<type::StructDefinition>(&*baseType);
            }
            return isArray || isStruct;
        };
        if (isComplex(varPtr->VarType)) {
            return exprResult{
                TypeToLLVM(varPtr->VarType),
                varAddr,
                "",
                true
            };
        }
        std::string tempReg = std::format("%{}", exprCnt++);
        std::string loadCode = std::format("{} = load {}, ptr {}, align 4\n",
                                           tempReg, llvmType, varAddr);
        return exprResult{
            TypeToLLVM(varPtr->VarType),
            tempReg,
            loadCode
        };
    }
    if (auto *const compPtr = _expression->GetCompositeExpression()) {
        auto isLogic = [](const ast::BaseOperator op) {
            return op == ast::BaseOperator::And || op == ast::BaseOperator::Or || op == ast::BaseOperator::Not || op ==
                   ast::BaseOperator::Equal || op == ast::BaseOperator::NotEqual || op == ast::BaseOperator::Greater ||
                   op == ast::BaseOperator::Less || op == ast::BaseOperator::GreaterEqual || op ==
                   ast::BaseOperator::LessEqual;
        };
        auto &[operators,components,opFirst] = **compPtr;
        if (components.empty()) {
            ErrorPrintln("Error: Composite expression has no components.\n");
            std::exit(-1);
        }
        if (components.size() >= 2 && (components[1]->GetMemberAccess() || operators[0] ==
                                       ast::BaseOperator::Subscript)) {
            return MemberAccessExpression(_expression, false);
        }
        if (opFirst) {
            if (const auto var = *(components[0]->GetVariable())) {
                if (operators[0] == ast::BaseOperator::AddressOf) {
                    return exprResult{
                        "ptr",
                        std::format("%{}", var->Name), ""
                    };
                }
                return exprResult{
                    TypeToLLVM(components[0]->GetType()),
                    std::format("%{}", var->Name), ""
                };
            }
        }
        auto workingExpr = components | std::views::transform([](const sPtr<ast::Expression> &expr) {
            return ExpressionExpand(expr);
        }) | std::ranges::to<std::vector<exprResult> >();
        auto isLogicResult = std::ranges::all_of(operators, isLogic);
        auto result = GradientExpression(workingExpr, operators);
        if (isLogicResult) {
            result.llvmType = "i1"; // 逻辑表达式结果类型为 i1
        }
        return result;
    }
    if (auto *const functionCallPtr = _expression->GetFunctionCall()) {
        if (functionCallPtr->get()->FunctionDecl->IsTypeConvert) {
            auto func = *functionCallPtr;
            return TypeConvert(func->Arguments[0], func->FunctionDecl->ReturnType);
        }
        const auto [isCopyResult, llvmType, resultVar, callCode] = FunctionCall(*functionCallPtr);
        return exprResult{
            llvmType,
            resultVar,
            callCode,
            isCopyResult
        };
    }
    if (auto *const initListPtr = _expression->GetInitializerList()) {
        return InitializerListExpression(*initListPtr, _type);
    }
    return {"", "", ""};
}

std::string GenClass::ConstExpressionExpand(const sPtr<ast::Type::CompileType> &_type,
                                            const sPtr<ast::Expression> &_expression, bool _isCondition) {
    if (!_expression || !_expression->Storage) return "";
    if (const auto *constVal = _expression->GetConstValue()) {
        std::string typeStr = GetTypeName(*_expression->GetType());
        if (_isCondition) {
            return std::format("{}", constVal->Value);
        }
        return std::format("{} {}", typeStr, constVal->Value);
    }
    if (const auto *initListPtr = _expression->GetInitializerList()) {
        return ConstInitializerListExpression(*initListPtr, _type);
    }
    if (const auto *compExprPtr = _expression->GetCompositeExpression()) {
        const auto &compExpr = *compExprPtr;
        if (compExpr->Components.empty()) return "";
        std::vector<std::string> workingExpr;
        for (const auto &comp: compExpr->Components) {
            workingExpr.push_back(ConstExpressionExpand(comp->GetType(), comp));
        }
        std::vector<ast::BaseOperator> workingOps = compExpr->Operators;
        for (size_t priority = 1; priority <= 12; ++priority) {
            for (size_t i = 0; i < workingOps.size();) {
                if (ast::OperatorPriority.at(workingOps[i]) == priority) {
                    std::string left = workingExpr[i];
                    std::string right = workingExpr[i + 1];
                    auto compileType = compExpr->Components[i]->GetType();
                    auto *type = std::get_if<type::BaseType>(&*compileType);
                    std::string opSymbol = OperatorToIR(type, workingOps[i]);
                    std::string combined = std::format("{} ({}, {})", opSymbol, left, right);
                    workingExpr[i] = combined;
                    workingExpr.erase(workingExpr.begin() + i + 1);
                    workingOps.erase(workingOps.begin() + i);
                } else {
                    ++i;
                }
            }
        }
        return workingExpr[0];
    }
    if (const auto *funcCallPtr = _expression->GetFunctionCall()) {
        const auto &funcCall = *funcCallPtr;
        auto functionName = funcCall->FunctionDecl ? funcCall->FunctionDecl->Name : "unknown_func";
        const auto *sourceType = std::get_if<type::BaseType>(&*funcCall->Arguments[0]->GetType());
        const auto *convertFuncType = &*std::ranges::find_if(type::BaseTypes, [&](const type::BaseType &baseType) {
            return baseType.Name == functionName;
        });
        std::string castOp = determineCastOperator(sourceType, convertFuncType);
        return std::format("{} ({} {})", castOp, GetTypeName(*convertFuncType),
                           ConstExpressionExpand(funcCall->Arguments[0]->GetType(), funcCall->Arguments[0]));
    }
    if (const auto &enumPtr = _expression->GetEnumValue()) {
        if (_isCondition) {
            return std::format("{}", enumPtr->Index);
        }
        return std::format("{} {}", "i32", enumPtr->Index);
    }

    ErrorPrintln("Error: Expression is not a constant expression and cannot be evaluated at compile time.\n");
    std::exit(-1);
}


GenClass::exprResult GenClass::TripleExpression(const expr &_left, const expr &_right, ast::BaseOperator _op) {
    const auto leftResult = ExpressionExpand(_left);
    const auto rightResult = ExpressionExpand(_right);
    auto code = std::format("{}\n{}\n", leftResult.code, rightResult.code);

    std::string llvmOp = OperatorToIR(std::get_if<type::BaseType>(&*_left->GetType()), _op);

    auto result = exprResult{
        leftResult.llvmType, std::format("%{}", exprCnt++),
        code + std::format("{} = {} {} {}, {}\n", std::format("%{}", exprCnt),
                           llvmOp, leftResult.llvmType, leftResult.resultVar, rightResult.resultVar)
    };

    return result;
}

GenClass::exprResult GenClass::TripleExpression(const exprResult &_left, const exprResult &_right,
                                                ast::BaseOperator _op) {
    std::string targetReg = std::format("%tr{}", exprCnt++);
    const std::string combinedCode = _left.code + _right.code;
    std::string llvmOp = OperatorToIR(_left.llvmType, _op);
    const std::string currentInstr = std::format("{} = {} {} {}, {}\n",
                                                 targetReg, llvmOp, _left.llvmType,
                                                 _left.resultVar, _right.resultVar);
    return exprResult{
        _left.llvmType,
        targetReg,
        combinedCode + currentInstr
    };
}

GenClass::exprResult GenClass::BinaryExpression(const expr &_expr, ast::BaseOperator _op) {
    using op = ast::BaseOperator;
    auto [llvmType, resultVar, code,_] = ExpressionExpand(_expr);
    size_t newReg = exprCnt++;
    switch (_op) {
        case op::BitNot:
        case op::Not: {
            const auto instr = std::format(
                "{} = xor {} {}, -1\n",
                std::format("%{}", newReg),
                llvmType,
                resultVar
            );
            return exprResult{
                llvmType,
                std::format("%{}", newReg),
                code + instr // 包含操作数的代码 + 自己的取反指令
            };
        }
        case op::AddressOf: {
            const auto instr = std::format(
                "{} = ptrtoint {} {} to ptr\n",
                std::format("%{}", newReg),
                llvmType,
                resultVar
            );
            return exprResult{
                "ptr",
                std::format("%{}", newReg),
                code + instr
            };
        }
        case op::Dereference: {
            const auto instr = std::format(
                "{} = load {}, ptr {}\n",
                std::format("%{}", newReg),
                llvmType,
                resultVar
            );
            return exprResult{
                llvmType,
                std::format("%{}", newReg),
                code + instr
            };
        }
        default:
            ErrorPrintln("Compiler Internal Error");
            std::exit(-1);
    }
}


GenClass::exprResult GenClass::GradientExpression(const std::vector<exprResult> &_expr,
                                                  const std::vector<ast::BaseOperator> &_ops) {
    std::vector<exprResult> workingExpr = _expr;
    std::vector<ast::BaseOperator> workingOps = _ops;
    for (size_t priority = 1; priority <= 12; ++priority) {
        for (size_t i = 0; i < workingOps.size();) {
            if (ast::OperatorPriority.at(workingOps[i]) == priority) {
                const exprResult &left = workingExpr[i];
                const exprResult &right = workingExpr[i + 1];
                const ast::BaseOperator op = workingOps[i];
                exprResult combined;
                combined = TripleExpression(left, right, op);
                workingExpr[i] = std::move(combined); // 更新左侧位置
                workingExpr.erase(workingExpr.begin() + i + 1); // 移除右侧操作数
                workingOps.erase(workingOps.begin() + i); // 移除已处理的运算符
            } else {
                ++i;
            }
        }
    }
    return workingExpr[0];
}

GenClass::exprResult GenClass::LeftExpressionExpand(const expr &_expr) {
    if (const auto *const vPtr = _expr->GetVariable(); vPtr != nullptr) {
        const auto type = TypeToLLVM((*vPtr)->VarType);
        const auto name = std::format("%{}", (*vPtr)->Name);

        return exprResult{
            type,
            name,
            ""
        };
    }
    if (const auto *const compPtr = _expr->GetCompositeExpression(); compPtr != nullptr) {
        return MemberAccessExpression(_expr, true);
    }
    ErrorPrintln("Error: Expression is not a valid left-hand side expression.\n");
    std::exit(-1);
}

GenClass::exprResult GenClass::TypeConvert(const expr &_expr, const sPtr<type::CompileType> &_targetType) {
    const auto nowType = TypeToLLVM(_expr->GetType());
    const auto nowTypeSize = type::GetSize(_expr->GetType());
    auto expr = ExpressionExpand(_expr);
    const auto targetType = TypeToLLVM(_targetType);
    const auto targetTypeSize = type::GetSize(_targetType);

    auto getReg = []() {
        return std::format("%ty{}", exprCnt++);
    };

    if (expr.llvmType == targetType) {
        return expr; // 类型相同，无需转换
    }

    auto isIntegerType = [](const std::string_view type) {
        return type.starts_with('i') || type.starts_with('u');
    };
    auto isFloatType = [](const std::string_view type) {
        return type.starts_with('f');
    };

    if (type::IsType<type::ArrayType>(_expr->GetType())) {
        if (!type::IsType<type::PointerType>(_targetType)) {
            ErrorPrintln("Error: Cannot convert array type to non-pointer type.\n");
            std::exit(-1);
        }
        auto reg = getReg();
        std::string code = std::format("{} = getelementptr inbounds {}, ptr {}, i64 0, i64 0\n",
                                       reg, nowType, expr.resultVar);
        return exprResult{targetType, reg, expr.code + code};
    }

    if (isIntegerType(nowType) && isIntegerType(targetType)) {
        if (nowTypeSize == targetTypeSize) return {targetType, expr.resultVar, expr.code}; // 符号互转是 No-op

        std::string op = (nowTypeSize < targetTypeSize)
                             ? (nowType[0] == 'i' ? "sext" : "zext") // 假设你的 nowType 字符串能区分有无符号
                             : "trunc";
        auto reg = getReg();
        std::string code = std::format("{} = {} {} {} to {}\n", reg, op, nowType, expr.resultVar, targetType);
        return exprResult{targetType, reg, expr.code + code};
    }

    if (isFloatType(nowType) && isFloatType(targetType)) {
        std::string op = (nowTypeSize < targetTypeSize) ? "fpext" : "fptrunc";
        auto reg = getReg();
        std::string code = std::format("{} = {} {} {} to {}\n", reg, op, nowType, expr.resultVar, targetType);
        return exprResult{targetType, reg, expr.code + code};
    }

    if (isIntegerType(nowType) && isFloatType(targetType)) {
        std::string op = (nowType[0] == 'i') ? "sitofp" : "uitofp";
        auto reg = getReg();
        std::string code = std::format("{} = {} {} {} to {}\n", reg, op, nowType, expr.resultVar, targetType);
        return exprResult{targetType, reg, expr.code + code};
    }

    if (isFloatType(nowType) && isIntegerType(targetType)) {
        std::string op = (targetType[0] == 'i') ? "fptosi" : "fptoui";
        auto reg = getReg();
        std::string code = std::format("{} = {} {} {} to {}\n", reg, op, nowType, expr.resultVar, targetType);
        return exprResult{targetType, reg, expr.code + code};
    }

    ErrorPrintln("Error: Unsupported type conversion from {} to {}.\n", nowType, targetType);
    std::exit(-1);
}
