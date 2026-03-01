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

namespace gen = mlc::ir::gen;

using GenClass = gen::IRGenerator;
namespace ast = mlc::ast;
using size_t = std::size_t;
template<typename type>
using sPtr = std::shared_ptr<type>;
namespace type = ast::Type;



std::string GenClass::GlobalVariable(const sPtr<ast::VariableStatement> &_variable) {
    if (const auto initExpr = _variable->Initializer; initExpr != nullptr && ConstExpressionCheck(initExpr)) {
        auto value = ConstExpressionExpand(_variable->VarType, initExpr);
        return std::format("@{} = global {}", _variable->Name, value);
    }
    ErrorPrintln("Error: Global variable '{}' must be initialized with a constant expression.\n", _variable->Name);
    std::exit(-1);
}

std::string GenClass::LocalVariable(const sPtr<ast::VariableStatement> &_variable) {
    auto registerName = std::format("%{}",_variable->Name);
    auto alignSize = std::visit([](auto &&t) -> size_t {
        return t.Size();
    }, *_variable->VarType);
    auto alloca = std::format("{} = alloca {}, align {}\n", registerName, TypeToLLVM(_variable->VarType),alignSize);
    std::string store;
    std::string varCode;
    if (_variable->Initializer) {
        if (_variable->Initializer->GetVariable()) {
            const auto [llvmType, resultVar, code] = ExpressionExpand(_variable->Initializer);
            varCode = code;
            auto tempRegister = std::format("%{}", exprCnt++);
            auto load = std::format("{} = load {}, {}* {}, align {}\n", tempRegister,llvmType, TypeToLLVM(_variable->VarType),resultVar ,alignSize);
            store += load;
            store += std::format("store {} {}, {}* {}, align {}\n", llvmType, tempRegister, TypeToLLVM(_variable->VarType), registerName,alignSize);
        }
        if (_variable->Initializer->GetConstValue()) {
            auto value = ConstExpressionExpand(_variable->VarType, _variable->Initializer);
            store += std::format("store {}, {}* {}, align {}\n", value, TypeToLLVM(_variable->VarType), registerName,alignSize);
        }
    }
    return std::format("{}{}{}", alloca, varCode, store);
}
