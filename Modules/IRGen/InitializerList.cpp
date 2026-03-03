//
// Created by Administrator on 2026/3/3.
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

//@__const.main.axx = private unnamed_addr constant
//<{ [23 x i32], [77 x i32] }>
//<{ [23 x i32] [i32 1, i32 2, i32 3, i32 4, i32 56, i32 7, i32 89, i32 78, i32 949, i32 49, i32 1, i32 21, i32 321, i32 321, i32 321, i32 321, i32 361, i32 32, i32 1321, i32 321, i32 23, i32 123, i32 132],
//[77 x i32] zeroinitializer }>,
//align 16


std::string GenClass::globalCode;
size_t GenClass::listCnt = 0;

GenClass::exprResult GenClass::InitializerListExpression(const sPtr<ast::InitializerList> &_initList,
                                                         const sPtr<type::CompileType> &_type) {
    auto type = TypeToLLVM(_type);
    auto varName = std::format("{}", exprCnt++);

    auto initCode = std::format("%{} = load ptr , ptr %{{}}\n", varName);
    std::string resultCode;
    for (auto [value,i] : std::views::zip(_initList->Values,std::views::iota(0u))) {
        auto [llvmType, resultVar, code, isCopyResult] = ExpressionExpand(value);
        if (llvmType == "list") {
            std::string memberId = std::format("{}", exprCnt++);
            resultCode += std::format("%{} = getelementptr inbounds {}, ptr %{}, i32 0, i32 {}\n",
                                      memberId, type, varName, i);
            resultCode += std::vformat(resultVar, std::make_format_args(memberId));
            resultCode += code;
        } else {
            resultCode += code;
            std::string memberId = std::format("{}", exprCnt++);
            resultCode += std::format("%{} = getelementptr inbounds {}, ptr %{}, i32 0, i32 {}\n",
                                      memberId, type, varName, i);
            if (isCopyResult) {
                resultCode += std::format("call void {}(ptr %{}, ptr {}, i64 {}, i1 false)\n",
                                         llvmCopy, memberId, resultVar, type::GetSize(value->GetType()));
            } else {
                resultCode += std::format("store {} {}, ptr %{}\n",
                                         llvmType, resultVar, memberId);
            }
        }
    }
    return exprResult{"list", initCode, resultCode, true};
}

std::string GenClass::ConstInitializerListExpression(
    const sPtr<ast::InitializerList> &_initList, const sPtr<type::CompileType> &_type) {
    auto elementStrings = _initList->Values
                          | std::views::transform([](const sPtr<ast::Expression> &expr) {
                              return ConstExpressionExpand(expr->GetType(), expr);
                          });
    std::string joinedElements = elementStrings
                                 | std::views::join_with(std::string(", "))
                                 | std::ranges::to<std::string>();
    if (std::get_if<type::StructDefinition>(&*_type)) {
        return std::format("{{ {} }}", joinedElements);
    }
    if (const auto *arrayType = std::get_if<type::ArrayType>(&*_type)) {
        return std::format("[{} x {}] {{ {} }}", arrayType->Length, GetTypeName(*arrayType->BaseType),
                           joinedElements);
    }
    ErrorPrintln("Error: Initializer list can only be used for struct or array types.\n");
    std::exit(-1);
}
