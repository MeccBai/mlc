//
// Created by Administrator on 2026/2/21.
//

module Parser;
import std;
import aux;

using astClass = mlc::parser::AbstractSyntaxTree;

template<typename type>
using sPtr = std::shared_ptr<type>;
namespace ast = mlc::ast;
namespace type = ast::Type;
using size_t = std::size_t;

std::string_view getVariableName(std::string_view declaration) {
    if (declaration.empty()) return "";

    // 1. 确定左边界：跳过所有开头的指针符号 '*'
    // 因为已经没有空格了，直接从第一个不是 '*' 的字符开始
    const size_t start = declaration.find_first_not_of('*');
    if (start == std::string_view::npos) return "";

    // 2. 确定右边界：变量名结束于第一个 '[' 或 '='
    // 剩下的情况就是整个字符串直到末尾（比如单纯的 'x'）
    const size_t end = declaration.find_first_of("[=", start);

    if (end == std::string_view::npos) {
        // 如果没有 [ 或 =，说明整个剩余部分就是变量名
        return declaration.substr(start);
    }

    return declaration.substr(start, end - start);
}

std::vector<ast::VariableStatement> astClass::variableParser(
    const std::string_view variableContent) {
    // 这里可以进一步解析变量声明，提取变量类型、名称和初始化表达式等信息
    std::println("{}", variableContent);

    const auto pos = variableContent.find(' ');
    auto type = variableContent.substr(0, pos);
    const auto variables = variableContent.substr(pos + 1);

    const auto it = std::ranges::find_if(typeSymbolTable, [&](const auto &t) {
        return std::visit([](auto &&arg) -> std::string_view {
            return arg.Name; // 只要所有子类都有 Name，这个就能编译通过
        }, *t) == type;
    });

    if (it == typeSymbolTable.end()) {
        ErrorPrintln("Error: Unknown type '{}'\n", type);
    }

    auto baseType = *it;

    std::vector<std::string_view> declarations;
    std::stack<char> brackets;
    size_t start = 0;
    for (size_t i = 0; i < variables.length(); i++) {
        const char c = variables[i];

        // 1. 括号入栈逻辑 (保持你原本的高质量实现)
        if (c == '{' || c == '(' || c == '[') {
            brackets.push(c);
        } else if (c == '}' || c == ')' || c == ']') {
            if (brackets.empty()) {
                /* 报错逻辑... */
                std::exit(-1);
            }
            if (const char top = brackets.top(); (c == '}' && top == '{') || (c == ')' && top == '(') || (c == ']' && top == '[')) {
                brackets.pop();
            } else {
                /* 报错逻辑... */
                std::exit(-1);
            }
        }
        // 2. 核心修改：遇到“外层”逗号 OR 分号时，都进行截取
        else if ((c == ',' || c == ';') && brackets.empty()) {
            // 简单修剪一下可能的首尾空格（如果你没预先做过 Trim 的话）
            // 虽然你可能已经脱水了，但这样更稳
            if (const std::string_view item = variables.substr(start, i - start); !item.empty()) {
                declarations.emplace_back(item);
            }

            start = i + 1; // 跳过当前的逗号或分号
        }
    }

    // 3. 兜底逻辑：处理没有以分号结尾的残余（虽然规范写法应该有分号）
    if (start < variables.length()) {
        std::string_view lastItem = variables.substr(start);
        // 这里做个简单的 strip，去掉可能的空白字符
        while (!lastItem.empty() && std::isspace(lastItem.back())) lastItem.remove_suffix(1);
        if (!lastItem.empty()) {
            declarations.emplace_back(lastItem);
        }
    }

    std::vector<ast::VariableStatement> result;
    for (auto declaration: declarations) {
        auto variableName = getVariableName(declaration);
        std::println("{} : {}", variableName, declaration);
        bool isPointer = false;
        if (declaration[0] == '$') {
            isPointer = true;
        }
        if (declaration.find('=') != std::string_view::npos) {
            const auto expressionContext = declaration.substr(declaration.find('=') + 1);
            auto expression = std::make_shared<ast::Expression>(expressionParser(expressionContext));
            if (isPointer) {
                size_t pointerLevel = 1;
                for (size_t i = 1; i < declaration.length(); i++) {
                    if (declaration[i] == '$') pointerLevel++;
                    else break;
                }
                auto pointerType = std::make_shared<type::PointerType>(type::PointerType(variableName, pointerLevel));
                pointerType->Finalize(baseType);
                result.emplace_back(variableName, std::make_shared<ast::Type::CompileType>(*pointerType), expression);
            } else {
                result.emplace_back(variableName, baseType, expression);
            }
        }
    }

    return result;
    //ast::VariableStatement("", {}, std::nullopt);
}
