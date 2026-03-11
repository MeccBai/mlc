//
// Created by Administrator on 2026/3/1.
//
module Generator;

import std;
import Token;
import keyword;
import Parser;
import aux;


std::string GenClass::GlobalVariable(const sPtr<ast::VariableStatement> &_variable) {
    if (const auto initExpr = _variable->Initializer; initExpr != nullptr && ConstExpressionCheck(initExpr)) {
        auto value = ConstExpressionExpand(_variable->VarType, initExpr);
        return std::format("@{} = global {}\n", _variable->Name, value);
    }
    ErrorPrintln("Error: Global variable '{}' must be initialized with a constant expression.\n", _variable->Name);
    std::exit(-1);
}

size_t gen::GetAlignment(const sPtr<type::CompileType>& t) {
    const auto s = type::GetSize(t);
    if (s <= 1) return 1;
    if (s <= 2) return 2;
    if (s <= 4) return 4;
    if (s <= 8) return 8;
    return 16; // 结构体或大数组通常对齐到 16
}
std::string GenClass::LocalVariable(const sPtr<ast::VariableStatement>& _variable) {
    auto llvmType = TypeToLLVM(_variable->VarType);
    auto regName = _variable->Name;
    exprCnt++;
    auto align = GetAlignment(_variable->VarType);
    std::string returnCode = std::format("%{} = alloca {}, align {}\n", regName, llvmType, align);
    if (_variable->Initializer) {
        auto [exprType, resultVar, code, isCopyResult] = ExpressionExpand(_variable->Initializer, _variable->VarType);
        if (exprType == "list") {
            auto argTemp = "%" + regName;
            returnCode += std::vformat(resultVar, std::make_format_args(argTemp));
            returnCode += code;
        }
        else if (isCopyResult) {
            returnCode += code;
            returnCode += std::format(
                "call void @{}(ptr %{}, ptr {}, i64 {}, i1 false)\n",llvmCopy,
                regName, resultVar, type::GetSize(_variable->VarType)
            );
        }
        else {
            returnCode += code;
            returnCode += std::format(
                "store {} {}, ptr %{}, align {}\n",
                exprType, resultVar, regName, align
            );
        }
    }
    return returnCode;
}
