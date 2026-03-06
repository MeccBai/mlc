//
// Created by Administrator on 2026/2/20.
//

module Token;

import :Type;
import :Statement;
import :Expression;

import aux;
import std;
import Parser;


ast::Expression::~Expression() = default;

ast::ConstValue::ConstValue(const std::string_view _value, const bool _isChar):
    Value(processLiteral(_value, _isChar)),IsChar(_isChar)  {
    if (_isChar) {
        Type = ast::Make<Type::CompileType>(*Type::BaseTypeMap.at("i8"));
    }
    else {
        Type = GetType();
    }
}

ast::Type::sPtr<ast::Type::CompileType> ast::ConstValue::GetType() const {
    if (Value.empty()) {
        return {};
    }
    if (IsChar) {
        return std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i8"));
    }
    if (Value == "null") {
        return std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("null"));
    }
    if (std::isdigit(Value[0]) || (Value[0] == '-' && Value.size() > 1)) {
        if (Value.find('.') != std::string::npos) {
            return std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("f64"));
        }
        return std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i32"));
    }
    if (Value.front() == '"') {
        const auto strType = std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i8"));
        const auto trueType = std::make_shared<Type::PointerType>(strType, 1);
        return std::make_shared<Type::CompileType>(*trueType);
    }
    if (Value.front() == '\'') {
        return std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i8"));
    }
    return {};
}

void ast::Type::ValidateType(const std::shared_ptr<CompileType> &targetType,
                             const std::shared_ptr<CompileType> &actualType,
                             const std::string_view contextInfo, const bool _tolerance) {
    if (!targetType || !actualType) {
        ErrorPrintln("Compiler internal error.\n", contextInfo);
        std::exit(-1);
    }
    auto getName = [](const CompileType &type) -> std::string {
        return std::visit([](auto &&t) -> std::string {
            return std::string(t.Name);
        }, type);
    };
    std::string expectedName = getName(*targetType);
    std::string actualName = getName(*actualType);
    const auto targetPtr = std::get_if<PointerType>(&*targetType);
    if (const auto actualPtr = std::get_if<BaseType>(&*actualType);
        targetPtr && actualPtr && actualPtr->Name == "null") {
        return;
    }
    if (_tolerance) {
        const auto actualBase = std::get_if<BaseType>(&*actualType);
        const auto targetBase = std::get_if<BaseType>(&*targetType);
        if (actualBase && targetBase) {
            auto isInteger = [](const std::string &typeName) {
                return typeName.starts_with('i') || typeName.starts_with('u');
            };
            if (isInteger(actualBase->Name) && isInteger(targetBase->Name)) {
                return;
            }
            if (actualBase->Name.starts_with('f') && targetBase->Name.starts_with('f')) {
                return;
            }
        }
    }
    if (expectedName != actualName) {
        ErrorPrintln("Error: Type mismatch for {}. Expected '{}', got '{}'\n",
                     contextInfo, expectedName, actualName);
        std::exit(-1);
    }
}

void ast::VariableStatement::InitListValidCheck() const {
    if (!Initializer) {
        return;
    }
    const auto listPtr = std::get_if<std::shared_ptr<InitializerList> >(&*(Initializer->Storage));
    if (!listPtr) {
        return;
    }
    const auto initializerList = *listPtr;

    const auto type = this->VarType;
    const auto structType = std::get_if<Type::StructDefinition>(&(*type));
    const auto arrayType = std::get_if<Type::ArrayType>(&(*type));
    if (!structType && !arrayType) {
        ErrorPrintln("Error: Initializer list can only be used for struct or array types.\n");
        std::exit(-1);
    }

    if (arrayType || structType) {
        // 定义递归 Lambda
        auto recursiveCheck = [&](auto &self,
                                  const std::shared_ptr<Type::CompileType> &target,
                                  const std::shared_ptr<Expression> &init) -> void {
            if (const auto listPtrTemp = std::get_if<std::shared_ptr<InitializerList> >(&*(init->Storage))) {
                const auto &list = *listPtrTemp;

                // 情况 A: 目标是数组
                if (const auto arr = std::get_if<Type::ArrayType>(&(*target))) {
                    if (list->Values.size() > arr->Length) {
                        ErrorPrintln("Error: Too many initializers (expected {}, got {}).\n", arr->Length,
                                     list->Values.size());
                        std::exit(-1);
                    }
                    for (const auto &val: list->Values) {
                        self(self, arr->BaseType, val); // 递归进入下一层
                    }
                }
                // 情况 B: 目标是结构体
                else if (const auto str = std::get_if<Type::StructDefinition>(&(*target))) {
                    if (list->Values.size() > str->Members.size()) {
                        ErrorPrintln("Error: Struct '{}' has only {} members.\n", str->Name, str->Members.size());
                        std::exit(-1);
                    }
                    for (size_t i = 0; i < list->Values.size(); ++i) {
                        self(self, str->Members[i].Type, list->Values[i]); // 递归进入成员
                    }
                } else {
                    ErrorPrintln("Error: Cannot use initializer list for non-composite type.\n");
                    std::exit(-1);
                }
            } else {
                // 2. 触底反弹：这已经是一个具体的表达式了，执行最终校验
                auto typeName = std::visit([](auto &&t) { return t.Name; }, *target);
                auto tip = std::format("element of type '{}'", typeName);
                ValidateType(target, init->GetType(), tip, true);
            }
        };
        recursiveCheck(recursiveCheck, this->VarType, this->Initializer);
    }
}


bool ast::ConstExpressionCheck(const std::shared_ptr<Expression> &_expr) {
    const auto data = &(*_expr->Storage);
    if (std::get_if<ConstValue>(data) != nullptr) {
        return true;
    }
    if (const auto funcCall = std::get_if<Type::sPtr<FunctionCall> >(data); funcCall != nullptr) {
        return std::ranges::all_of((*funcCall)->Arguments, ConstExpressionCheck);
    }
    if (const auto compExpr = std::get_if<Type::sPtr<CompositeExpression> >(data); compExpr != nullptr) {
        return std::ranges::all_of((*compExpr)->Components, ConstExpressionCheck);
    }
    if (const auto varPtr = std::get_if<Type::sPtr<Variable> >(data); varPtr != nullptr) {
        return (*varPtr)->Initializer != nullptr && ConstExpressionCheck((*varPtr)->Initializer);
    }
    return false;
}
