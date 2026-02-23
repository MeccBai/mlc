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
    std::vector<ast::Type::StructDefinition> structs(_structContents.size());
    std::vector<std::vector< std::pair<std::shared_ptr<ast::Type::CompileType>,std::string_view> > > lazyPointerTypes(structs.size());

    for (auto [structDef,lazyPtrs]: std::views::zip(_structContents, lazyPointerTypes)) {
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
            if (memberType[1] == '$') {
                ErrorPrintln("Error: Multi-level or two level Pointer is not allowed", memberType);
                std::exit(-1);
            }
            auto typePtr = findType(memberType);

            if (isPointer) {
                auto ptr = std::make_shared<ast::Type::CompileType>(ast::Type::PointerType(memberName, 1));
                lazyPtrs.emplace_back(ptr,memberType);
                members.emplace_back(std::string(memberName), ptr);
            } else {
                if (typePtr.expired()) {
                    ErrorPrintln("Error : Invalid type '{}' for struct member '{}'\n", memberType, memberName);
                    std::exit(-1);
                }
            }
        }
        structs.emplace_back(std::string(structName), members);
    }

    auto findStruct = [structs](std::string_view typeName) {
      for (const auto &structDef: structs) {
          if (structDef.Name == typeName) {
              return std::make_shared<ast::Type::CompileType>(structDef);
          }
      }
      return std::shared_ptr<ast::Type::CompileType>(nullptr);
    };

    for (const auto& ptrs : lazyPointerTypes) {
        for (const auto &[ptr, name] : ptrs) {
            auto typePtr = findType(name);
            if (typePtr.expired()) {
                typePtr = findStruct(name);
            }
            if (typePtr.expired()) {
                ErrorPrintln("Error: Unknown type '{}' for pointer member\n", name);
                std::exit(-1);
            }
            auto* specificPtr = std::get_if<ast::Type::PointerType>(ptr.get());
            specificPtr->Finalize(typePtr);
        }
    }

    return structs;
}
