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

        struct ExprResult {
            std::string code; // 生成的 IR 代码
            std::string resultVar; // 存储结果的变量名（如果有）
        };

        static std::string Struct(const sPtr<ast::Type::StructDefinition>& _structDef);

        static std::string GlobalVariable(const sPtr<ast::VariableStatement>& _variable);

        static std::string LocalVariable(std::string_view _functionName,const sPtr<ast::VariableStatement>& _variable);

        static ExprResult ExpressionExpand(const sPtr<ast::Expression>& _expression);

        static std::string ConstExpressionExpand(const sPtr<ast::Type::CompileType>& _type,const sPtr<ast::Expression> &_expression);
        static std::string OperatorToIR(std::vector<ast::BaseOperator>::value_type value) ;

    };



}
