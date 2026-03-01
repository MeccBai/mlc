//
// Created by Administrator on 2026/2/26.
//

export module Generator;
import Token;
import std;

namespace mlc::ir::gen {

    using size_t = std::size_t;

    namespace type = ast::Type;

    export class IRGenerator {
    public:

        template<typename type>
        using sPtr = std::shared_ptr<type>;

        using expr =  sPtr<ast::Expression>;

        struct ExprResult {
            std::string llvmType;  // 比如 "i32", "ptr", "float"
            std::string resultVar; // 比如 "%1", "10", "@global_var"
            std::string code;      // 比如 "  %1 = add i32 2, 3\n"
        };

        static size_t exprCnt;

        static std::string Struct(const sPtr<ast::Type::StructDefinition>& _structDef);

        static std::string GlobalVariable(const sPtr<ast::VariableStatement>& _variable);

        static std::string LocalVariable(const sPtr<ast::VariableStatement>& _variable);

        static ExprResult ExpressionExpand(sPtr<ast::Expression>& _expression);

        static std::string ConstExpressionExpand(const sPtr<ast::Type::CompileType>& _type,const sPtr<ast::Expression> &_expression);

        static std::string OperatorToIR(const type::BaseType* _type,ast::BaseOperator _value) ;

        static std::string TypeToLLVM(const sPtr<type::CompileType> &_type);

        static std::string TripleExpression(const expr& _left,const expr&_right, ast::BaseOperator _op);

        static ExprResult BinaryExpression(const expr& _expr,ast::BaseOperator _op);

    };

    std::string determineCastOperator(const type::BaseType *_sourceType, const type::BaseType *_targetType);

}
