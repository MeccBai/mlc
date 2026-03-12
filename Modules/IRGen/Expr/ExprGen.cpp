//
// Created by Administrator on 2026/2/27.
//
module Generator;

import std;
import Token;
import keyword;
import Parser;
import aux;
using astClass = mlc::parser::AbstractSyntaxTree;

size_t GenClass::exprCnt = 1;

GenClass::exprResult deference(const sPtr<ast::Expression> &_expr) {
    const auto regName = std::format("%dr{}", GenClass::exprCnt);
    auto returnType = _expr->GetType();
    if (const auto *ptrType = type::GetType<type::PointerType>(returnType)) {
        returnType = ptrType->Dereference();
        if (type::IsType<type::StructDefinition>(returnType)) {
            ErrorPrintln("Error: use -> to access struct\n");
            std::exit(-1);
        }
    }
    const auto llvmType = GenClass::TypeToLLVM(returnType);
    const auto &var = **_expr->GetVariable();
    const auto instruction = std::format("%{} = load ptr, ptr %{}, align 8\n", GenClass::exprCnt, var.Name);
    const auto finalInstruction = instruction + std::format("{} = load {}, ptr %{}, align 4\n", regName, llvmType,
                                                            GenClass::exprCnt++);
    return GenClass::exprResult{llvmType, regName, finalInstruction};
}

GenClass::exprResult compositeExpand(const sPtr<ast::Expression> &_expression) {
    auto compPtr = *_expression->GetCompositeExpression();
    auto isLogic = [](const ast::BaseOperator op) {
        return op == ast::BaseOperator::And || op == ast::BaseOperator::Or || op == ast::BaseOperator::Not || op ==
               ast::BaseOperator::Equal || op == ast::BaseOperator::NotEqual || op == ast::BaseOperator::Greater ||
               op == ast::BaseOperator::Less || op == ast::BaseOperator::GreaterEqual || op ==
               ast::BaseOperator::LessEqual;
    };
    auto &[operators,components,opFirst] = *compPtr;
    if (components.empty()) {
        ErrorPrintln("Error: Composite expression has no components.\n");
        std::exit(-1);
    }
    if (components.size() >= 2 && (components[1]->GetMemberAccess() || operators[0] ==
                                   ast::BaseOperator::Subscript)) {
        return GenClass::MemberAccessExpression(_expression, false);
    }
    if (opFirst) {
        return GenClass::BinaryExpression(components[0], operators[0]);
    }
    const auto workingExpr = components | std::views::transform([](const sPtr<ast::Expression> &expr) {
        return GenClass::ExpressionExpand(expr);
    }) | std::ranges::to<std::vector<GenClass::exprResult> >();
    const auto isLogicResult = std::ranges::all_of(operators, isLogic);
    auto result = GenClass::GradientExpression(workingExpr, operators);
    if (isLogicResult) {
        result.llvmType = "i1"; // 逻辑表达式结果类型为 i1
    }
    return result;
}

GenClass::exprResult GenClass::ExpressionExpand(const sPtr<ast::Expression> &_expression,
                                                const sPtr<type::CompileType> &_type) {
    if (!_expression || !_expression->Storage) return {"", "", ""};


    return std::visit(overloaded{
        [&](const type::sPtr<ast::InitializerList> &initList) {
            return InitializerListExpression(initList, _type);
        },
        [&](const ast::ConstValue &constVal) -> exprResult {
            const auto llvmTypeName = TypeToLLVM(_expression->GetType());
            return {llvmTypeName, constVal.Value, ""};
        },
        [&](const ast::EnumValue &enumVal) -> exprResult {
            const auto llvmTypeName = TypeToLLVM(_expression->GetType());
            return {llvmTypeName, std::to_string(enumVal.Index), ""};
        },
        [&](const type::sPtr<ast::Variable> &var) {
            return variableExpand(var);
        },
        [&](const type::sPtr<ast::CompositeExpression> &) {
            return compositeExpand(_expression);
        },
        [&](const type::sPtr<ast::FunctionCall> &funcCall) -> exprResult {
            if (funcCall->FunctionDecl->IsTypeConvert) {
                return TypeConvert(funcCall->Arguments[0], funcCall->FunctionDecl->ReturnType);
            }
            const auto [isCopyResult, resType, resVar, code] = FunctionCall(funcCall);
            return {resType, resVar, code, isCopyResult};
        },
        [&](const auto &) -> exprResult {
            return {"", "", ""};
        }
    }, *_expression->Storage);
}

GenClass::exprResult GenClass::TripleExpression(const expr &_left, const expr &_right, ast::BaseOperator _op) {
    const auto leftResult = ExpressionExpand(_left);
    const auto rightResult = ExpressionExpand(_right);
    const auto code = std::format("{}\n{}\n", leftResult.code, rightResult.code);

    const auto llvmOp = OperatorToIR(std::get_if<type::BaseType>(&*_left->GetType()), _op);

    const auto result = exprResult{
        leftResult.llvmType, std::format("%{}", exprCnt++),
        code + std::format("{} = {} {} {}, {}\n", std::format("%{}", exprCnt),
                           llvmOp, leftResult.llvmType, leftResult.resultVar, rightResult.resultVar)
    };

    return result;
}

GenClass::exprResult GenClass::TripleExpression(const exprResult &_left, const exprResult &_right,
                                                ast::BaseOperator _op) {
    const auto targetReg = std::format("%tr{}", exprCnt++);
    const auto combinedCode = _left.code + _right.code;
    const auto llvmOp = OperatorToIR(_left.llvmType, _op);
    const auto currentInstr = std::format("{} = {} {} {}, {}\n",
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
            const auto *var = _expr->GetVariable();
            if (!var) {
                ErrorPrintln("Error: Address-of operator '@' can only be applied to variables.");
                std::exit(-1);
            }
            return exprResult{
                "ptr",
                std::format("%{}", (*var)->Name), ""
            };
        }
        case op::Dereference: {
            return deference(_expr);
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
                const auto &left = workingExpr[i];
                const auto &right = workingExpr[i + 1];
                const auto op = workingOps[i];
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

