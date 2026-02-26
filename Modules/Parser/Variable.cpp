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

    // 1. 确定左边界：跳过所有开头的指针符号 '$'
    // 因为已经没有空格了，直接从第一个不是 '$' 的字符开始
    const size_t start = declaration.find_first_not_of('$');
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

astClass::StatementTable<ast::Statement> astClass::globalVariableParser(

    const std::string_view variableContent) {

    // 这里可以进一步解析变量声明，提取变量类型、名称和初始化表达式等信息
    auto globalContext = ContextTable<ast::VariableStatement>{};
    const auto result = variableParser(globalContext, variableContent);
    for (const auto &var: result) {
        auto* vptr = std::get_if<ast::VariableStatement>(var.get());
        variableSymbolTable.emplace_back(std::make_shared<ast::VariableStatement>(*vptr));
    }
    return result;
    //ast::VariableStatement("", {}, std::nullopt);

}

astClass::StatementTable<ast::Statement> astClass::localVariableParser(
    ContextTable<ast::VariableStatement> &_context, std::string_view variableContent) {
    return variableParser(_context, variableContent);
    //ast::VariableStatement("", {}, std::nullopt);
}


struct VariablePack {
    std::string_view Type;
    std::string_view Name;
    std::string_view InitExpression;
};

std::vector<VariablePack> variablePacked(const std::string_view _variable) {
    std::vector<VariablePack> packs;

    // 1. 提取基础类型 (寻找第一个空格或 $ 符号)
    auto typePos = _variable.find_first_of(" $");
    if (typePos == std::string_view::npos) return {};

    std::string_view baseType = _variable.substr(0, typePos);
    std::string_view remaining = _variable.substr(typePos);
    // 去掉开头的空格，但保留 $，因为它属于变量修饰部分
    while(!remaining.empty() && std::isspace(remaining.front())) remaining.remove_prefix(1);

    // 2. 扫描并切分多个声明 (a[10], $b=10)
    std::stack<char> brackets;
    size_t start = 0;

    for (size_t i = 0; i <= remaining.length(); i++) {
        char c = (i < remaining.length()) ? remaining[i] : ';'; // 模拟分号结束

        if (c == '{' || c == '(' || c == '[') brackets.push(c);
        else if (c == '}' || c == ')' || c == ']') {
            if (!brackets.empty()) brackets.pop();
        }
        // 遇到外层逗号或模拟的分号
        else if ((c == ',' || c == ';') && brackets.empty()) {
            std::string_view fullDecl = remaining.substr(start, i - start);
            while(!fullDecl.empty() && std::isspace(fullDecl.front())) fullDecl.remove_prefix(1);
            while(!fullDecl.empty() && std::isspace(fullDecl.back())) fullDecl.remove_suffix(1);

            if (!fullDecl.empty()) {
                // 3. 进一步拆分 Name 和 InitExpression
                auto eqPos = fullDecl.find('=');
                if (eqPos != std::string_view::npos) {
                    packs.push_back({
                        baseType,
                        fullDecl.substr(0, eqPos),
                        fullDecl.substr(eqPos + 1)
                    });
                } else {
                    packs.push_back({baseType, fullDecl, ""});
                }
            }
            start = i + 1;
        }
    }
    return packs;
}

astClass::StatementTable<ast::Statement> astClass::variableParser(ContextTable<ast::VariableStatement> &_context,
                                                                  const std::string_view variableContent) {
    auto packs = variablePacked(variableContent);
    StatementTable<ast::Statement> result;

    for (const auto&[Type, Name, InitExpression] : packs) {
        // 1. 获取 BaseType
        const auto it = std::ranges::find_if(typeSymbolTable, [&](const auto &t) {
            return std::visit([](auto &&arg) { return arg.Name; }, *t) == Type;
        });
        if (it == typeSymbolTable.end()) {
            ErrorPrintln("Error: Unknown type '{}'\n", Type);
            std::exit(-1);
        }
        auto currentType = *it;

        // 2. 处理修饰符 (指针 & 多维数组) 并提取纯变量名
        std::string_view decl = Name;

        // --- 处理指针前缀 $$$ ---
        size_t pLevel = 0;
        while(pLevel < decl.size() && decl[pLevel] == '$') pLevel++;
        if(pLevel > 0) {
            auto pType = std::make_shared<type::PointerType>(pLevel);
            pType->Finalize(currentType);
            currentType = std::make_shared<type::CompileType>(*pType);
            decl.remove_prefix(pLevel);
        }

        // --- 处理数组后缀 [10][20] ---
        auto bracketPos = decl.find('[');
        std::string_view realName = decl.substr(0, bracketPos);
        if (bracketPos != std::string_view::npos) {
            std::string_view suffix = decl.substr(bracketPos);
            while(!suffix.empty() && suffix[0] == '[') {
                size_t end = suffix.find(']');
                size_t dim = std::stoull(std::string(suffix.substr(1, end - 1)));
                currentType = std::make_shared<type::CompileType>(
                    ast::Type::ArrayType(currentType, dim)
                );
                suffix = suffix.substr(end + 1);
            }
        }

        sPtr<ast::Expression> initExpr = nullptr;
        if (!InitExpression.empty()) {
            initExpr = expressionParser(_context, InitExpression);
            ValidateType(currentType, initExpr->GetType(), realName);
        }

        // 4. 构建并注册
        auto varStmt = std::make_shared<ast::VariableStatement>(realName, currentType, initExpr);
        auto stmtPtr = std::make_shared<ast::Statement>(std::move(*varStmt));
        auto* vPtr = std::get_if<ast::VariableStatement>(stmtPtr.get());

        result.emplace_back(stmtPtr);
        _context.emplace_back(std::shared_ptr<ast::VariableStatement>(stmtPtr, vPtr));
    }
    return result;
}
