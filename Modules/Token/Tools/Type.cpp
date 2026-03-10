//
// Created by Administrator on 2026/2/27.
//

module Token;

import :Type;
import :Statement;
import :Expression;

import aux;
import std;
import Parser;

namespace type = ast::Type;
//std::variant<BaseType, StructDefinition, EnumDefinition, PointerType, ArrayType>

bool type::IsArrayOrPointer(const sPtr<CompileType> &_type) {
    return std::holds_alternative<ArrayType>(*_type) || std::holds_alternative<PointerType>(*_type);
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

bool toleranceCheck(const std::shared_ptr<type::CompileType> &targetType,
                             const std::shared_ptr<type::CompileType> &actualType) {
    const auto *const actualBase = std::get_if<type::BaseType>(&*actualType);
    if (const auto *const targetBase = std::get_if<type::BaseType>(&*targetType); actualBase && targetBase) {
        auto isInteger = [](const std::string &typeName) {
            return typeName.starts_with('i') || typeName.starts_with('u');
        };
        if (isInteger(actualBase->Name) && isInteger(targetBase->Name)) {
            return true;
        }
        if (actualBase->Name.starts_with('f') && targetBase->Name.starts_with('f')) {
            return true;
        }
    }
    return false;
}

void ast::Type::ValidateType(const sPtr<CompileType> &_targetType,
                             const sPtr<CompileType> &_actualType,
                             const std::string_view _contextInfo, const bool _tolerance) {
    if (!_targetType || !_actualType) {
        ErrorPrintln("Compiler internal error.\n", _contextInfo);
        std::exit(-1);
    }
    std::string expectedName = GetTypeName(*_targetType);
    std::string actualName = GetTypeName(*_actualType);
    const auto *const targetPtr = GetType<PointerType>(_targetType);
    if (const auto *const actualPtr = GetType<BaseType>(_actualType);
        targetPtr && actualPtr && actualPtr->Name == "null") {
        return;
    }
    if (_tolerance && toleranceCheck(_targetType, _actualType)) {
        return;
    }
    if (expectedName != actualName) {
        ErrorPrintln("Error: Type mismatch for {}. Expected '{}', got '{}'\n",
                     _contextInfo, expectedName, actualName);
        std::exit(-1);
    }
}

void ast::VariableStatement::InitListValidCheck() const {
    if (!Initializer) {
        return;
    }
    if (const auto *const listPtr = Initializer->GetInitializerList(); !listPtr) {
        return;
    }
    const auto type = this->VarType;
    const auto *const  structType = GetType<Type::StructDefinition>(type);
    const auto *const  arrayType = GetType<Type::ArrayType>(type);
    if (!structType && !arrayType) {
        ErrorPrintln("Error: Initializer list can only be used for struct or array types.\n");
        std::exit(-1);
    }
    if (arrayType || structType) {
        // 定义递归 Lambda
        auto recursiveCheck = [&](auto &self,
                                  const std::shared_ptr<Type::CompileType> &target,
                                  const std::shared_ptr<Expression> &init) -> void {
            if (const auto *const  listPtrTemp = std::get_if<std::shared_ptr<InitializerList> >(&*(init->Storage))) {
                const auto &list = *listPtrTemp;

                // 情况 A: 目标是数组
                if (const auto *const  arr = type::GetType<Type::ArrayType>(target)) {
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
                else if (const auto *const str =type::GetType<Type::StructDefinition>(target)) {
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
                auto typeName = GetTypeName(*target);
                const auto tip = std::format("element of type '{}'", typeName);
                ValidateType(target, init->GetType(), tip, true);
            }
        };
        recursiveCheck(recursiveCheck, this->VarType, this->Initializer);
    }
}


bool ast::ConstExpressionCheck(const std::shared_ptr<Expression> &_expr) {
    if (_expr->GetConstValue() != nullptr) {
        return true;
    }
    if (const auto *const compExpr = _expr->GetCompositeExpression(); compExpr != nullptr) {
        return std::ranges::all_of((*compExpr)->Components, ConstExpressionCheck);
    }
    if (const auto *const varPtr = _expr->GetVariable(); varPtr != nullptr) {
        return (*varPtr)->Initializer != nullptr && ConstExpressionCheck((*varPtr)->Initializer);
    }
    if (const auto *const enumPtr = _expr->GetEnumValue(); enumPtr != nullptr) {
        return true;
    }
    return false;
}


std::string ast::Type::ArrayType::GetTypeName() const {
    std::string baseName = type::GetTypeName(*BaseType);
    return std::format("{}[{}]", baseName, Length);
}

ast::Type::sPtr<ast::FunctionDeclaration> ast::FunctionScope::ToDeclaration() const {
    return Make<FunctionDeclaration>(FunctionDeclaration(Name, ReturnType, Parameters, IsVarList));
}


ast::ConstValue::ConstValue(const std::string_view _value, const bool _isChar):
    Value(processLiteral(_value, _isChar)),IsChar(_isChar)  {
    if (_isChar) {
        Type = ast::Make<Type::CompileType>(*Type::BaseTypeMap.at("i8"));
    }
    else {
        Type = GetType();
    }
}