//
// Created by Administrator on 2026/2/20.
//

module Compiler;

import std;
import Token;

namespace seg = mlc::seg;
namespace ast = mlc::ast;
using size_t = std::size_t;


inline bool isIdentifierChar(char c) {
    return std::isalnum(c) || c == '_';
}


auto seg::TopTokenize(const std::string_view _source) -> std::vector<TokenStatement> {
    std::vector<TokenStatement> fragments;
    std::size_t cursor = 0;
    const std::size_t length = _source.length();

    auto skipSpaces = [&]() {
        while (cursor < length && std::isspace(_source[cursor])) {
            cursor++;
        }
    };

    auto matchWord = [&](const std::string_view word) -> bool {
        if (cursor + word.length() > length) return false;
        if (_source.substr(cursor, word.length()) != word) return false;
        if (cursor + word.length() < length && isIdentifierChar(_source[cursor + word.length()])) return false;
        return true;
    };

    auto skipBlock = [&](const char open, const char close) -> bool {
        if (cursor < length && _source[cursor] == open) {
            int depth = 0;
            depth = 1;
            cursor++;
            while (cursor < length && depth > 0) {
                if (_source[cursor] == open) {
                    depth++;
                } else if (_source[cursor] == close) {
                    depth--;
                }
                cursor++;
            }
            return depth == 0;
        }
        return false;
    };

    auto findSemicolon = [&]() {
        while (cursor < length) {
            if (_source[cursor] == ';') {
                cursor++; // Consume semicolon
                break;
            } else if (_source[cursor] == '{') {
                skipBlock('{', '}');
            } else {
                cursor++;
            }
        }
    };

    while (cursor < length) {
        skipSpaces();
        if (cursor >= length) break;

        const std::size_t start = cursor;
        ast::GlobalStatement type = ast::GlobalStatement::VariableDeclaration; // Default

        if (matchWord("typedef")) {
            type = ast::GlobalStatement::Typedef;
            findSemicolon();
        } else if (matchWord("struct")) {
            type = ast::GlobalStatement::StructDefinition;
            findSemicolon();
        } else if (matchWord("enum")) {
            type = ast::GlobalStatement::EnumDefinition;
            findSemicolon();
        } else {
            matchWord("static");
            matchWord("extern");
            bool isFunction = false;
            size_t probe = cursor;
            while (probe < length) {
                if (_source[probe] == '(') {
                    isFunction = true;
                    break;
                }
                if (_source[probe] == ';' || _source[probe] == '{') {
                    isFunction = false;
                    break;
                }
                probe++;
            }

            if (isFunction) {
                cursor = probe;
                if (!skipBlock('(', ')')) {
                }
                skipSpaces();
                if (cursor < length && _source[cursor] == ';') {
                    type = ast::GlobalStatement::FunctionDeclaration;
                    cursor++; // Consume ';'
                } else if (cursor < length && _source[cursor] == '{') {
                    type = ast::GlobalStatement::FunctionDefinition;
                    skipBlock('{', '}');
                } else {
                    findSemicolon();
                    type = ast::GlobalStatement::VariableDeclaration;
                }
            } else {
                type = ast::GlobalStatement::VariableDeclaration;
                findSemicolon();
            }
        }

        fragments.push_back({type, _source.substr(start, cursor - start)});
    }

    return fragments;
}
