#pragma once

namespace Be
{

    template <uint8_t n, typename T>
    struct Vector
    {
    public:
        using scalar_type = T;

    public:
        FixedArray<T, n> data;

    public:
        constexpr Vector() noexcept
        {
            data.fill(0);
        }

        constexpr Vector(const T &rhs) noexcept
        {
            data.fill(rhs);
        }

        template <uint8_t N>
        constexpr Vector(const Vector<N, T> &o) noexcept
        {
            MemCopy(data.data(), o.data.data(), std::min(N, n) * sizeof(T));
        }

        template <typename O>
        constexpr Vector(const Vector<n, O> &o) noexcept
        {
            for (uint8_t i = 0; i < n; i++)
            {
                data[i] = T(o.data[i]);
            }
        }

        template <uint8_t N, typename O>
        constexpr Vector(const Vector<N, O> &o) noexcept
        {
            for (uint8_t i = 0; i < std::min(N, n); i++)
            {
                data[i] = T(o.data[i]);
            }
        }

    public:
        [[nodiscard]] forceinline constexpr T operator[](const uint8_t index) const noexcept
        {
            ASSERT_MSG(index < n, "Index is greater then Vector size.");
            return data.at(index);
        }

        [[nodiscard]] forceinline constexpr T &operator[](const uint8_t index) noexcept
        {
            ASSERT_MSG(index < n, "Index is greater then Vector size.");
            return data.at(index);
        }

    public:
        template <uint8_t N>
        constexpr Vector<n, T> &operator=(const Vector<N, T> &o) noexcept
        {
            MemCopy(data.data(), o.data.data(), std::min(N, n) * sizeof(T));
            return *this;
        }

        template <uint8_t N, typename O>
        constexpr Vector<n, T> &operator=(const Vector<N, O> &o) noexcept
        {
            for (uint8_t i = 0; i < std::min(N, n); i++)
            {
                data[i] = T(o.data[i]);
            }
            return *this;
        }

    public:
        [[nodiscard]] forceinline Vector<n, T> operator-() const noexcept
        {
            Vector<n, T> result;
            for (uint8_t i = 0; i < n; ++i)
            {
                result[i] = -data[i];
            }
            return result;
        }

        [[nodiscard]] forceinline Vector<n, T> &operator*=(const T &scalar) noexcept
        {
            for (uint8_t i = 0; i < n; ++i)
            {
                data[i] *= scalar;
            }
            return *this;
        }

        [[nodiscard]] forceinline Vector<n, T> &operator/=(const T &scalar) noexcept
        {
            Vector<n, T> result;
            for (uint8_t i = 0; i < n; ++i)
            {
                data[i] /= scalar;
            }
            return *this;
        }

    public:
        [[nodiscard]] forceinline operator String() const noexcept
        {
            return ToString();
        }

    public:
        [[nodiscard]] String ToString() const noexcept
        {
            String ret = "[";
            for (uint8_t i = 0; i < data.size(); ++i)
            {
                ret += (i == data.size() - 1) ? (data[i]) : (data[i] + ", ");
            }
            return ret + "]";
        }
    };

    // Vector2
    template <typename T>
    struct Vector<2, T>
    {
    public:
        using scalar_type = T;

    public:
        union
        {
            FixedArray<T, 2> data;
            struct
            {
                T x;
                T y;
            };
        };

    public:
        constexpr Vector() noexcept : data{0, 0}
        {
        }

        constexpr Vector(const T &rhs) noexcept : data{rhs, rhs}
        {
        }

        constexpr Vector(const T &x, const T &y) noexcept : data{x, y}
        {
        }

        template <typename O>
        constexpr Vector(const Vector<2, O> &o) noexcept
        {
            x = T(o.data[0]);
            y = T(o.data[1]);
        }

        template <uint8_t N, typename O>
        constexpr Vector(const Vector<N, O> &o) noexcept
        {
            for (uint8_t i = 0; i < std::min(N, uint8_t(2)); i++)
            {
                data[i] = T(o.data[i]);
            }
        }

    public:
        [[nodiscard]] forceinline T operator[](const uint8_t index) const noexcept
        {
            ASSERT_MSG(index < 2, "Index is greater then 1.");
            return data.at(index);
        }

        [[nodiscard]] forceinline T &operator[](const uint8_t index) noexcept
        {
            ASSERT_MSG(index < 2, "Index is greater then 1.");
            return data.at(index);
        }

    public:
        template <uint8_t N>
        constexpr Vector<2, T> &operator=(const Vector<N, T> &o) noexcept
        {
            MemCopy(data.data(), o.data.data(), std::min(N, uint8_t(2)) * sizeof(T));
            return *this;
        }

        template <uint8_t N, typename O>
        constexpr Vector<2, T> &operator=(const Vector<N, O> &o) noexcept
        {
            for (uint8_t i = 0; i < std::min(N, uint8_t(2)); i++)
            {
                data[i] = T(o.data[i]);
            }
            return *this;
        }

    public:
        [[nodiscard]] forceinline Vector<2, T> operator-() const noexcept
        {
            return {-x, -y};
        }

        forceinline Vector<2, T> &operator*=(const T &scalar) noexcept
        {
            data[0] *= scalar;
            data[1] *= scalar;
            return *this;
        }

        forceinline Vector<2, T> &operator/=(const T &scalar) noexcept
        {
            data[0] /= scalar;
            data[1] /= scalar;
            return *this;
        }

    public:
        [[nodiscard]] forceinline operator String() const noexcept
        {
            return ToString();
        }

    public:
        [[nodiscard]] String ToString() const noexcept
        {
            return String("[") + std::to_string(x) + ", " + std::to_string(y) + "]";
        }
    };

    // Vector3
    template <typename T>
    struct Vector<3, T>
    {
    public:
        using scalar_type = T;

    public:
        static constexpr Vector<3, T> ZERO{0.0, 0.0, 0.0};
        static constexpr Vector<3, T> AXIS_X{1.0, 0.0, 0.0};
        static constexpr Vector<3, T> AXIS_Y{0.0, 1.0, 0.0};
        static constexpr Vector<3, T> AXIS_Z{0.0, 0.0, 1.0};
        static constexpr Vector<3, T> UP{AXIS_Y};
        static constexpr Vector<3, T> RIGHT{AXIS_X};
        static constexpr Vector<3, T> FORWARD{AXIS_Z};
        static constexpr Vector<3, T> DOWN{0.0, -1.0, 0.0};
        static constexpr Vector<3, T> LEFT{-1.0, 0.0, 0.0};
        static constexpr Vector<3, T> BACK{0.0, 0.0, -1.0};

    public:
        union
        {
            FixedArray<T, 3> data;
            struct
            {
                T x;
                T y;
                T z;
            };
            struct
            {
                Vector<2, T> xy;
                T _ignored_z;
            };
            struct
            {
                T _ignored_x;
                Vector<2, T> yz;
            };
        };

    public:
        constexpr Vector() noexcept : data{0, 0, 0}
        {
        }

        constexpr Vector(const T &rhs) noexcept : data{rhs, rhs, rhs}
        {
        }

        constexpr Vector(const T &x, const T &y, const T &z) noexcept : data{x, y, z}
        {
        }

        constexpr Vector(const Vector<2, T> &vec, const T &z) noexcept : data{vec.x, vec.y, z}
        {
        }

        constexpr Vector(const T &x, const Vector<2, T> &vec) noexcept : data{x, vec.x, vec.y}
        {
        }

    public:
        [[nodiscard]] forceinline T operator[](const uint8_t index) const noexcept
        {
            ASSERT_MSG(index < 3, "Arg size is greater then 2.");
            return data.at(index);
        }

        [[nodiscard]] forceinline T &operator[](const uint8_t index) noexcept
        {
            ASSERT_MSG(index < 3, "Arg size is greater then 2.");
            return data.at(index);
        }

    public:
        template <uint8_t N>
        constexpr Vector<3, T> &operator=(const Vector<N, T> &o) noexcept
        {
            MemCopy(data.data(), o.data.data(), std::min(N, uint8_t(3)) * sizeof(T));
            return *this;
        }

        template <uint8_t N, typename O>
        constexpr Vector<3, T> &operator=(const Vector<N, O> &o) noexcept
        {
            for (uint8_t i = 0; i < std::min(N, uint8_t(3)); i++)
            {
                data[i] = T(o.data[i]);
            }
            return *this;
        }

    public:
        [[nodiscard]] forceinline Vector<3, T> operator-() const noexcept
        {
            return {-x, -y, -z};
        }

        forceinline Vector<3, T> &operator*=(const T &scalar) noexcept
        {
            data[0] *= scalar;
            data[1] *= scalar;
            data[2] *= scalar;
            return *this;
        }

        forceinline Vector<3, T> &operator/=(const T &scalar) noexcept
        {
            data[0] /= scalar;
            data[1] /= scalar;
            data[2] /= scalar;
            return *this;
        }

    public:
        template <typename O>
        [[nodiscard]] forceinline operator Vector<3, O>() const noexcept
        {
            Vector<3, O> ret;
            for (uint8_t i = 0; i < 3; ++i)
            {
                ret[i] = O(data[i]);
            }
        }

        [[nodiscard]] forceinline operator String() const noexcept
        {
            return ToString();
        }

    public:
        [[nodiscard]] forceinline String ToString() const noexcept
        {
            return String("[") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]";
        }
    };

    // Vector4
    template <typename T>
    struct Vector<4, T>
    {
    public:
        using scalar_type = T;

    public:
        union
        {
            FixedArray<T, 4> data;
            struct
            {
                T x;
                T y;
                T z;
                T w;
            };
            struct
            {
                T r;
                T g;
                T b;
                T a;
            };
            struct
            {
                Vector<2, T> xy;
                T _ignored1_z;
                T _ignored1_w;
            };
            struct
            {
                T _ignored2_x;
                Vector<2, T> yz;
                T _ignored2_w;
            };
            struct
            {
                T _ignored3_x;
                T _ignored3_y;
                Vector<2, T> zw;
            };
            struct
            {
                Vector<3, T> xyz;
                T _ignored4_w;
            };
            struct
            {
                Vector<3, T> rgb;
                T _ignored_a;
            };
        };

    public:
        constexpr Vector() noexcept : data{0, 0, 0, 0}
        {
        }

        constexpr Vector(const T &rhs) noexcept : data{rhs, rhs, rhs, rhs}
        {
        }

        constexpr Vector(const T &x, const T &y, const T &z, const T &w) noexcept : data{x, y, z, w}
        {
        }

        constexpr Vector(const Vector<2, T> &xy, const Vector<2, T> &zw) noexcept : data{xy.x, xy.y, zw.x, zw.y}
        {
        }

        constexpr Vector(const Vector<2, T> &xy, const T &z, const T &w) noexcept : data{xy.x, xy.y, z, w}
        {
        }

        constexpr Vector(const T &x, const T &y, const Vector<2, T> &zw) noexcept : data{x, y, zw.x, zw.y}
        {
        }

        constexpr Vector(const T &x, const Vector<2, T> &yz, const T &w) noexcept : data{x, yz.x, yz.y, w}
        {
        }

        constexpr Vector(const Vector<3, T> &xyz, const T &w) noexcept : data{xyz.x, xyz.y, xyz.z, w}
        {
        }

        constexpr Vector(const T &x, const Vector<3, T> &yzw) noexcept : data{x, yzw.x, yzw.y, yzw.z}
        {
        }

    public:
        [[nodiscard]] forceinline T operator[](const uint8_t index) const noexcept
        {
            ASSERT_MSG(index < 4, "Index is greater then 4.");
            return data.at(index);
        }

        [[nodiscard]] forceinline T &operator[](const uint8_t index) noexcept
        {
            ASSERT_MSG(index < 4, "Index is greater then 4.");
            return data.at(index);
        }

    public:
        template <uint8_t N>
        constexpr Vector<4, T> &operator=(const Vector<N, T> &o) noexcept
        {
            MemCopy(data.data(), o.data.data(), std::min(N, uint8_t(4)) * sizeof(T));
            return *this;
        }

        template <uint8_t N, typename O>
        constexpr Vector<4, T> &operator=(const Vector<N, O> &o) noexcept
        {
            for (uint8_t i = 0; i < std::min(N, uint8_t(4)); i++)
            {
                data[i] = T(o.data[i]);
            }
            return *this;
        }

    public:
        [[nodiscard]] forceinline Vector<4, T> operator-() const noexcept
        {
            return {-x, -y, -z, -w};
        }

        forceinline Vector<4, T> &operator*=(const T &scalar) noexcept
        {
            data[0] *= scalar;
            data[1] *= scalar;
            data[2] *= scalar;
            data[3] *= scalar;
            return *this;
        }

        forceinline Vector<4, T> &operator/=(const T &scalar) noexcept
        {
            data[0] /= scalar;
            data[1] /= scalar;
            data[2] /= scalar;
            data[3] /= scalar;
            return *this;
        }

    public:
        template <typename O>
        [[nodiscard]] forceinline operator Vector<4, O>() const noexcept
        {
            Vector<4, O> ret;
            for (uint8_t i = 0; i < 4; ++i)
            {
                ret[i] = O(data[i]);
            }
        }

        [[nodiscard]] forceinline operator String() const noexcept
        {
            return ToString();
        }

    public:
        [[nodiscard]] forceinline String ToString() const noexcept
        {
            return String("[") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + "]";
        }
    };

    using Bool2 = Vector<2, bool>;
    using Bool3 = Vector<3, bool>;
    using Bool4 = Vector<4, bool>;

    using Byte2 = Vector<2, int8_t>;
    using Byte3 = Vector<3, int8_t>;
    using Byte4 = Vector<4, int8_t>;

    using UByte2 = Vector<2, uint8_t>;
    using UByte3 = Vector<3, uint8_t>;
    using UByte4 = Vector<4, uint8_t>;

    using Short2 = Vector<2, int16_t>;
    using Short3 = Vector<3, int16_t>;
    using Short4 = Vector<4, int16_t>;

    using UShort2 = Vector<2, uint16_t>;
    using UShort3 = Vector<3, uint16_t>;
    using UShort4 = Vector<4, uint16_t>;

    using UInt2 = Vector<2, uint32_t>;
    using UInt3 = Vector<3, uint32_t>;
    using UInt4 = Vector<4, uint32_t>;

    using Int2 = Vector<2, int32_t>;
    using Int3 = Vector<3, int32_t>;
    using Int4 = Vector<4, int32_t>;

    using Long2 = Vector<2, int64_t>;
    using Long3 = Vector<3, int64_t>;
    using Long4 = Vector<4, int64_t>;

    using ULong2 = Vector<2, uint64_t>;
    using ULong3 = Vector<3, uint64_t>;
    using ULong4 = Vector<4, uint64_t>;

    using Float2 = Vector<2, float>;
    using Float3 = Vector<3, float>;
    using Float4 = Vector<4, float>;

    using Double2 = Vector<2, double>;
    using Double3 = Vector<3, double>;
    using Double4 = Vector<4, double>;

    using Colorf3 = Vector<3, float>;
    using Colorf4 = Vector<4, float>;

    using Vector2 = Vector<2, BE_MATH_PRECISION>;
    using Vector3 = Vector<3, BE_MATH_PRECISION>;
    using Vector4 = Vector<4, BE_MATH_PRECISION>;

    // addition
    template <uint8_t n, class T>
    [[nodiscard]] forceinline Vector<n, T> operator+(const Vector<n, T> &lhs, const T scalar) noexcept
    {
        Vector<n, T> result;
        for (uint8_t i = 0; i < n; ++i)
        {
            result[i] = lhs[i] + scalar;
        }
        return result;
    }

    template <uint8_t n, class T>
    [[nodiscard]] forceinline Vector<n, T> operator+(const T scalar, const Vector<n, T> &rhs) noexcept
    {
        Vector<n, T> result;
        for (uint8_t i = 0; i < n; ++i)
        {
            result[i] = rhs[i] + scalar;
        }
        return result;
    }

    template <uint8_t n, class T>
    [[nodiscard]] forceinline Vector<n, T> operator+(const Vector<n, T> &lhs, const Vector<n, T> &rhs) noexcept
    {
        Vector<n, T> result;
        for (uint8_t i = 0; i < n; ++i)
        {
            result[i] = lhs[i] + rhs[i];
        }
        return result;
    }

    // subtraction
    template <uint8_t n, class T>
    [[nodiscard]] forceinline Vector<n, T> operator-(const Vector<n, T> &lhs, const T scalar) noexcept
    {
        Vector<n, T> result;
        for (uint8_t i = 0; i < n; ++i)
        {
            result[i] = lhs[i] - scalar;
        }
        return result;
    }

    template <uint8_t n, class T>
    [[nodiscard]] forceinline Vector<n, T> operator-(const Vector<n, T> &lhs, const Vector<n, T> &rhs) noexcept
    {
        Vector<n, T> result;
        for (uint8_t i = 0; i < n; ++i)
        {
            result[i] = lhs[i] - rhs[i];
        }
        return result;
    }

    // multiplication
    template <uint8_t n, class T>
    [[nodiscard]] forceinline Vector<n, T> operator*(const Vector<n, T> lhs, const T &scalar) noexcept
    {
        Vector<n, T> result;
        for (uint8_t i = 0; i < n; ++i)
        {
            result[i] = lhs[i] * scalar;
        }
        return result;
    }

    template <uint8_t n, class T>
    [[nodiscard]] forceinline Vector<n, T> operator*(const T scalar, const Vector<n, T> &lhs) noexcept
    {
        Vector<n, T> result;
        for (uint8_t i = 0; i < n; ++i)
        {
            result[i] = lhs[i] * scalar;
        }
        return result;
    }

    // hadamard product
    template <uint8_t n, class T>
    [[nodiscard]] forceinline Vector<n, T> operator*(const Vector<n, T> &lhs, const Vector<n, T> &rhs) noexcept
    {
        Vector<n, T> result;
        for (uint8_t i = 0; i < n; ++i)
        {
            result[i] = lhs[i] * rhs[i];
        }
        return result;
    }

    // division
    template <uint8_t n, class T>
    [[nodiscard]] forceinline Vector<n, T> operator/(const Vector<n, T> &lhs, const T scalar) noexcept
    {
        Vector<n, T> result;
        for (uint8_t i = 0; i < n; ++i)
        {
            result[i] = lhs[i] / scalar;
        }
        return result;
    }

    template <uint8_t n, class T>
    [[nodiscard]] forceinline Vector<n, T> operator/(const T scalar, const Vector<n, T> &rhs) noexcept
    {
        Vector<n, T> result;
        for (uint8_t i = 0; i < n; ++i)
        {
            result[i] = scalar / rhs[i];
        }
        return result;
    }

    // hadamard product
    template <uint8_t n, class T>
    [[nodiscard]] forceinline Vector<n, T> operator/(const Vector<n, T> &lhs, const Vector<n, T> &rhs) noexcept
    {
        Vector<n, T> result;
        for (uint8_t i = 0; i < n; ++i)
        {
            result[i] = lhs[i] / rhs[i];
        }
        return result;
    }

    namespace Math
    {

        template <int I, typename T>
        [[nodiscard]] forceinline constexpr Vector<I, T> Max(const Vector<I, T> &lhs, const Vector<I, T> &rhs) noexcept
        {
            Vector<I, T> res;
            for (int i = 0; i < I; i++)
            {
                res[i] = std::max(lhs[i], rhs[i]);
            }
            return res;
        }

        template <int I, typename T>
        [[nodiscard]] forceinline constexpr Vector<I, T> Min(const Vector<I, T> &lhs, const Vector<I, T> &rhs) noexcept
        {
            Vector<I, T> res;
            for (int i = 0; i < I; i++)
            {
                res[i] = std::min(lhs[i], rhs[i]);
            }
            return res;
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline bool Equals(const Vector<n, T> &lhs, const Vector<n, T> &rhs, const T epsilon = BE_DEFAULT_EPSILON_T) noexcept
        {
            for (uint8_t i = 0; i < n; ++i)
            {
                if (!Equals(lhs[i], rhs[i], epsilon))
                {
                    return false;
                }
            }
            return true;
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline bool IsZero(const Vector<n, T> &lhs, const T epsilon = static_cast<T>(0)) noexcept
        {
            return Equals(lhs, Vector<n, T>(0), epsilon);
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline T Dot(const Vector<n, T> &lhs, const Vector<n, T> &rhs) noexcept
        {
            T result{};
            for (uint8_t i = 0; i < n; ++i)
            {
                result += lhs[i] * rhs[i];
            }
            return result;
        }

        template <typename T>
        [[nodiscard]] forceinline constexpr Vector<3, T> Max(const Vector<3, T> &lhs, const Vector<3, T> &rhs) noexcept
        {
            Vector<3, T> res;
            res[0] = std::max(lhs[0], rhs[0]);
            res[1] = std::max(lhs[1], rhs[1]);
            res[2] = std::max(lhs[2], rhs[2]);
            return res;
        }

        template <typename T>
        [[nodiscard]] forceinline constexpr Vector<3, T> Min(const Vector<3, T> &lhs, const Vector<3, T> &rhs) noexcept
        {
            Vector<3, T> res;
            res[0] = std::min(lhs[0], rhs[0]);
            res[1] = std::min(lhs[1], rhs[1]);
            res[2] = std::min(lhs[2], rhs[2]);
            return res;
        }

        template <class T>
        [[nodiscard]] forceinline T Dot(const Vector<2, T> &lhs, const Vector<2, T> &rhs) noexcept
        {
            return lhs.x * rhs.x + lhs.y * rhs.y;
        }

        template <class T>
        [[nodiscard]] forceinline T Dot(const Vector<3, T> &lhs, const Vector<3, T> &rhs) noexcept
        {
            return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
        }

        template <class T>
        [[nodiscard]] forceinline T Dot(const Vector<4, T> &lhs, const Vector<4, T> &rhs) noexcept
        {
            return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline T LengthSquared(const Vector<n, T> &vec) noexcept
        {
            return Dot(vec, vec);
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline T Length(const Vector<n, T> &vec) noexcept
        {
            return std::sqrt(Dot(vec, vec));
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline T Distance(const Vector<n, T> &lhs, const Vector<n, T> &rhs) noexcept
        {
            return Length(lhs - rhs);
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline T DistanceSquared(const Vector<n, T> &lhs, const Vector<n, T> &rhs) noexcept
        {
            return LengthSquared(lhs - rhs);
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline Vector<n, T> Normalize(const Vector<n, T> &vec) noexcept
        {
            T len = Length(vec);
            return vec / len;
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline Vector<n, T> Scale(const Vector<n, T> &vec, T new_len) noexcept
        {
            T len = Length(vec);
            return vec * new_len / len;
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline Vector<n, T> Pow(const Vector<n, T> &vec, T exponent) noexcept
        {
            Vector<n, T> result = Vector<n, T>();
            for (uint8_t i = 0; i < n; ++i)
            {
                result[i] = std::pow(vec[i], exponent);
            }
            return result;
        }

        // Vectors need to be prenormalized
        // if input vectors are zero it will generate NaN
        template <uint8_t n, class T>
        [[nodiscard]] forceinline T AngleFastUnsigned(const Vector<n, T> &lhs, const Vector<n, T> &rhs) noexcept
        {
            return std::acos(Dot(lhs, rhs));
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline T AngleSafeUnsigned(const Vector<n, T> &lhs, const Vector<n, T> &rhs) noexcept
        {
            return AngleFastUnsigned(Normalize(lhs), Normalize(rhs));
        }

        // project source Vector onto target Vector
        template <uint8_t n, class T>
        [[nodiscard]] forceinline T ProjectionFactor(Vector<n, T> const &source, Vector<n, T> const &target) noexcept
        {
            return Dot(source, target) / LengthSquared(target);
        }

        template <uint8_t n, class T>
        [[nodiscard]] forceinline Vector<n, T> Projection(Vector<n, T> const &source, Vector<n, T> const &target) noexcept
        {
            return target * ProjectionFactor(source, target);
        }

        template <uint8_t n, typename T>
        [[nodiscard]] forceinline Vector<n, T> Swizzle(Vector<n, T> const &vec, Vector<n, uint32_t> const &indices) noexcept
        {
            Vector<n, T> result;
            for (uint8_t i = 0; i < n; ++i)
            {
                result[indices[i]] = vec[i];
            }
            return result;
        }

        // Vector2
        template <class T>
        [[nodiscard]] forceinline Vector<2, T> Perpendicular(const Vector<2, T> &vec) noexcept
        {
            Vector<2, T> result;
            result.x = -vec.y;
            result.y = vec.x;
            return result;
        }

        // Vectors need to be prenormalized
        template <class T>
        [[nodiscard]] forceinline T Angle(const Vector<2, T> &lhs, const Vector<2, T> &rhs) noexcept
        {
            return std::atan2(rhs.y, rhs.x) - std::atan2(lhs.y, lhs.x);
        }

        // Vector3
        // direction will be left handed (lhs = thumb, rhs = index, result -> middle finger);
        template <class T>
        [[nodiscard]] forceinline Vector<3, T> Cross(const Vector<3, T> &lhs, const Vector<3, T> &rhs) noexcept
        {
            Vector<3, T> result;

            result.x = lhs.y * rhs.z - lhs.z * rhs.y;
            result.y = lhs.z * rhs.x - lhs.x * rhs.z;
            result.z = lhs.x * rhs.y - lhs.y * rhs.x;

            return result;
        }

        // inputs vectors must be prenormalized,
        // and, for accurate measurments the outAxis should also be normalized after
        // if input vectors are zero it will generate NaN
        template <class T>
        [[nodiscard]] forceinline T AngleFastAxis(const Vector<3, T> &lhs, const Vector<3, T> &rhs, Vector<3, T> &out_axis) noexcept
        {
            out_axis = Cross(lhs, rhs);
            return AngleFastUnsigned(lhs, rhs);
        }

        template <class T>
        [[nodiscard]] forceinline T AngleSafeAxis(const Vector<3, T> &lhs, const Vector<3, T> &rhs, Vector<3, T> &out_axis) noexcept
        {
            Vector<3, T> lhs_n = Normalize(lhs);
            Vector<3, T> rhs_n = Normalize(rhs);
            T result = AngleFastAxis(lhs_n, rhs_n, out_axis);
            out_axis = Normalize(out_axis);
            return result;
        }
    }

    // operators
    template <uint8_t n, class T>
    [[nodiscard]] forceinline bool operator==(const Vector<n, T> &lhs, const Vector<n, T> &rhs) noexcept
    {
        return Math::Equals(lhs, rhs);
    }
}

namespace std
{
    template <uint8_t n, class T>
    struct hash<Be::Vector<n, T>>
    {
        [[nodiscard]] std::size_t operator()(const Be::Vector<n, T> &v) const noexcept
        {
            Be::HashValue h{};
            for (uint8_t i = 0; i < n; i++)
            {
                h += Be::HashOf(v[i]);
            }
            return size_t(h);
        }
    };
}
