//
// Created by Administrator on 2026/2/23.
//

module Parser;
import std;
import aux;

namespace ast = mlc::ast;
using size_t = std::size_t;
using astClass = mlc::parser::AbstractSyntaxTree;

std::vector<ast::Statement> mlc::parser::AbstractSyntaxTree::caseBlockParser(
    ContextTable<ast::VariableStatement> &_context, std::string_view statementContent) {
}

ast::SubScope astClass::subScopeParser(ContextTable<ast::VariableStatement> &_context,
                                       const std::string_view _subScopeContent) {

    if (_subScopeContent.find("switch(") == 0) {
        auto pos = _subScopeContent.find('){');
        auto condition = _subScopeContent.substr(7, pos - 7);

    }
    else {
        if (_subScopeContent.find("if(")) {


        }
        else if (_subScopeContent.find("while(")) {

        }
        else if (_subScopeContent.find("do{")) {

        }
    }

    return ast::SubScope({},{},{});
}
