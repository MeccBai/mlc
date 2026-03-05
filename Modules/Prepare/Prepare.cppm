//
// Created by Administrator on 2026/2/19.
//

export module Prepare;
import std;


namespace mlc::prepare {
    std::string removeComments(const std::string &source) {
        std::string result;
        result.reserve(source.length()); // 优化内存分配 ⚡

        enum class State { Default, SingleLine, MultiLine, StringLiteral };
        auto state = State::Default;

        for (std::size_t i = 0; i < source.length(); ++i) {
            char c = source[i];
            char next = (i + 1 < source.length()) ? source[i + 1] : '\0';

            switch (state) {
                case State::Default:
                    if (c == '/' && next == '/') {
                        state = State::SingleLine;
                        i++; // 跳过第二个 /
                    } else if (c == '/' && next == '*') {
                        state = State::MultiLine;
                        i++; // 跳过 *
                    } else if (c == '"') {
                        state = State::StringLiteral;
                        result += c;
                    } else {
                        result += c;
                    }
                    break;
                case State::SingleLine:
                    if (c == '\n') {
                        state = State::Default;
                        result += c; // 保留换行符，方便后续压缩处理
                    }
                    break;
                case State::MultiLine:
                    if (c == '*' && next == '/') {
                        state = State::Default;
                        i++; // 跳过 /
                    }
                    break;
                case State::StringLiteral:
                    if (c == '\\' && next == '"') {
                        // 处理转义字符 \"
                        result += c;
                        result += next;
                        i++;
                    } else if (c == '"') {
                        state = State::Default;
                        result += c;
                    } else {
                        result += c;
                    }
                    break;
            }
        }
        return result;
    }

    bool isSymbol(const char c) {
        static constexpr std::string_view symbols = ",;=+-*/()[]{}<>.:&|!%^~?@$";
        return symbols.find(c) != std::string::npos;
    }

    export std::string Prepare(const std::string &_input) {
        const std::string context = removeComments(_input);

        // 1. 归一化空格（保留所有空格，只把换行符等转为空格）
        std::string normalized;
        bool lastWasSpace = false;
        bool inString = false;

        for (std::size_t i = 0; i < context.length(); ++i) {
            char c = context[i];

            // 处理字符串边界，防止处理字符串内部的空格
            if (c == '"' && (i == 0 || context[i - 1] != '\\')) {
                inString = !inString;
            }

            if (!inString && std::isspace(static_cast<unsigned char>(c))) {
                if (!lastWasSpace) {
                    normalized += ' ';
                    lastWasSpace = true;
                }
            } else {
                normalized += c;
                lastWasSpace = false;
            }
        }

        // 2. 精准脱水：只删除符号前后的空格，但严禁触碰字符串内部 🛡️
        std::string finalResult;
        inString = false;

        for (std::size_t i = 0; i < normalized.length(); ++i) {
            const char current = normalized[i];

            // 维护字符串状态
            if (current == '"' && (i == 0 || normalized[i - 1] != '\\')) {
                inString = !inString;
                finalResult += current;
                continue;
            }

            if (inString) {
                // 字符串内部：原封不动保留
                finalResult += current;
            } else {
                // 代码区：如果是空格，检查前后是否是符号
                if (current == ' ') {
                    bool prevIsSym = (i > 0 && isSymbol(normalized[i - 1]));
                    bool nextIsSym = (i + 1 < normalized.length() && isSymbol(normalized[i + 1]));

                    // 如果前后有符号，这个空格就是多余的，跳过它
                    if (prevIsSym || nextIsSym) continue;
                }
                finalResult += current;
            }
        }

        // 3. Trim 两端
        auto isSpace = [](const unsigned char c) { return std::isspace(c); };
        auto trimmed = finalResult
                       | std::views::drop_while(isSpace)
                       | std::views::reverse
                       | std::views::drop_while(isSpace)
                       | std::views::reverse;

        return trimmed | std::ranges::to<std::string>();
    }
}
