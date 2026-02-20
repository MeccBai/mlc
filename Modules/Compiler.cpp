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

    auto matchWord = [&](std::string_view word) -> bool {
        if (cursor + word.length() > length) return false;
        // Check if the substring matches
        if (_source.substr(cursor, word.length()) != word) return false;
        // Check if the next character is not an identifier character (ensure whole word match)
        if (cursor + word.length() < length && isIdentifierChar(_source[cursor + word.length()])) return false;
        return true;
    };

    auto skipBlock = [&](char open, char close) -> bool {
        int depth = 0;
        if (cursor < length && _source[cursor] == open) {
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
            // struct could be a definition "struct A { ... };" or a variable decl "struct A x;"
            // But usually at top level starting with struct implies a definition or declaration.
            // Let's look ahead.
            // Actually user logic: "先匹配enum / stuct /typedef这三者"
            // If it starts with struct, treat as struct definition for now, or just consume until semicolon.
            // Struct definitions end with semicolon.
            type = ast::GlobalStatement::StructDefinition;
            findSemicolon();
        } else if (matchWord("enum")) {
            type = ast::GlobalStatement::EnumDefinition;
            findSemicolon();
        } else {
            // "其余情况" - Function or Variable
            // Logic: Scan ahead. If we encounter '(', it's likely a function.
            // If we encounter ';' first, it's a variable.

            bool isFunction = false;
            size_t probe = cursor;

            // We need to carefully probe without consuming, but skipping blocks like strings or nested parenthesis could be complex.
            // Simplified probe:
            while (probe < length) {
                if (_source[probe] == '(') {
                    isFunction = true;
                    break;
                }
                if (_source[probe] == ';' || _source[probe] == '{') { // '{' should not happen before '(' in function/variable at top level usually, unless initializing array? "int a[] = {1};"
                    // If we hit '{' before '(', it's an initialization, so it's a variable.
                    // If we hit ';' before '(', it's a variable.
                    isFunction = false;
                    break;
                }
                probe++;
            }

            if (isFunction) {
                // Determine if Definition or Declaration
                // Move cursor to probe (the '(')
                // Note: we need to consume the part before '('
                cursor = probe;

                // Skip the parameter list
                // We are at '(', skipBlock will consume it and closing ')'
                if (!skipBlock('(', ')')) {
                     // Error or incomplete
                     // Just consume until end or semicolon to recover?
                }

                skipSpaces();
                if (cursor < length && _source[cursor] == ';') {
                    type = ast::GlobalStatement::FunctionDeclaration;
                    cursor++; // Consume ';'
                } else if (cursor < length && _source[cursor] == '{') {
                    type = ast::GlobalStatement::FunctionDefinition;
                    skipBlock('{', '}');
                } else {
                    // Unexpected (e.g. macro or error), treat like variable until semicolon
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