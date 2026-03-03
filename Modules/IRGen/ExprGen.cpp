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


GenClass::ExprResult GenClass::ExpressionExpand(const sPtr<ast::Expression> &_expression) {
    // 1. 获取表达式的 LLVM 类型 (调用你之前写的 TypeToLLVM)
    const auto type = TypeToLLVM(_expression->GetType());
    if (const auto constPtr = _expression->GetConstValue()) {
        return {type, constPtr->Value, ""};
    }
    if (const auto enumPtr = _expression->GetEnumValue()) {
        return {type, std::to_string(enumPtr->Index), ""};
    }
    if (const auto varPtr = _expression->GetVariable()) {
        auto result = ExprResult{
            type, std::format("%{}", (*varPtr)->Name), ""
        };
        exprCnt += 1;
        return result;
    }
    if (const auto compPtr = _expression->GetCompositeExpression()) {
        auto &[operators,components,opFirst] = **compPtr;
        if (components.empty()) {
            ErrorPrintln("Error: Composite expression has no components.\n");
            std::exit(-1);
        }
        if (components.size() >= 2 && components[1]->GetMemberAccess()) {
            return MemberAccessExpression(_expression);
        }
        if (opFirst) {
            if (const auto var = components[0]->GetVariable()) {
                return ExprResult{
                    std::format("*{}", TypeToLLVM(components[0]->GetType())),
                    std::format("%{}", (*var)->Name), ""
                };
            }
        }
        auto expandExpr = components | std::views::transform([](const sPtr<ast::Expression> &expr) {
            return ExpressionExpand(expr);
        }) | std::ranges::to<std::vector<ExprResult> >();
    }

    if (const auto functionCallPtr = _expression->GetFunctionCall()) {
        const auto funcCall = FunctionCall(*functionCallPtr);
        if (funcCall.isCopyResult) {
            return ExprResult {
                funcCall.llvmType,
                funcCall.resultVar,
                funcCall.callCode,
                funcCall.isCopyResult
            };
        }
        auto resultVar = std::format("%{}", exprCnt++);
        return ExprResult{
            funcCall.llvmType,
            funcCall.resultVar,
            std::format("%{} = {}",resultVar, funcCall.callCode)
        };
    }
    if (const auto initListPtr = _expression->GetInitializerList()) {

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


GenClass::ExprResult GenClass::TripleExpression(const expr &_left, const expr &_right, ast::BaseOperator _op) {
    const auto leftResult = ExpressionExpand(_left);
    const auto rightResult = ExpressionExpand(_right);
    auto code = std::format("{}\n{}\n", leftResult.code, rightResult.code);

    std::string llvmOp = OperatorToIR(std::get_if<type::BaseType>(&*_left->GetType()), _op);

    auto result = ExprResult{
        leftResult.llvmType, std::format("%{}", exprCnt),
        code + std::format("{} = {} {} {}, {}\n", std::format("%{}", exprCnt),
                           llvmOp, leftResult.llvmType, leftResult.resultVar, rightResult.resultVar)
    };

    return result;
}

GenClass::ExprResult GenClass::TripleExpression(const ExprResult &_left, const ExprResult &_right,
                                                ast::BaseOperator _op) {
    std::string targetReg = std::format("%{}", exprCnt++);
    const std::string combinedCode = _left.code + _right.code;
    std::string llvmOp = OperatorToIR(_left.llvmType, _op);
    const std::string currentInstr = std::format("  {} = {} {} {}, {}\n",
                                                 targetReg, llvmOp, _left.llvmType,
                                                 _left.resultVar, _right.resultVar);
    return ExprResult{
        _left.llvmType,
        targetReg,
        combinedCode + currentInstr
    };
}

GenClass::ExprResult GenClass::BinaryExpression(const expr &_expr, ast::BaseOperator _op) {
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

GenClass::ExprResult GenClass::GradientExpression(const std::vector<ExprResult> &_expr,
                                                  const std::vector<ast::BaseOperator> &_ops) {
    std::vector<ExprResult> workingExpr = _expr;
    std::vector<ast::BaseOperator> workingOps = _ops;

    for (size_t priority = 1; priority <= 12; ++priority) {
        for (size_t i = 0; i < workingOps.size();) {
            if (ast::OperatorPriority.at(workingOps[i]) == priority) {
                const ExprResult &left = workingExpr[i];
                const ExprResult &right = workingExpr[i + 1];
                const ast::BaseOperator op = workingOps[i];
                ExprResult combined = TripleExpression(left, right, op);
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
