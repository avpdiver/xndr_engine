#pragma once

namespace Be
{

    template <typename ID, usize_t SIZE, bool OPTIMIZED, uint32_t SEED = UMax>
    struct NamedHandle
    {
    public:
        static constexpr usize_t Size = SIZE;

    public:
        using Self = NamedHandle<ID, SIZE, OPTIMIZED, SEED>;
        using ValueType = HashValue;
        using WithStringType = NamedHandle<ID, SIZE, false, SEED>;
        using OptimizedType = NamedHandle<ID, SIZE, true, SEED>;

    public:
        using HasherType = std::hash<Self>;

    public:
        struct EqualToType
        {
            [[nodiscard]] bool operator()(const Self &lhs, const Self &rhs) const noexcept
            {
                return lhs.m_hash == rhs.m_hash;
            }
        };

    public:
        static constexpr usize_t MaxStringLength = SIZE;
        static constexpr uint32_t Seed = SEED;
        static constexpr bool IsOptimized = OPTIMIZED;

    private:
        static constexpr HashValue EmptyHash = ConstHashOf("", 0, SEED);

    public:
        constexpr NamedHandle() noexcept
            : m_hash{EmptyHash}
        {
        }

        explicit constexpr NamedHandle(HashValue hash) noexcept
            : m_hash{hash}
        {
        }

        explicit constexpr NamedHandle(StringView name) noexcept
            : m_hash{ConstHashOf(name.data(), name.length(), Seed)}
        {
        }

        explicit constexpr NamedHandle(const char *name) noexcept
            : m_hash{ConstHashOf(name, SIZE, Seed)}
        {
        }

        template <usize_t StringSize>
        explicit constexpr NamedHandle(const FixedString<StringSize> &name) noexcept
            : m_hash{ConstHashOf(name.data(), std::min(SIZE, StringSize), Seed)}
        {
        }

        constexpr NamedHandle(Self &&) noexcept = default;
        constexpr NamedHandle(const Self &) noexcept = default;

    public:
        constexpr Self &operator=(Self &&) noexcept = default;
        constexpr Self &operator=(const Self &) noexcept = default;

        // template <usize_t StringSize>
        // constexpr Self &operator=(const FixedString<StringSize> *name) noexcept
        //     : m_hash{ConstHashOf(name.data(), std::min<UMax, StringSize>, Seed)}
        // {
        // }

    public:
        [[nodiscard]] auto operator<=>(const Self &) const = default;

    public:
        [[nodiscard]] constexpr HashValue Hash() const noexcept
        {
            return m_hash;
        }

        [[nodiscard]] constexpr bool IsDefined() const noexcept
        {
            return m_hash != EmptyHash;
        }

    private:
        HashValue m_hash{};
    };

    template <typename ID, usize_t SIZE, uint32_t SEED>
    struct NamedHandle<ID, SIZE, false, SEED>
    {
    public:
        static constexpr usize_t Size = SIZE;

    public:
        using Self = NamedHandle<ID, SIZE, false, SEED>;
        using ValueType = HashValue;
        using OptimizedType = NamedHandle<ID, SIZE, true, SEED>;
        using WithStringType = NamedHandle<ID, SIZE, false, SEED>;

    public:
        using HasherType = std::hash<Self>;

    public:
        struct EqualToType
        {
            [[nodiscard]] bool operator()(const Self &lhs, const Self &rhs) const noexcept
            {
                return lhs.m_hash == rhs.m_hash and lhs.m_name == rhs.m_name;
            }
        };

    public:
        static constexpr usize_t MaxStringLength = SIZE;
        static constexpr uint32_t Seed = SEED;
        static constexpr bool IsOptimized = false;

    private:
        static constexpr HashValue EmptyHash = ConstHashOf("", 0, SEED);

    public:
        constexpr NamedHandle() noexcept : m_hash{EmptyHash}
        {
        }

        explicit constexpr NamedHandle(HashValue hash) noexcept
            : m_hash{hash}
        {
        }

        explicit constexpr NamedHandle(StringView name) noexcept
            : m_hash{ConstHashOf(name.data(), name.length(), Seed)},
              m_name{name}
        {
        }

        explicit constexpr NamedHandle(const char *name) noexcept
            : m_hash{ConstHashOf(name, UMax, Seed)},
              m_name{name}
        {
        }

        template <usize_t StrSize>
        explicit constexpr NamedHandle(const FixedString<StrSize> &name) noexcept
            : m_hash{ConstHashOf(name.data(), name.length(), Seed)},
              m_name{name}
        {
        }

        explicit constexpr NamedHandle(const OptimizedType &other) noexcept
            : m_hash{other.Hash()}
        {
        }

        constexpr NamedHandle(Self &&) noexcept = default;
        constexpr NamedHandle(const Self &) noexcept = default;

    public:
        constexpr Self &operator=(Self &&) noexcept = default;
        constexpr Self &operator=(const Self &) noexcept = default;

    public:
        [[nodiscard]] auto operator<=>(const Self &) const = default;

    public:
        [[nodiscard]] constexpr operator OptimizedType() const noexcept
        {
            return OptimizedType{Hash()};
        }

        [[nodiscard]] constexpr StringView GetName() const noexcept
        {
            return m_name;
        }

        [[nodiscard]] constexpr HashValue Hash() const noexcept
        {
            return m_hash;
        }

        [[nodiscard]] constexpr bool IsDefined() const noexcept
        {
            return m_hash != EmptyHash;
        }

    private:
        HashValue m_hash;
        FixedString<SIZE> m_name;
    };

    template <typename UID, usize_t Size, uint32_t Seed>
    [[nodiscard]] inline bool operator==(const NamedHandle<UID, Size, false, Seed> &lhs, const NamedHandle<UID, Size, true, Seed> &rhs) noexcept
    {
        return lhs.Hash() == rhs.Hash();
    }

    template <typename UID, usize_t Size, uint32_t Seed>
    [[nodiscard]] inline bool operator==(const NamedHandle<UID, Size, true, Seed> &lhs, const NamedHandle<UID, Size, false, Seed> &rhs) noexcept
    {
        return lhs.Hash() == rhs.Hash();
    }

    namespace details
    {
        template <typename UID, usize_t Size, uint32_t Seed>
        struct MemCopyAvailable<NamedHandle<UID, Size, true, Seed>>
        {
            static constexpr bool value = true;
        };
        template <typename UID, usize_t Size, uint32_t Seed>
        struct ZeroMemAvailable<NamedHandle<UID, Size, true, Seed>>
        {
            static constexpr bool value = false;
        };
        template <typename UID, usize_t Size, uint32_t Seed>
        struct TriviallySerializable<NamedHandle<UID, Size, true, Seed>>
        {
            static constexpr bool value = true;
        };

        template <typename UID, usize_t Size, uint32_t Seed>
        struct MemCopyAvailable<NamedHandle<UID, Size, false, Seed>>
        {
            static constexpr bool value = true;
        };
        template <typename UID, usize_t Size, uint32_t Seed>
        struct ZeroMemAvailable<NamedHandle<UID, Size, false, Seed>>
        {
            static constexpr bool value = false;
        };
        template <typename UID, usize_t Size, uint32_t Seed>
        struct TriviallySerializable<NamedHandle<UID, Size, false, Seed>>
        {
            static constexpr bool value = false;
        };
    }

}

template <typename UID, size_t Size, bool Optimize, uint32_t Seed>
struct std::hash<Be::NamedHandle<UID, Size, Optimize, Seed>>
{
    [[nodiscard]] size_t operator()(const Be::NamedHandle<UID, Size, Optimize, Seed> &value) const noexcept
    {
        return size_t(value.Hash());
    }
};
