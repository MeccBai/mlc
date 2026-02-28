//
// Created by Administrator on 2026/2/26.
//

export module Generator;
import Token;
import std;

namespace mlc::ir::gen {

    namespace type = ast::Type;

    export class IRGenerator {
    public:

        template<typename type>
        using sPtr = std::shared_ptr<type>;

        struct ExprResult {
            std::string llvmType;  // 比如 "i32", "ptr", "float"
            std::string resultVar; // 比如 "%1", "10", "@global_var"
            std::string code;      // 比如 "  %1 = add i32 2, 3\n"
        };

        static std::string Struct(const sPtr<ast::Type::StructDefinition>& _structDef);

        static std::string GlobalVariable(const sPtr<ast::VariableStatement>& _variable);

        static std::string LocalVariable(const sPtr<ast::VariableStatement>& _variable);

        static ExprResult ExpressionExpand(const sPtr<ast::Expression>& _expression);

        static std::string ConstExpressionExpand(const sPtr<ast::Type::CompileType>& _type,const sPtr<ast::Expression> &_expression);

        static std::string OperatorToIR(const type::BaseType* _type,ast::BaseOperator _value) ;

        static std::string TypeToLLVM(const sPtr<type::CompileType> &_type);

    };



}
