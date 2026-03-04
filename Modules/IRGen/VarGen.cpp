//
// Created by Administrator on 2026/3/1.
//
module Generator;

import std;
import Token;
import std;
import keyword;
import Parser;
import aux;


std::string GenClass::GlobalVariable(const sPtr<ast::VariableStatement> &_variable) {
    if (const auto initExpr = _variable->Initializer; initExpr != nullptr && ConstExpressionCheck(initExpr)) {
        auto value = ConstExpressionExpand(_variable->VarType, initExpr);
        return std::format("@{} = global {}", _variable->Name, value);
    }
    ErrorPrintln("Error: Global variable '{}' must be initialized with a constant expression.\n", _variable->Name);
    std::exit(-1);
}

size_t GetAlignment(const sPtr<type::CompileType>& t) {
    size_t s = type::GetSize(t);
    if (s <= 1) return 1;
    if (s <= 2) return 2;
    if (s <= 4) return 4;
    if (s <= 8) return 8;
    return 16; // 结构体或大数组通常对齐到 16
}
std::string GenClass::LocalVariable(const sPtr<ast::VariableStatement>& _variable) {
    auto llvmType = TypeToLLVM(_variable->VarType);
    auto regName = _variable->Name; // 变量名
    exprCnt++;
    auto align = GetAlignment(_variable->VarType);
    std::string code = std::format("%{} = alloca {}, align {}\n", regName, llvmType, align);
    if (_variable->Initializer) {
        auto initRes = ExpressionExpand(_variable->Initializer, _variable->VarType);
        if (initRes.llvmType == "list") {
            auto argTemp = "%" + regName;
            code += std::vformat(initRes.resultVar, std::make_format_args(argTemp));
            code += initRes.code;
        }
        else if (initRes.isCopyResult) {
            // 场景 B: 结构体/大对象拷贝 (memcpy)
            code += initRes.code;
            code += std::format(
                "  call void @llvm.memcpy.p0.p0.i64(ptr %{}, ptr {}, i64 {}, i1 false)\n",
                regName, initRes.resultVar, type::GetSize(_variable->VarType)
            );
        }
        else {
            // 场景 C: 普通标量赋值 (i32 a = 1 + 2)
            code += initRes.code;
            code += std::format(
                "  store {} {}, ptr %{}, align {}\n",
                initRes.llvmType, initRes.resultVar, regName, align
            );
        }
    }

    return code;
}
