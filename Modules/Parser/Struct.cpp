//
// Created by Administrator on 2026/2/22.
//

module Parser;
import std;
import aux;
import :Decl;


struct structPack {
    std::string_view name;
    std::vector<std::string_view> memberDefs;
};

struct structMemberPack {
    std::string name;
    std::string typeName;
    bool isPointer;
};

structPack parseStructDef(const std::string_view _structDef) {

    const auto structDefine = _structDef;
    const auto structNameStart = structDefine.find("struct") + 6; // 跳过 "struct "
    const auto structName = structDefine.substr(structNameStart+1, structDefine.find('{')-7);
    auto memberDefs = split(
        structDefine.substr(structDefine.find('{') + 1, structDefine.rfind('}') - structDefine.find('{') - 1), ";");
    memberDefs.pop_back();
    return {structName, memberDefs};
}

structMemberPack parseStructMember(const std::string_view _memberDef) {
    if (_memberDef.empty()) {
        ErrorPrintln("Error: Empty struct member definition\n");
        std::exit(-1);
    }
    bool isPointer = false;
    auto pos = _memberDef.find(' ');
    if (pos == std::string_view::npos) {
        pos = _memberDef.find('$');
        if (pos == std::string_view::npos) {
            ErrorPrintln("Error: Invalid struct member definition '{}'\n", _memberDef);
            std::exit(-1);
        }
        isPointer = true;
    }
    const auto memberType = _memberDef.substr(0, pos);
    const auto memberName = _memberDef.substr(pos + 1 * (isPointer ? 0 : 1));
    if (memberName.find('[') != std::string_view::npos || memberName.find(']') != std::string_view::npos) {
        ErrorPrintln("Error: Struct member not allow array type '{}'\n", memberName);
        std::exit(-1);
    }
    if (isPointer) {
        return {
            std::string(memberName.substr(1)),
            std::string(memberType), true};
    }
    return {std::string(memberName),std::string(memberType) ,false};
}


std::vector<type::StructDefinition> astClass::structDefParser(
    const std::vector<seg::TokenStatement> &_structContents) const {
    std::vector<type::StructDefinition> structs;
    std::vector<std::vector<std::pair<sPtr<type::CompileType>,std::string>>> lazyPointerTypes(_structContents.size());

    const auto findStruct = [&structs](const std::string_view _typeName) {
        for (const auto &structDef: structs) {
            const auto view = std::string_view(structDef.Name);
            if (const auto typeName = _typeName; view == typeName) {
                return ast::Make<type::CompileType>(structDef);
            }
        }
        return sPtr<ast::Type::CompileType>(nullptr);
    };

    const auto findType = [this, findStruct](const std::string_view _typeName) {
        for (const auto &type: typeSymbolTable) {
            if (auto *const basePtr = std::get_if<ast::Type::BaseType>(&*type)) {
                const auto view = std::string_view(basePtr->Name);
                if (const auto typeName = _typeName; view == typeName) {
                    return std::make_shared<ast::Type::CompileType>(*basePtr);
                }
            }
        }
        return findStruct(_typeName);
    };

    for (auto [structToken,lazyPtrs]: std::views::zip(_structContents, lazyPointerTypes)) {
        auto [_,structDef, isExported] = structToken;
        auto [structName, memberDefs] = parseStructDef(structDef);
        lazyPtrs.clear();
        auto members = std::vector<type::StructMember>{};
        for (auto membersViews = memberDefs | std::views::transform(parseStructMember);
            const auto&[name, typeName, isPointer]:membersViews) {
            if (isPointer) {
                auto ptr = std::make_shared<ast::Type::CompileType>(ast::Type::PointerType(1));
                lazyPtrs.emplace_back(ptr, typeName);
                //std::println("{}",typeName);
                members.emplace_back(std::string(name), ptr);
            }
            else {
                auto typePtr = findType(typeName);
                if (!typePtr) {
                    ErrorPrintln("Error: Unknown type '{}' for struct member '{}'\n", typeName, name);
                    std::exit(-1);
                }
                members.emplace_back(std::string(name), typePtr);
            }
        }
        structs.emplace_back(structName, members,isExported);
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
