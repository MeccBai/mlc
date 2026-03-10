//
// Created by Administrator on 2026/3/1.
//
module Generator;

import std;
import Token;
import keyword;
import Parser;
import aux;


GenClass::exprResult GenClass::MemberAccessExpression(const expr &_base, bool _isWrite) {
    auto *const composite = _base->GetCompositeExpression();
    if (!composite) {
        ErrorPrintln("Error: Invalid member access expression.\n");
        std::exit(-1);
    }
    const auto & [operators,components, opFirst] = **composite;
    exprResult currentResult = LeftExpressionExpand(components[0]);
    auto currentType = components[0]->GetType();
    for (size_t i = 0; i < components.size() - 1; ++i) {
        const auto op = operators[i];
        const auto &nextComp = components[i + 1];
        currentResult = MemberAccessBinary(&*currentType, currentResult, nextComp, op);
        currentType = components[i + 1]->GetType();
    }

    if (!_isWrite) {
        if (type::GetType<type::BaseType>(currentType)) {
            auto reg = std::format("ma{}", exprCnt++);
            currentResult.code += std::format(
                "%{} = load {}, ptr {}, align {}\n",
                reg,
                TypeToLLVM(currentType),
                currentResult.resultVar,
                type::GetSize(currentType)
            );
            currentResult.resultVar = std::format("%{}", reg);
        }
    }
    return currentResult;
}

GenClass::exprResult GenClass::MemberAccessBinary(const type::CompileType *_type, const exprResult &_parent,
                                                  const expr &_child, ast::BaseOperator _op) {
    auto gepReg = std::format("%ma{}", exprCnt++);

    if (const auto *const arrayType = std::get_if<type::ArrayType>(_type)) {
        auto indexResult = ExpressionExpand(_child);
        auto gepInstr = std::format(
            "{} = getelementptr {}, ptr {}, i32 0, i32 {}\n",
            gepReg,
            _parent.llvmType,
            _parent.resultVar,
            indexResult.resultVar
        );
        return exprResult{
            TypeToLLVM(arrayType->BaseType),
            gepReg,
            _parent.code + indexResult.code + gepInstr
        };
    }

    if (std::get_if<type::StructDefinition>(_type)) {
        auto *const memberPtr = std::get_if<sPtr<ast::MemberAccess> >(&*_child->Storage);
        if (!memberPtr) {
            ErrorPrintln("Error: Invalid member access node.\n");
            std::exit(-1);
        }
        auto *memberAccess = memberPtr->get();
        auto gepInstr = std::format(
            "{} = getelementptr {}, ptr {}, i32 0, i32 {}\n",
            gepReg,
            _parent.llvmType,
            _parent.resultVar,
            memberAccess->Index
        );
        return exprResult{
            TypeToLLVM(memberAccess->GetType()),
            gepReg,
            _parent.code + gepInstr
        };
    }

    if (const auto *const pointerType = std::get_if<type::PointerType>(_type)) {
        if (_op == ast::BaseOperator::Arrow) {
            auto loadReg = std::format("%ma{}", exprCnt++);
            auto loadInstr = std::format(
                "{} = load ptr, ptr {}, align 8\n",
                loadReg,
                _parent.resultVar
            );
            auto derefResult = exprResult{
                TypeToLLVM(pointerType->BaseType),
                loadReg,
                _parent.code + loadInstr
            };
            return MemberAccessBinary(&*pointerType->BaseType, derefResult, _child, ast::BaseOperator::Dot);
        }
    }

    ErrorPrintln("Error: Invalid parent type for member access.\n");
    std::exit(-1);
}
