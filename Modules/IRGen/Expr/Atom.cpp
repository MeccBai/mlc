//
// Created by Administrator on 2026/3/12.
//
//
module Generator;

import std;
import Token;
import keyword;
import Parser;
import aux;
using astClass = mlc::parser::AbstractSyntaxTree;


GenClass::exprResult gen::variableExpand( const sPtr<ast::VariableStatement>& _var) {
    const auto varAddr = std::format("%{}", _var->Name);
    const auto llvmType = GenClass::TypeToLLVM(_var->VarType);
    auto isComplex = [](const sPtr<type::CompileType> &cType) {
        const auto isArray = type::GetType<type::ArrayType>(cType) != nullptr;
        const auto isStruct = type::GetType<type::StructDefinition>(cType) != nullptr && type::GetSize(cType) > 8;
        if (const auto *const ptr = type::GetType<type::PointerType>(cType)) {
            const auto baseType = ptr->Dereference();
            return type::GetType<type::ArrayType>(baseType) ||
                type::GetType<type::StructDefinition>(baseType);
        }
        return isArray || isStruct;
    };
    if (isComplex(_var->VarType)) {
        return GenClass::exprResult{
            "ptr",
            varAddr,
            "",
            true
        };
    }
    const auto tempReg = std::format("%{}", GenClass::exprCnt++);
    const auto loadCode = std::format("{} = load {}, ptr {}, align 4\n",
                                       tempReg, llvmType, varAddr);
    return GenClass::exprResult{
        GenClass::TypeToLLVM(_var->VarType),
        tempReg,
        loadCode
    };
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
        const auto code = std::format("{} = getelementptr inbounds {}, ptr {}, i64 0, i64 0\n",
                                      reg, nowType, expr.resultVar);
        return exprResult{targetType, reg, expr.code + code};
    }

    if (isIntegerType(nowType) && isIntegerType(targetType)) {
        if (nowTypeSize == targetTypeSize) return {targetType, expr.resultVar, expr.code}; // 符号互转是 No-op

        const auto *const op = (nowTypeSize < targetTypeSize)
                                   ? (nowType[0] == 'i' ? "sext" : "zext") // 假设你的 nowType 字符串能区分有无符号
                                   : "trunc";
        auto reg = getReg();
        const auto code = std::format("{} = {} {} {} to {}\n", reg, op, nowType, expr.resultVar, targetType);
        return exprResult{targetType, reg, expr.code + code};
    }

    if (isFloatType(nowType) && isFloatType(targetType)) {
        const auto *const op = (nowTypeSize < targetTypeSize) ? "fpext" : "fptrunc";
        const auto reg = getReg();
        const auto code = std::format("{} = {} {} {} to {}\n", reg, op, nowType, expr.resultVar, targetType);
        return exprResult{targetType, reg, expr.code + code};
    }

    if (isIntegerType(nowType) && isFloatType(targetType)) {
        const auto *const op = (nowType[0] == 'i') ? "sitofp" : "uitofp";
        const auto reg = getReg();
        const auto code = std::format("{} = {} {} {} to {}\n", reg, op, nowType, expr.resultVar, targetType);
        return exprResult{targetType, reg, expr.code + code};
    }

    if (isFloatType(nowType) && isIntegerType(targetType)) {
        const auto *const op = (targetType[0] == 'i') ? "fptosi" : "fptoui";
        const auto reg = getReg();
        const auto code = std::format("{} = {} {} {} to {}\n", reg, op, nowType, expr.resultVar, targetType);
        return exprResult{targetType, reg, expr.code + code};
    }

    ErrorPrintln("Error: Unsupported type conversion from {} to {}.\n", nowType, targetType);
    std::exit(-1);
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
                    const auto left = workingExpr[i];
                    const auto right = workingExpr[i + 1];
                    const auto compileType = compExpr->Components[i]->GetType();
                    auto *type = type::GetType<type::BaseType>(compileType);
                    const auto opSymbol = OperatorToIR(type, workingOps[i]);
                    const auto combined = std::format("{} ({}, {})", opSymbol, left, right);
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
        if (!funcCall->FunctionDecl->IsTypeConvert) {
            ErrorPrintln("Error: Only type conversion function calls can be evaluated in constant expressions.\n");
            std::exit(-1);
        }
        auto functionName = funcCall->FunctionDecl ? funcCall->FunctionDecl->Name : "unknown_func";
        const auto *sourceType = std::get_if<type::BaseType>(&*funcCall->Arguments[0]->GetType());
        const auto *convertFuncType = &*std::ranges::find_if(type::BaseTypes, [&](const type::BaseType &baseType) {
            return baseType.Name == functionName;
        });
        const auto castOp = determineCastOperator(sourceType, convertFuncType);
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

