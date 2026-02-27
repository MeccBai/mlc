//
// Created by Administrator on 2026/2/27.
//

export module Token:Type;


import std;
using size_t = std::size_t;
export namespace mlc::ast::Type {
    class BaseType;
    class StructDefinition;
    class EnumDefinition;
    class PointerType;
    class ArrayType;

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
        }

        const std::string Name;
        const std::vector<std::string> Values;

        static std::size_t Size() {
            return 4;
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
        const std::size_t Length; // 0 for incomplete array
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


    void ValidateType(const std::shared_ptr<CompileType> &targetType,
                      const std::shared_ptr<CompileType> &actualType,
                      std::string_view contextInfo);

} // namespace mlc::ast::Type
