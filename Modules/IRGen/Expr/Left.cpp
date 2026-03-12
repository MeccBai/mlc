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
        auto &[operators,components,opFirst] = **compPtr;
        if (components.empty()) {
            ErrorPrintln("Error: Composite expression has no components.\n");
            std::exit(-1);
        }
        if (components.size() >= 2 && (components[1]->GetMemberAccess() || operators[0] ==
                                       ast::BaseOperator::Subscript)) {
            return MemberAccessExpression(_expr, true);
        }
        if (opFirst) {
            if (const auto var = *(components[0]->GetVariable())) {
                if (operators[0] == ast::BaseOperator::AddressOf) {
                    return exprResult{
                        "ptr",
                        std::format("%{}", var->Name), ""
                    };
                }
                const auto regName = std::format("%{}", exprCnt);
                auto returnType = components[0]->GetType();
                if (auto *ptrType = type::GetType<type::PointerType>(returnType)) {
                    returnType = ptrType->Dereference();
                    if (type::IsType<type::StructDefinition>(returnType)) {
                        ErrorPrintln("Error: use -> to access struct\n");
                        std::exit(-1);
                    }
                }
                const auto llvmType = TypeToLLVM(returnType);
                const auto instruction = std::format("%{} = load ptr,ptr %{}, align 8\n", exprCnt++, var->Name);
                return exprResult{llvmType, regName, instruction};
            }
        }
        return MemberAccessExpression(_expr, true);
    }
    ErrorPrintln("Error: Expression is not a valid left-hand side expression.\n");
    std::exit(-1);
}
