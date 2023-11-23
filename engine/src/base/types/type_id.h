#pragma once

namespace Be
{

    namespace details
    {
        struct StdTypeID final
        {
        public:
            using TypeIndex = std::type_index;

        private:
            enum EUnknown
            {
            };

        private:
            TypeIndex m_value;

        public:
            StdTypeID() noexcept : m_value{typeid(EUnknown)} {}
            explicit StdTypeID(const TypeIndex &value) noexcept : m_value{value} {}

        public:
            [[nodiscard]] auto operator<=>(const StdTypeID &) const noexcept = default;

        public:
            [[nodiscard]] TypeIndex Get() const noexcept
            {
                return m_value;
            }

            [[nodiscard]] const char *Name() const noexcept
            {
                return m_value.name();
            }
        };

        template <typename T>
        struct StdTypeIdOf final
        {
            [[nodiscard]] static StdTypeID Get() noexcept
            {
                return StdTypeID(typeid(T));
            }
        };
    }

    using TypeId = details::StdTypeID;

    template <typename T>
    [[nodiscard]] forceinline static TypeId TypeIdOf() noexcept
    {
        return details::StdTypeIdOf<T>::Get();
    }

    template <typename T>
    [[nodiscard]] forceinline static TypeId TypeIdOf(const T &) noexcept
    {
        return TypeIdOf<T>();
    }

}

template <>
struct std::hash<Be::details::StdTypeID>
{
    [[nodiscard]] size_t operator()(const Be::details::StdTypeID &value) const noexcept
    {
        return std::hash<std::type_index>{}(value.Get());
    }
};
