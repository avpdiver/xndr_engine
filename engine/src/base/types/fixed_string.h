#pragma once

namespace Be
{

    template <typename CharType, usize_t StringSize>
        requires(StringSize <= 512)
    struct TFixedString
    {
    public:
        static constexpr usize_t FixedSize = StringSize;

    public:
        using value_type = CharType;
        using iterator = CharType *;
        using const_iterator = CharType const *;

    public:
        using ViewType = BasicStringView<CharType>;
        using Self = TFixedString<CharType, StringSize>;

    private:
        using LengthType = Conditional<alignof(CharType) < 4, byte_t, uint32_t>;

    public:
        constexpr TFixedString() noexcept
        {
        }

        constexpr TFixedString(const ViewType &view) noexcept
            : TFixedString{view.data(), view.length()}
        {
        }

        constexpr TFixedString(const CharType *str) noexcept
        {
            for (; str[m_length] && m_length < StringSize; ++m_length)
            {
                m_array[m_length] = str[m_length];
            }
            m_array[m_length] = CharType{0};
        }

        constexpr TFixedString(const CharType *str, usize_t length) noexcept
        {
            ASSERT(length < StringSize);

            for (; m_length < length && m_length < StringSize; ++m_length)
            {
                m_array[m_length] = str[m_length];
            }
            m_array[m_length] = CharType{0};
        }

        constexpr TFixedString(Self &&) noexcept = default;
        constexpr TFixedString(const Self &) noexcept = default;

        constexpr ~TFixedString() noexcept = default;

    public:
        Self &operator=(Self &&) noexcept = default;
        Self &operator=(const Self &) noexcept = default;
        Self &operator=(const CharType *str) noexcept
        {
            m_length = 0;
            for (; str[m_length] && m_length < StringSize; ++m_length)
            {
                m_array[m_length] = str[m_length];
            }
            m_array[m_length] = CharType{0};
            return (*this);
        }

    public:
        [[nodiscard]] constexpr operator ViewType() const noexcept
        {
            return ViewType{data(), length()};
        }

    public:
        [[nodiscard]] constexpr usize_t size() const noexcept
        {
            return m_length;
        }

        [[nodiscard]] constexpr usize_t length() const noexcept
        {
            return size();
        }

        [[nodiscard]] static constexpr usize_t capacity() noexcept
        {
            return StringSize;
        }

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return m_length == 0;
        }

        [[nodiscard]] constexpr CharType const *c_str() const noexcept
        {
            return m_array; // always non-null
        }

        [[nodiscard]] constexpr CharType const *data() const noexcept
        {
            return m_array;
        }

        [[nodiscard]] CharType *data() noexcept
        {
            return m_array;
        }

    public:
        [[nodiscard]] constexpr CharType &operator[](usize_t i) noexcept
        {
            ASSERT(i < m_length);
            return m_array[i];
        }

        [[nodiscard]] constexpr CharType const &operator[](usize_t i) const noexcept
        {
            ASSERT(i < m_length);
            return m_array[i];
        }

    public:
        [[nodiscard]] constexpr bool operator==(const ViewType &rhs) const noexcept
        {
            return ViewType(*this) == rhs;
        }

        [[nodiscard]] constexpr bool operator!=(const ViewType &rhs) const noexcept
        {
            return not(*this == rhs);
        }

        [[nodiscard]] constexpr bool operator>(const ViewType &rhs) const noexcept
        {
            return ViewType(*this) > rhs;
        }

        [[nodiscard]] constexpr bool operator<(const ViewType &rhs) const noexcept
        {
            return ViewType(*this) < rhs;
        }

    public:
        Self &operator<<(CharType value) noexcept
        {
            push_back(value);
            return *this;
        }

    public:
        [[nodiscard]] iterator begin() noexcept
        {
            return &m_array[0];
        }

        [[nodiscard]] const_iterator begin() const noexcept
        {
            return &m_array[0];
        }

        [[nodiscard]] iterator end() noexcept
        {
            return &m_array[m_length];
        }

        [[nodiscard]] const_iterator end() const noexcept
        {
            return &m_array[m_length];
        }

    public:
        void clear() noexcept
        {
            m_array[0] = CharType(0);
            m_length = 0;
        }

        void resize(usize_t new_size) noexcept
        {
            ASSERT(new_size < capacity());

            new_size = std::min(new_size, capacity() - 1);

            if (new_size > m_length)
            {
                std::memset(&m_array[m_length], 0, new_size - m_length + 1);
            }

            m_length = LengthType(new_size);
        }

        void push_back(CharType value) noexcept
        {
            ASSERT(usize_t{m_length} + 1u < capacity());

            m_array[m_length] = value;
            m_array[m_length + 1] = CharType{0};
            ++m_length;
        }

    private:
        LengthType m_length = 0;
        CharType m_array[StringSize] = {};
    };

    template <usize_t StringSize>
    using FixedString = TFixedString<char, StringSize>;

}

template <typename CharType, size_t StringSize>
struct std::hash<Be::TFixedString<CharType, StringSize>>
{
    [[nodiscard]] size_t operator()(const Be::TFixedString<CharType, StringSize> &value) const noexcept
    {
        return hash<Be::BasicStringView<CharType>>()(value);
    }
};
