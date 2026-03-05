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


GenClass::exprResult GenClass::MemberAccessExpression(const expr &_base) {
    const auto composite = _base->GetCompositeExpression();
    if (!composite) {
        ErrorPrintln("Error: Invalid member access expression.\n");
        std::exit(-1);
    }

    // 1. 📢 解析第一个元素
    exprResult currentResult = ExpressionExpand((*composite)->Components[0]);
    // 💡 获取第一个元素的类型作为初始化类型
    const type::CompileType* currentType = (*composite)->Components[0]->GetType().get();

    // 2. 依次扫描后面的组件
    for (size_t i = 0; i < (*composite)->Operators.size(); ++i) {
        const auto op = (*composite)->Operators[i];
        const auto& nextComp = (*composite)->Components[i+1];
        currentResult = MemberAccessBinary(currentType, currentResult, nextComp, op);
        currentType = &*(*composite)->Components[i+1]->GetType();
    }

    return currentResult;
}

GenClass::exprResult GenClass::MemberAccessBinary(const type::CompileType * _type, const exprResult &_parent, const expr &_child, ast::BaseOperator _op) {

    // 💡 处理数组索引的情况：a[i]
    if (const auto arrayType = std::get_if<type::ArrayType>(_type)) {
        // 这里假设 _child 是索引表达式本身
        auto indexResult = ExpressionExpand(_child);
        std::string gepReg = std::format("%{}", exprCnt++);

        // GEP 数组寻址：父类型是数组，索引是 childResult 的值
        std::string gepInstr = std::format(
            "  {} = getelementptr {}, ptr {}, i32 0, i32 {}\n",
            gepReg,
            _parent.llvmType, // 父数组类型
            _parent.resultVar, // 数组地址
            indexResult.resultVar  // 动态索引
        );

        return exprResult{
            TypeToLLVM(arrayType->BaseType), // 💡 返回的是元素类型！
            gepReg,
            _parent.code + indexResult.code + gepInstr
        };
    }

    // 💡 处理结构体成员的情况：a.b
    if (const auto structDef = std::get_if<type::StructDefinition>(_type)) {
        // 🚨 修正这里：确保从 _child 中正确获取 MemberAccess
        const auto memberPtr = std::get_if<sPtr<ast::MemberAccess>>(&*_child->Storage);
        if (!memberPtr) {
            ErrorPrintln("Error: Invalid member access node.\n");
            std::exit(-1);
        }
        auto memberAccess = memberPtr->get();

        std::string gepReg = std::format("%{}", exprCnt++);
        std::string gepInstr = std::format(
            "{} = getelementptr {}, ptr {}, i32 0, i32 {}\n",
            gepReg,
            _parent.llvmType, // 父结构体类型
            _parent.resultVar, // 结构体地址
            memberAccess->Index      // 编译期常量索引
        );
        return exprResult{
            TypeToLLVM(memberAccess->GetType()), // 💡 返回的是成员类型！
            gepReg,
            _parent.code + gepInstr
        };
    }

    if (const auto pointerType = std::get_if<type::PointerType>(_type)) {
        if (_op == ast::BaseOperator::Arrow) {
            // 处理 a->b 的情况：先解引用父指针，再访问成员
            std::string loadReg = std::format("%{}", exprCnt++);
            std::string loadInstr = std::format(
                "{} = load {}, ptr {}, align 4\n",
                loadReg,
                TypeToLLVM(pointerType->BaseType), // 解引用后的类型
                _parent.resultVar // 父指针地址
            );
            // 递归调用 MemberAccessBinary 来访问成员
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
