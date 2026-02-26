//
// Created by Administrator on 2026/2/26.
//

export module Generator;
import Token;
import std;

namespace mlc::ir::gen {

    export class IRGenerator {
    public:

        template<typename type>
        using sPtr = std::shared_ptr<type>;

        static std::string Struct(const sPtr<ast::Type::StructDefinition>& _structDef);

        static std::string GlobalVariable(const sPtr<ast::VariableStatement>& _variable);

        static std::string LocalVariable(const sPtr<ast::VariableStatement>& _variable);

    };

}
