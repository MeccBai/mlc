//
// Created by Administrator on 2026/2/27.
//

export module Token:Type;


import std;
import aux;
using size_t = std::size_t;
export namespace mlc::ast::Type {
    template <typename type>
    using sPtr = std::shared_ptr<type>;

    class BaseType;
    class StructDefinition;
    class EnumDefinition;
    class PointerType;
    class ArrayType;

    std::unordered_set<std::string_view> KeyWords = {
        "struct", "enum", "void", "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64",
        "nullptr","switch","case","default","break","continue","if","else","while","return"
    };

    void IsValidName(const std::string_view _name) {
        bool result = true;
        if (_name.empty()) result =  false;
        if (KeyWords.contains(_name)) result = false;
        if (!std::isalpha(_name[0]) && _name[0] != '_') result = false;
        for (const char ch: _name) {
            if (!std::isalnum(ch) && ch != '_') result = false;
        }
        if (!result) {
            ErrorPrintln("Error: Invalid name '{}'\n", _name);
            std::exit(-1);
        }
    }

    using CompileType = std::variant<BaseType, StructDefinition, EnumDefinition, PointerType, ArrayType>;

    class BaseType {
    public:
        explicit BaseType(const std::string_view _name, const std::size_t _size) : Name(_name), size(_size) {
        }

        const std::string Name;
        [[nodiscard]] std::size_t Size() const { return size; }

        bool operator ==(const BaseType &_other) const {
            return _other.Name == Name;
        }

        bool operator >(const BaseType &_other) const {
            return size > _other.Size();
        }

        bool operator <(const BaseType &_other) const {
            return size < _other.Size();
        }

    private:
        const std::size_t size;
    };

    class EnumDefinition {
    public:
        explicit EnumDefinition(const std::string_view _name, std::vector<std::string> &_values) : Name(_name),
            Values(std::move(_values)) {
            IsValidName(Name);
            std::ranges::for_each(Values,[](const std::string_view _name){IsValidName(_name);});
        }

        const std::string Name;
        const std::vector<std::string> Values;

        static std::size_t Size() {
            return 4;
        }

        [[nodiscard]] std::optional<size_t> GetValueIndex(const std::string_view _value) const {
            for (size_t i = 0; i < Values.size(); ++i) {
                if (Values[i] == _value) {
                    return i;
                }
            }
            return std::nullopt;
        }

    };

    struct StructMember {
        std::string Name;
        std::shared_ptr<CompileType> Type;
    };

    class StructDefinition {
    public:
        explicit StructDefinition(const std::string_view _name, std::vector<StructMember> &_members,
                                  const bool _isExported = false)
            : Name(_name), Members(std::move(_members)), IsExported(_isExported) {
            IsValidName(Name);
            std::ranges::for_each(Members, [](const StructMember &member) {
                IsValidName(member.Name);
            });
        }

        const std::string Name;
        const std::vector<StructMember> Members; // type and name
        const bool IsExported;

        [[nodiscard]] size_t Size() const;
    };

    class ArrayType {
    public:
        [[nodiscard]] std::string GetTypeName() const;

        explicit ArrayType(std::shared_ptr<CompileType> _baseType,
                           std::size_t _length) : BaseType(std::move(_baseType)),
                                                  Length(_length), Name(this->GetTypeName()) {
        }

        [[nodiscard]] std::weak_ptr<CompileType> GetBaseType() const {
            return BaseType;
        }

        const std::shared_ptr<CompileType> BaseType;
        const std::size_t Length;
        const std::string Name;

        [[nodiscard]] size_t Size() const;
    };

    class PointerType {
    public:
        explicit PointerType(std::shared_ptr<CompileType> _baseType, const size_t _pointerLevel = 1)
            : PointerLevel(_pointerLevel), BaseType(std::move(_baseType)) {
        }

        explicit PointerType(const size_t _pointerLevel = 1) : PointerLevel(_pointerLevel) {
        }

        const std::string Name;

        void Finalize(std::shared_ptr<CompileType> _baseType) {
            BaseType = std::move(_baseType);
            const std::string baseName = std::visit([](auto &&t) -> std::string {
                return std::string(t.Name);
            }, *BaseType);
            const std::string suffix(PointerLevel, '$');
            const_cast<std::string &>(Name) = baseName + suffix;
        }

        [[nodiscard]] std::string GetTypeName() const {
            return Name;
        }

        [[nodiscard]] std::weak_ptr<CompileType> GetBaseType() const {
            return BaseType;
        }

        const size_t PointerLevel;

        std::shared_ptr<CompileType> BaseType;

        static std::size_t Size() {
            return 8;
        }
    };

    ArrayType * GetArrayType(const std::shared_ptr<CompileType> &_type);
    BaseType * GetBaseType(const std::shared_ptr<CompileType> &_type);
    StructDefinition * GetStructDef(const std::shared_ptr<CompileType> &_type);
    EnumDefinition * GetEnumDef(const std::shared_ptr<CompileType> &_type);
    PointerType * GetPointerType(const std::shared_ptr<CompileType> &_type);

    template<typename _type>
    concept IsCompileType =requires
    {
        requires (
          std::is_same_v<std::remove_cvref_t<_type>, BaseType> ||
          std::is_same_v<std::remove_cvref_t<_type>, StructDefinition> ||
          std::is_same_v<std::remove_cvref_t<_type>, EnumDefinition> ||
          std::is_same_v<std::remove_cvref_t<_type>, PointerType> ||
            std::is_same_v<std::remove_cvref_t<_type>, ArrayType>
        );
    };

    template<IsCompileType _compileType>
    sPtr<CompileType> MakeCompileType(_compileType && _type) {
        return std::make_shared<CompileType>(std::forward<_compileType>(_type));
    }


    template<IsCompileType _compileType>
    sPtr<CompileType> MakeCompileType(_compileType & _type) {
        return std::make_shared<CompileType>(_type);
    }

    void ValidateType(const std::shared_ptr<CompileType> &targetType,
                      const std::shared_ptr<CompileType> &actualType,
                      std::string_view contextInfo);

    bool IsArrayOrPointer (const sPtr<CompileType>& _type );

    size_t GetSize (const sPtr<CompileType>& _type );


} // namespace mlc::ast::Type
