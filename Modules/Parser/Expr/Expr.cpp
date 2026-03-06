//
// Created by Administrator on 2026/2/21.
//

module Parser;
import std;
import aux;
import :Decl;
import :Expr;

ast::BaseOperator toBaseOperator(const std::string_view _token) {
    if (ast::BaseOperators.contains(_token)) {
        return ast::BaseOperators.at(_token);
    }
    ErrorPrintln("Invalid operator : '{}'", _token);
    std::exit(-1);
}


bool isNameChar(const char c) {
    return std::isalnum(c) || c == '_';
}
using VariableContext = astClass::ContextTable<ast::VariableStatement>;
using FunctionContext = astClass::ContextTable<ast::FunctionDeclaration>;


bool isUnary(const ast::BaseOperator op) {
    switch (op) {
        case ast::BaseOperator::Sub: // -a
        case ast::BaseOperator::AddressOf: // &a
        case ast::BaseOperator::Dereference: // *a
        case ast::BaseOperator::Not: // !a
        case ast::BaseOperator::BitNot: // ~a
            return true;
        default:
            return false;
    }
}



using VariableContext = astClass::ContextTable<ast::VariableStatement>;
using FunctionContext = astClass::ContextTable<ast::FunctionDeclaration>;

std::string operatorToString(ast::BaseOperator op) {
    for (const auto &[str, val]: ast::BaseOperators) {
        if (val == op) return std::string(str);
    }
    return "UnknownOp";
}

template<typename type>
using sPtr = std::shared_ptr<type>;

sPtr<ast::Expression> astClass::expressionParser(ContextTable<ast::VariableStatement> &_contextTable,
                                                 const std::string_view _expressionContent) {
    if (const auto pos = _expressionContent.find('{'); pos != std::string_view::npos) {
        if (_expressionContent.back() != '}') {
            ErrorPrintln("MLC Syntax Error: Unmatched opening brace '{{' in expression: {}", _expressionContent);
            std::exit(-1);
        }
    }

    if (const auto pos = _expressionContent.find('='); pos != std::string_view::npos) {
        if (_expressionContent[pos + 1] != '=') {
            ErrorPrintln("MLC Syntax Error: Unexpected assignment operator '=' in expression: {}", _expressionContent);
            std::exit(-1);
        }
    }

    const auto expressionTree = ast::deepSplit(_expressionContent);

    ast::dumpFragments(expressionTree);

    auto result = expressionTreeParser(_contextTable, expressionTree);

    return result;
}

sPtr<ast::Expression> astClass::constExpressionParser(const std::string_view _constExpressionContent) {
    auto dummyContext = ContextTable<ast::VariableStatement>{};
    return expressionParser(dummyContext, _constExpressionContent);
}


// --- 3. 调度中心：主解析器 ---
sPtr<ast::Expression> astClass::expressionTreeParser(ContextTable<ast::VariableStatement> &_context,
                                                     const ast::exprTree &_expressionContent) {
    // --- 1. 处理原子 (叶子节点) ---
    if (const auto atomPtr = std::get_if<std::string_view>(&_expressionContent.data)) {
        // 使用你写的 parseAtom 或之前的逻辑处理常量、变量、函数
        return parseAtom(_context, *atomPtr);
    }


    // --- 2. 处理容器 (std::vector<exprTree>) ---
    const auto fragments = std::get<std::vector<ast::exprTree> >(_expressionContent.data);

    // 安全检查：如果容器里只有一个元素，剥壳重来
    if (fragments.size() == 1) return expressionTreeParser(_context, fragments[0]);

    // 查找当前层级结合力最弱的分割点
    int splitIndex = findSplitOperator(fragments);

    // 如果没找到操作符，说明是某种未处理的语法错误
    if (splitIndex == -1) return expressionTreeParser(_context, fragments[0]);

    const auto opStr = std::get<std::string_view>(fragments[splitIndex].data);
    const auto op = toBaseOperator(opStr);

    // --- 3. 拦截成员访问 (最高优先级处理) ---
    // 这里顺应你的想法：一旦遇到 . 或 -> 且它是当前的分割点（或者手动拦截）
    if (op == ast::BaseOperator::Dot || op == ast::BaseOperator::Arrow) {
        // 注意：因为 findSplitOperator 找的是最低优先级，
        // 如果它找到了 . 说明这一层只有后缀运算。
        return handleMemberAccess(_context, fragments, splitIndex);
    }

    if (op == ast::BaseOperator::Subscript) {
        return handleSubscriptAccess(_context, fragments, splitIndex);
    }

    if (op == ast::BaseOperator::AddressOf) {
        if (fragments.size() != 2) {
            ErrorPrintln("Address-of operator '@' must be followed by exactly one operand.");
            std::exit(-1);
        }
        return ast::MakeExpression(ast::MakeCompExpr(
            std::vector{expressionTreeParser(_context, fragments[1])},
            std::vector{op}
        ));
    }

    // --- 4. 递归构建 CompositeExpression ---
    // 左侧：从开始到 splitIndex 之前的所有片段
    const std::vector leftPart(fragments.begin(), fragments.begin() + splitIndex);
    const ast::exprTree leftTree = (leftPart.size() == 1) ? leftPart[0] : ast::exprTree(leftPart);

    // 右侧：从 splitIndex 之后到最后的所有片段
    const std::vector rightPart(fragments.begin() + splitIndex + 1, fragments.end());
    const ast::exprTree rightTree = (rightPart.size() == 1) ? rightPart[0] : ast::exprTree(rightPart);

    if (leftPart.empty()) {
        // 关键：不能直接返回右边，得给它套个“单目”的壳子！
        return ast::MakeExpression(ast::MakeCompExpr(
                std::vector{expressionTreeParser(_context, rightTree)},
                std::vector{op}
            )
        );
    }

    const auto leftExpr = expressionTreeParser(_context, leftTree);
    const auto rightExpr = expressionTreeParser(_context, rightTree);
    type::ValidateType(leftExpr->GetType(), rightExpr->GetType(), "Binary operator type mismatch");

    return ast::MakeExpression(ast::MakeCompExpr(
            std::vector{leftExpr, rightExpr},
            std::vector{op}
        )
    );
}


int astClass::findSplitOperator(const std::vector<ast::exprTree> &fragments) {
    int maxPriority = -1;
    int splitIndex = -1;

    for (int i = 0; i < static_cast<int>(fragments.size()); ++i) {
        if (fragments[i].isOperator) {
            const auto opStr = std::get<std::string_view>(fragments[i].data);
            auto op = toBaseOperator(opStr);
            const auto priority = ast::OperatorPriority.at(op);

            // 【关键修改】：实现左结合性
            // 找到优先级最高的（数值最大）的运算符作为分割点
            if (priority > maxPriority) {
                maxPriority = priority;
                splitIndex = i;
            }
            // 如果优先级相同，且它是左结合的运算符（如 []），保留之前找到的最左边的那个
            // 也就是在 if (priority > maxPriority) 中就已经包含了对左结合的处理
        }
    }
    return splitIndex;
}
