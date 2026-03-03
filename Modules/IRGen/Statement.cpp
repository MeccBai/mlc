//
// Created by Administrator on 2026/3/3.
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

std::string GenClass::StatementGenerate(const sPtr<ast::Statement> &_stmt,
                                        const sPtr<ast::FunctionDeclaration> &_decl) {
    return "\n";
}
