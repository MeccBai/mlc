//
// Created by Administrator on 2026/2/22.
//

module Parser;
import std;
import aux;

using astClass = mlc::parser::AbstractSyntaxTree;
template<typename type>
using sPtr = std::shared_ptr<type>;
namespace ast = mlc::ast;
using size_t = std::size_t;
namespace type = ast::Type;


std::vector<type::StructDefinition> astClass::structDefParser(
    const std::vector<std::string_view> &_structContents) {

    const std::vector<ast::Type::StructDefinition> structs;

    for (auto structDef: _structContents) {
        auto structName = structDef.substr(0, structDef.find('{'));
        auto memberDefs = Spilit(
            structDef.substr(structDef.find('{') + 1, structDef.rfind('}') - structDef.find('{') - 1), ";");

        std::vector<ast::Type::StructMember> members;
        for (const auto &memberDef: memberDefs) {
            const auto pos = memberDef.find(' ');
            if (pos == std::string_view::npos) {
                ErrorPrintln("Error: Invalid struct member definition '{}'\n", memberDef);
                std::exit(-1);
            }
            const auto memberType = memberDef.substr(0, pos);
            const auto memberName = memberDef.substr(pos + 1);
            bool isPointer = false;
            if (memberType[0] == '$') {
                isPointer = true;
            }


        }

    }

    return structs;
}
