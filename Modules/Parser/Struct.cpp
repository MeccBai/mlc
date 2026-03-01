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
    const std::vector<std::string_view> &_structContents) const {
    std::vector<type::StructDefinition> structs;
    std::vector<std::vector<std::pair<sPtr<type::CompileType>,std::string_view>>> lazyPointerTypes(_structContents.size());

    auto trim = [](std::string_view str) {
        while (!str.empty() && std::isspace(str.front())) str.remove_prefix(1);
        while (!str.empty() && std::isspace(str.back())) str.remove_suffix(1);
        return str;
    };


    const auto findStruct = [&structs, trim](const std::string_view _typeName) {
        for (const auto &structDef: structs) {
            const auto view = trim(std::string_view(structDef.Name));
            if (const auto typeName = trim(_typeName); view == typeName) {
                return std::make_shared<ast::Type::CompileType>(structDef);
            }
        }
        return std::shared_ptr<ast::Type::CompileType>(nullptr);
    };

    const auto findType = [this, trim, findStruct](const std::string_view _typeName) {
        for (const auto &type: typeSymbolTable) {
            if (const auto basePtr = std::get_if<ast::Type::BaseType>(&*type)) {
                const auto view = trim(std::string_view(basePtr->Name));
                if (const auto typeName = trim(_typeName); view == typeName) {
                    return std::make_shared<ast::Type::CompileType>(*basePtr);
                }
            }
        }
        return findStruct(_typeName);
    };

    for (auto [structDef,lazyPtrs]: std::views::zip(_structContents, lazyPointerTypes)) {
        const auto structNameStart = structDef.find("struct") + 6; // 跳过 "struct "
        auto structName = structDef.substr(structNameStart+1, structDef.find('{')-7);
        auto memberDefs = split(
            structDef.substr(structDef.find('{') + 1, structDef.rfind('}') - structDef.find('{') - 1), ";");
        std::vector<ast::Type::StructMember> members;
        for (const auto &memberDef: memberDefs) {
            if (memberDef.empty()) continue; // 跳过空成员定义（可能是最后一个分号后面）
            bool isPointer = false;
            auto pos = memberDef.find(' ');
            if (pos == std::string_view::npos) {
                pos = memberDef.find('$');
                if (pos == std::string_view::npos) {
                    ErrorPrintln("Error: Invalid struct member definition '{}'\n", memberDef);
                    std::exit(-1);
                }
                isPointer = true;
            }
            const auto memberType = memberDef.substr(0, pos);
            const auto memberName = memberDef.substr(pos + 1 * (isPointer ? 0 : 1));
            if (memberName.find('[') != std::string_view::npos || memberName.find(']') != std::string_view::npos) {
                ErrorPrintln("Error: Struct member not allow array type '{}'\n", memberName);
                std::exit(-1);
            }
            auto typePtr = findType(memberType);
            if (isPointer) {
                auto ptr = std::make_shared<ast::Type::CompileType>(ast::Type::PointerType(1));
                lazyPtrs.emplace_back(ptr,memberType);
                members.emplace_back(std::string(memberName.substr(1)), ptr);
            } else {
                if (!typePtr) {
                    ErrorPrintln("Error : Invalid type '{}' for struct member '{}'\n", memberType, memberName);
                    std::exit(-1);
                }
                members.emplace_back(std::string(memberName), typePtr);
            }
        }
        structs.emplace_back(std::string(structName), members);
    }



    for (const auto& ptrs : lazyPointerTypes) {
        for (const auto &[ptr, name] : ptrs) {
            auto typePtr = findType(name);
            if (!typePtr) {
                typePtr = findStruct(name);
            }
            if (typePtr == nullptr) {
                ErrorPrintln("Error: Unknown type '{}' for pointer member\n", name);
                std::exit(-1);
            }
            auto* specificPtr = std::get_if<ast::Type::PointerType>(ptr.get());
            specificPtr->Finalize(typePtr);
        }
    }

    return structs;
}
