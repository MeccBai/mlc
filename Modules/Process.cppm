//
// Created by Administrator on 2026/2/19.
//

export module Process;
import std;


namespace mlc {
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
        static constexpr std::string symbols = ",;=+-*/()[]{}<>.";
        return symbols.find(c) != std::string::npos;
    }

    export std::string PreProcess(const std::string &_input) {
        const std::string context = removeComments(_input);
        auto isSpace = [](const unsigned char c) { return std::isspace(c); };
        const auto normalized = context
                          | std::views::transform([&](const char c) {
                              return isSpace(static_cast<unsigned char>(c)) ? ' ' : c;
                          })
                          | std::views::chunk_by([](const char a, const char b) {
                              return a == ' ' && b == ' ';
                          })
                          | std::views::transform([](auto &&_chunk) {
                              return *std::ranges::begin(_chunk);
                          })
                          | std::ranges::to<std::string>(); // 先转成 string 方便后续窗口扫描
        std::string finalResult;
        for (std::size_t i = 0; i < normalized.length(); ++i) {
            const char current = normalized[i];
            if (current == ' ') {
                const bool prevIsSym = (i > 0 && isSymbol(normalized[i - 1]));
                if (const bool nextIsSym = (i + 1 < normalized.length() && isSymbol(normalized[i + 1])); prevIsSym || nextIsSym) continue;
            }
            finalResult += current;
        }

        auto trimmed = finalResult
                       | std::views::drop_while(isSpace)
                       | std::views::reverse
                       | std::views::drop_while(isSpace)
                       | std::views::reverse;

        return trimmed | std::ranges::to<std::string>();
    }
}
