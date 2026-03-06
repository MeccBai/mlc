//
// Created by Administrator on 2026/3/6.
//
module Parser;
import Token;
import aux;
import :Decl;

std::vector<std::string_view> argSplit(const std::string_view str) {
    std::vector<std::string_view> results;
    int depth = 0;
    size_t start = 0;

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '(' || str[i] == '{' || str[i] == '[') depth++;
        else if (str[i] == ')' || str[i] == '}' || str[i] == ']') depth--;
        else if (str[i] == ',' && depth == 0) {
            results.push_back(str.substr(start, i - start));
            start = i + 1;
        }
    }
    results.push_back(str.substr(start)); // 别忘了最后一块
    if (*results.rbegin()->rbegin() == ';') {
        *results.rbegin() = results.back().substr(0, results.back().size() - 2);
    }
    if (*results.rbegin()->rbegin() == ')') {
        *results.rbegin() = results.back().substr(0, results.back().size() - 1);
    }
    return results;
}

std::vector<std::string_view> split(std::string_view str, const std::string_view _delimiter) {
    std::vector<std::string_view> result;
    size_t start = 0;
    while (true) {
        const size_t pos = str.find(_delimiter, start);
        if (pos == std::string_view::npos) {
            result.push_back(str.substr(start));
            break;
        }
        result.push_back(str.substr(start, pos - start));
        start = pos + _delimiter.length();
    }
    return result;
}
