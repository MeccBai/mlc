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

        using expr = sPtr<ast::Expression>;

        struct ExprResult {
            std::string llvmType; // 比如 "i32", "ptr", "float"
            std::string resultVar; // 比如 "%1", "10", "@global_var"
            std::string code; // 比如 "  %1 = add i32 2, 3\n"
            bool isCopyResult = false; // 是否需要通过内存访问来获取结果（比如结构体成员访问）
        };

        struct FuncResult {
            bool isCopyResult; // 是否需要调用 llvm.memcpy 来复制返回值
            std::string llvmType; // 返回值的 LLVM 类型
            std::string resultVar; // 返回值所在的变量（可能是寄存器或者全局变量）
            std::string code; // 可能需要的额外代码（比如复制结构体返回值的 memcpy 调用）
            std::vector<std::string> llvmTypes; // 参数的llvmType
            std::string functionDecl; // 函数声明（如果需要）
            bool isVarList = false;
        };

        struct FuncArg {
            bool isCasting; //是否被转为了i64
            bool isMemoryArg; //是否是内存参数（需要传递指针）
            size_t size; //参数大小
            std::string llvmType;
            std::string originalType; //原始类型（用于生成 memcpy 代码）
            std::string resultVar;
        };

        struct FuncHeader {
            FuncResult funcResult;
            std::vector<FuncArg> args;
        };

        struct FuncCall {
            bool isCopyResult; // 是否需要调用 llvm.memcpy 来复制返回值
            std::string llvmType; // 返回值的 LLVM 类型
            std::string resultVar; // 返回值所在的变量（可能是寄存器或者全局变量）
            std::string callCode; // 返回值的 LLVM 类型
        };

        static size_t exprCnt;

        static std::string globalCode;

        static std::string Struct(const sPtr<ast::Type::StructDefinition> &_structDef);

        static std::string GlobalVariable(const sPtr<ast::VariableStatement> &_variable);

        static std::string LocalVariable(const sPtr<ast::VariableStatement> &_variable);

        static ExprResult ExpressionExpand(const sPtr<ast::Expression> &_expression);

        static std::string ConstExpressionExpand(const sPtr<ast::Type::CompileType> &_type,
                                                 const sPtr<ast::Expression> &_expression);

        static std::string OperatorToIR(const type::BaseType *_type, ast::BaseOperator _value);

        static std::string OperatorToIR(std::string_view _llvmType, ast::BaseOperator _value);

        static std::string TypeToLLVM(const sPtr<type::CompileType> &_type);

        static ExprResult TripleExpression(const expr &_left, const expr &_right, ast::BaseOperator _op);

        static ExprResult TripleExpression(const ExprResult &_left, const ExprResult &_right, ast::BaseOperator _op);

        static ExprResult BinaryExpression(const expr &_expr, ast::BaseOperator _op);

        static ExprResult GradientExpression(const std::vector<ExprResult> &_expr,
                                             const std::vector<ast::BaseOperator> &_ops);

        static ExprResult MemberAccessExpression(const expr &_base);

        static ExprResult MemberAccessBinary(const type::CompileType *_type, const ExprResult &_parent,
                                             const expr &_child, ast::BaseOperator _op);

        static FuncResult FunctionUnit(const sPtr<ast::FunctionDeclaration> &_funcDecl);

        static std::string FunctionDeclarationGenerate(const sPtr<ast::FunctionDeclaration> &_funcDecl);

        static FuncCall FunctionCall(const sPtr<ast::FunctionCall> &_funcCall);

        static ExprResult FunctionArg(FuncArg & _funcArg,size_t _index);

        static std::string FunctionGenerate(const sPtr<ast::FunctionScope> &_func);

        static FuncArg FunctionArgAnalyze(const ast::VariableStatement &_param);

        static std::string StatementGenerate(const sPtr<ast::Statement> &_stmt,const sPtr<ast::FunctionDeclaration>& _decl);

        ExprResult InitializerListExpression(const sPtr<ast::InitializerList> &_initList);
    };

    export
    {
        constexpr std::string_view llvmCopy = "@llvm.memcpy.p0.p0.i64";
        constexpr std::string_view llvmSet = "@llvm.memset.p0.i64";
    }

    std::string determineCastOperator(const type::BaseType *_sourceType, const type::BaseType *_targetType);
}
