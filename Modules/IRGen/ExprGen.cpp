//
// Created by Administrator on 2026/2/27.
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

size_t GenClass::exprCnt = 0;

GenClass::ExprResult GenClass::ExpressionExpand(sPtr<ast::Expression> &_expression) {
    // 1. 获取表达式的 LLVM 类型 (调用你之前写的 TypeToLLVM)

    std::string type = TypeToLLVM(_expression->GetType());
    if (const auto constPtr = std::get_if<ast::ConstValue>(&*_expression->Storage)) {
        return {type, constPtr->Value, ""};
    }
    if (const auto enumPtr = std::get_if<ast::EnumValue>(&*_expression->Storage)) {
        return {type, std::to_string(enumPtr->Index), ""};
    }
    if (const auto varPtr = std::get_if<sPtr<ast::Variable> >(&*_expression->Storage)) {
        auto result = ExprResult{
            type, std::format("%{}", exprCnt),
            std::format("%{} = load {}, {} %{}", exprCnt, type, type, varPtr->get()->Name)
        };
        exprCnt += 1;
        return result;
    }
    if (const auto memberPtr = std::get_if<sPtr<ast::MemberAccess>>(&*_expression->Storage)) {
        auto structName = TypeToLLVM(std::make_shared<type::CompileType>(*memberPtr->get()->StructDef));
        auto basePtrName =  memberPtr->get()->Name;
        size_t gepReg = exprCnt++;
        size_t loadReg = exprCnt++; // 💥 修正这里：确保 loadReg 是全新的编号
        const auto gepInstr = std::format(
            "%{} = getelementptr {}, {}* %{}, i32 0, i32 {}",
            gepReg,
            structName,
            structName,
            basePtrName,
            memberPtr->get()->Index
        );
        const auto loadInstr = std::format(
            "%{} = load {}, {} %{}",
            loadReg,
            type,
            type,
            gepReg // 使用 GEP 的结果作为地址
        );
        const auto code = std::format("{}\n{}\n", gepInstr, loadInstr);
        auto result = ExprResult{
            type,
            std::format("%{}", loadReg), // 💡 返回 LOAD 的结果 (数值)
            code
        };
        return result;
    }
#if 0
    if (const auto compPtr = std::get_if<sPtr<ast::CompositeExpression> >(&*_expression->Storage)) {
        auto &[operators,components,opFirst] = **compPtr;
        if (opFirst) {
            if (auto var = std::get_if<ast::Variable>(components[0]->Storage.get())) {
                return ExprResult{
                    std::format("*{}", TypeToLLVM(components[0]->GetType())),
                    std::format("%{}", var->Name), ""
                };
            }
        }
        auto expandExpr = components | std::views::transform([this](const sPtr<ast::Expression> &expr) {
            return ExpressionExpand(expr);
        }) | std::ranges::to<std::vector<ExprResult> >();
    }
#endif
    if (const auto functionCallPtr = std::get_if<sPtr<ast::FunctionCall> >(&*_expression->Storage)) {
    }

}

std::string GenClass::ConstExpressionExpand(const sPtr<ast::Type::CompileType> &_type,
                                            const sPtr<ast::Expression> &_expression) {
    if (!_expression || !_expression->Storage) return "";
    const auto &data = *_expression->Storage;
    if (const auto *constVal = std::get_if<ast::ConstValue>(&data)) {
        std::string typeStr = GetTypeName(*_expression->GetType());
        return std::format("{} {}", typeStr, constVal->Value);
    }
    if (const auto *initListPtr = std::get_if<sPtr<ast::InitializerList> >(&data)) {
        const auto &initList = *initListPtr;
        auto elementStrings = initList->Values
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
    }

    if (const auto *compExprPtr = std::get_if<sPtr<ast::CompositeExpression> >(&data)) {
        const auto &compExpr = *compExprPtr;
        if (compExpr->Components.empty()) return "";

        std::string result = ConstExpressionExpand(compExpr->Components[0]->GetType(), compExpr->Components[0]);
        for (size_t i = 0; i < compExpr->Operators.size(); ++i) {
            std::string opSymbol = ast::BaseIROperators.at(compExpr->Operators[i]);
            std::string nextComponent = ConstExpressionExpand(compExpr->Components[i + 1]->GetType(),
                                                              compExpr->Components[i + 1]);
            result = std::format("{} ({} {}, {})", opSymbol, GetTypeName(*_expression->GetType()), result,
                                 nextComponent);
        }
        return result;
    }

    if (const auto *funcCallPtr = std::get_if<sPtr<ast::FunctionCall> >(&data)) {
        const auto &funcCall = *funcCallPtr;
        auto functionName = funcCall->FunctionDecl ? funcCall->FunctionDecl->Name : "unknown_func";
        auto sourceType = std::get_if<type::BaseType>(&*funcCall->Arguments[0]->GetType());
        auto convertFuncType = &*std::ranges::find_if(type::BaseTypes, [&](const type::BaseType &baseType) {
            return baseType.Name == functionName;
        });
        std::string castOp = determineCastOperator(sourceType, convertFuncType);
        return std::format("{} ({} {})", castOp, GetTypeName(*convertFuncType),
                           ConstExpressionExpand(funcCall->Arguments[0]->GetType(), funcCall->Arguments[0]));
    }

    ErrorPrintln("Error: Expression is not a constant expression and cannot be evaluated at compile time.\n");
    std::exit(-1);
}

std::string GenClass::OperatorToIR(const type::BaseType *_type, const ast::BaseOperator _value) {
    switch (_value) {
        // --- 算术运算 (整数) ---
        case ast::BaseOperator::Add:
            if (_type->Name.starts_with('f')) {
                return "fadd"; // 浮点数加法
            }
            return "add";
        case ast::BaseOperator::Sub:
            if (_type->Name.starts_with('f')) {
                return "fsub"; // 浮点数减法
            }
            return "sub";
        case ast::BaseOperator::Mul:
            if (_type->Name.starts_with('f')) {
                return "fmul"; // 浮点数乘法
            }
            if (_type->Name.starts_with('i')) {
                return "mul"; // 整数乘法
            }
            return "mul";
        case ast::BaseOperator::Div:
            if (_type->Name.starts_with('i')) {
                return "sdiv"; // 假设有符号
            }
            if (_type->Name.starts_with('f')) {
                return "fdiv"; // 浮点数除法
            }
            return "udiv"; // 无符号
        case ast::BaseOperator::Mod:
            if (_type->Name.starts_with('i')) {
                return "srem"; // 假设有符号
            }
            if (_type->Name.starts_with('f')) {
                return "frem"; // 浮点数模运算
            }
            return "urem"; // 无符号
        // --- 关系运算 (整数比较) ---
        // ⚠️ LLVM 的 icmp 需要具体的条件代码 (eq, ne, sgt, etc.)
        case ast::BaseOperator::Equal:
            if (_type->Name.starts_with('f')) {
                return "fcmp oeq"; // 浮点数相等
            }
            return "icmp eq";
        case ast::BaseOperator::NotEqual:
            if (_type->Name.starts_with('f')) {
                return "fcmp one"; // 浮点数不相等
            }
            return "icmp ne";
        case ast::BaseOperator::Greater:
            if (_type->Name.starts_with('f')) {
                return "fcmp ogt"; // 浮点数大于
            }
            if (_type->Name.starts_with('i')) {
                return "icmp sgt"; // 有符号大于
            }
            return "icmp ugt"; // 无符号大于
        case ast::BaseOperator::Less:
            if (_type->Name.starts_with('f')) {
                return "fcmp olt"; // 浮点数小于
            }
            if (_type->Name.starts_with('i')) {
                return "icmp slt"; // 有符号小于
            }
            return "icmp ult"; // 无符号小于
        case ast::BaseOperator::GreaterEqual:
            if (_type->Name.starts_with('f')) {
                return "fcmp oge"; // 浮点数大于等于
            }
            if (_type->Name.starts_with('i')) {
                return "icmp sge"; // 有符号大于等于
            }
            return "icmp uge"; // 无符号大于等于
        case ast::BaseOperator::LessEqual:
            if (_type->Name.starts_with('f')) {
                return "fcmp ole"; // 浮点数小于等于
            }
            if (_type->Name.starts_with('i')) {
                return "icmp sle"; // 有符号小于等于
            }
            return "icmp ule"; // 无符号小于等于

        // --- 逻辑运算 (通常使用位运算指令) ---
        case ast::BaseOperator::And: return "and";
        case ast::BaseOperator::Or: return "or";
        case ast::BaseOperator::Not: return "xor";
        // Not 通常使用 xor 1 实现

        // --- 位运算 ---
        case ast::BaseOperator::BitAnd: return "and";
        case ast::BaseOperator::BitOr: return "or";
        case ast::BaseOperator::BitXor: return "xor";
        case ast::BaseOperator::ShiftLeft: return "shl";
        // ⚠️ 右移需要区分算术右移和逻辑右移
        case ast::BaseOperator::ShiftRight: return "ashr"; // 算术右移

        // --- 指针操作 ---
        case ast::BaseOperator::AddressOf: return "ptrtoint"; // 视具体需求而定
        case ast::BaseOperator::Dereference: return "load"; // 需要额外处理类型

        default: return "";
    }
}

std::string GenClass::TypeToLLVM(const sPtr<type::CompileType> &_type) {
    if (const auto baseType = std::get_if<type::BaseType>(&*_type)) {
        if (baseType->Name.starts_with('i')) {
            return std::format("i{}", baseType->Size() * 8);
        }
        if (baseType->Name.starts_with('f')) {
            return std::format("f{}", baseType->Size() * 8);
        }
    }
    if (const auto structDef = std::get_if<type::StructDefinition>(&*_type)) {
        return std::format("%struct.{}", structDef->Name);
    }
    if (const auto arrayType = std::get_if<type::ArrayType>(&*_type)) {
        return std::format("[{} x {}]", arrayType->Length, TypeToLLVM(arrayType->BaseType));
    }
    if (std::get_if<type::PointerType>(&*_type)) {
        return "ptr";
    }
    if (std::get_if<type::EnumDefinition>(&*_type)) {
        return "i32";
    }
    ErrorPrintln("Error: Unsupported type for LLVM IR generation.");
    std::exit(-1);
}
#if 0
std::string GenClass::TripleExpression(const expr &_left, const expr &_right, ast::BaseOperator _op) {
    const auto leftResult = ExpressionExpand(_left);
    const auto rightResult = ExpressionExpand(_right);
    auto code = std::format("{}\n{}\n", leftResult.code, rightResult.code);

    std::string llvmOp = OperatorToIR(std::get_if<type::BaseType>(&*_left->GetType()), _op);

    return std::format("{} {} {}, {}", llvmOp, leftResult.llvmType, leftResult.resultVar, rightResult.resultVar);
}

GenClass::ExprResult GenClass::BinaryExpression(const expr &_expr, ast::BaseOperator _op) {
    using op = ast::BaseOperator;
    auto [llvmType, resultVar, code] = ExpressionExpand(_expr);
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
            return ExprResult{
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
            return ExprResult{
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
            return ExprResult{
                llvmType + "*",
                std::format("%{}", newReg),
                code + instr
            };
        }
        default:
            ErrorPrintln("Compiler Internal Error");
            std::exit(-1);
    }
}
#endif