#pragma once

namespace Be
{

    template <class T>
    struct Quaternion
    {
    public:
        union
        {
            struct
            {
                union
                {
                    struct
                    {
                        T x, y, z;
                    };
                    Vector<3, T> v;
                };
                T w;
            };
            Vector<4, T> v4;
            std::array<T, 4> data;
        };

    public:
        // Identity Quaternion
        Quaternion() noexcept : x(0), y(0), z(0), w(1)
        {
        }

        // Axis angle initialization
        Quaternion(const Vector<3, T> &axis, const T angle) noexcept
        {
            const T half_angle = angle * T(0.5);
            const T sin_half = std::sin(half_angle);
            x = axis.x * sin_half;
            y = axis.y * sin_half;
            z = axis.z * sin_half;
            w = std::cos(half_angle);
        }

        Quaternion(const T &lx, const T &ly, const T &lz, const T &lw) noexcept
        {
            x = lx;
            y = ly;
            z = lz;
            w = lw;
        }

        // Euler angle initialization
        explicit Quaternion(const Vector<3, T> &euler) noexcept
        {
            Vector<3, T> fac = euler * T(0.5);
            Vector<3, T> c(std::cos(fac.x), std::cos(fac.y), std::cos(fac.z));
            Vector<3, T> s(std::sin(fac.x), std::sin(fac.y), std::sin(fac.z));

            x = s.x * c.y * c.z - c.x * s.y * s.z;
            y = c.x * s.y * c.z + s.x * c.y * s.z;
            z = c.x * c.y * s.z - s.x * s.y * c.z;
            w = c.x * c.y * c.z + s.x * s.y * s.z;
        }

        // Rotation Matrix initialization
        Quaternion(const Matrix<3, 3, T> &rot) noexcept
        {
            uint8_t biggest_index = 0;
            T r012 = rot[0][0] - rot[1][1] - rot[2][2];
            T r102 = rot[1][1] - rot[0][0] - rot[2][2];
            T r201 = rot[2][2] - rot[0][0] - rot[1][1];
            T biggest_val = rot[0][0] + rot[1][1] + rot[2][2];

            if (r012 > biggest_val)
            {
                biggest_val = r012;
                biggest_index = 1;
            }
            if (r102 > biggest_val)
            {
                biggest_val = r102;
                biggest_index = 2;
            }
            if (r201 > biggest_val)
            {
                biggest_val = r201;
                biggest_index = 3;
            }

            biggest_val = std::sqrt(biggest_val + T(1)) * T(0.5);
            T mult = T(0.25) / biggest_val;

            switch (biggest_index)
            {
            case 0:
                x = (rot[2][1] - rot[1][2]) * mult;
                y = (rot[0][2] - rot[2][0]) * mult;
                z = (rot[1][0] - rot[0][1]) * mult;
                w = -biggest_val;
            case 1:
                x = biggest_val;
                y = (rot[1][0] - rot[0][1]) * mult;
                z = (rot[0][2] - rot[2][0]) * mult;
                w = -(rot[2][1] - rot[1][2]) * mult;
            case 2:
                x = (rot[1][0] - rot[0][1]) * mult;
                y = biggest_val;
                z = (rot[2][1] - rot[1][2]) * mult;
                w = -(rot[0][2] - rot[2][0]) * mult;
            case 3:
                x = (rot[0][2] - rot[2][0]) * mult;
                y = (rot[2][1] - rot[1][2]) * mult;
                z = biggest_val;
                w = -(rot[1][0] - rot[0][1]) * mult;
            }
        }

    public:
        [[nodiscard]] forceinline constexpr T operator[](const uint8_t index) const noexcept
        {
            ASSERT_MSG(index < 4, "Index is greater then Quaternion size.");
            return data.at(index);
        }

        [[nodiscard]] forceinline constexpr T &operator[](const uint8_t index) noexcept
        {
            ASSERT_MSG(index < 4, "Index is greater then Quaternion size.");
            return data.at(index);
        }

    public:
        Vector<4, T> ToAxisAngle() const noexcept
        {
            Vector<4, T> result;

            const T angle = T(2) * std::acos(w);
            const T rcpLen = T(1) / std::sqrt(T(1) - w * w);

            result.xyz = Vector<3, T>(x, y, z) * rcpLen;
            result.w = angle;

            return result;
        }

        Vector<3, T> ToEuler() const noexcept
        {
            const T qxx(x * x);
            const T qyy(y * y);
            const T qzz(z * z);
            const T qww(w * w);

            T pitch = std::atan2(T(2) * (y * z + w * x), qww - qxx - qyy + qzz);
            T roll = std::atan2(T(2) * (x * y + w * z), qww + qxx - qyy - qzz);

            return Vector<3, T>(pitch, Yaw(), roll);
        }

    public:
        T Pitch() const noexcept
        {
            return std::atan2(T(2) * (y * z + w * x), w * w - x * x - y * y + z * z);
        }

        T Yaw() const noexcept
        {
            return std::asin(Clamp(T(-2) * (x * z - w * y), T(-1), T(1)));
        }

        T Roll() const noexcept
        {
            return std::atan2(T(2) * (x * y + w * z), w * w + x * x - y * y - z * z);
        }

    public:
        Matrix<3, 3, T> ToMatrix() const noexcept
        {
            Matrix<3, 3, T> mat;

            mat[0][0] = T(1) - T(2) * y * y - T(2) * z * z;
            mat[1][0] = T(2) * x * y - T(2) * w * z;
            mat[2][0] = T(2) * x * z + T(2) * w * y;

            mat[0][1] = T(2) * x * y + T(2) * w * z;
            mat[1][1] = T(1) - T(2) * x * x - T(2) * z * z;
            mat[2][1] = T(2) * y * z - T(2) * w * x;

            mat[0][2] = T(2) * x * z - T(2) * w * y;
            mat[1][2] = T(2) * y * z + T(2) * w * x;
            mat[2][2] = T(1) - T(2) * x * x - T(2) * y * y;

            return mat;
        }

    public:
        std::string ToString() const noexcept
        {
            return String("[") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + " - " + std::to_string(w) + "]";
        }
    };

    using Quatf = Quaternion<float>;
    using Quatd = Quaternion<double>;
    using Quat = Quaternion<BE_MATH_PRECISION>;

    // operations
    // Grassman product
    template <typename T>
    [[nodiscard]] forceinline Quaternion<T> operator*(const Quaternion<T> &lhs, const Quaternion<T> &rhs) noexcept
    {
        const T w = rhs.w * lhs.w - Dot(rhs.v, lhs.v);
        Vector<3, T> v = rhs.w * lhs.v + lhs.w * rhs.v + Cross(lhs.v, rhs.v);

        return Quaternion<T>(v.x, v.y, v.z, w);
    }

    // Rotate Vector with Quaternion
    template <typename T>
    [[nodiscard]] forceinline Vector<3, T> operator*(const Quaternion<T> &q, const Vector<3, T> &vec) noexcept
    {
        // slow version but mathematically correct
        // quat vecQuat = quat( vec.x, vec.y, vec.z, 0 );
        // return (q * vecQuat * inverse( q )).v;

        // fast version
        // Vector<3, T> temp = T(2) * cross( q.v, vec );
        // return vec + q.w * temp + cross( q.v, temp);

        // faster version
        // return T(2) * dot( q.v, vec ) * q.v
        //	+ (q.w * q.w - dot( q.v, q.v )) * vec
        //	+ T(2) * q.w * cross( q.v, vec );

        // fastest version
        return (T(2) * (q.w * q.w) - T(1)) * vec + T(2) * (Dot(q.v, vec) * q.v + q.w * Cross(q.v, vec));
    }

    template <typename T>
    [[nodiscard]] forceinline Quaternion<T> &Normalize(Quaternion<T> &q) noexcept
    {
        q.v4 = Normalize(q.v4);
        return q;
    }

    // unit Quaternion inversion - cheap assuming q is normalized
    template <typename T>
    [[nodiscard]] forceinline Quaternion<T> Inverse(const Quaternion<T> &q) noexcept
    {
        return Quaternion<T>(-q.x, -q.y, -q.z, q.w);
    }

    template <typename T>
    [[nodiscard]] forceinline Quaternion<T> InverseSafe(const Quaternion<T> &q) noexcept
    {
        auto result = Quaternion<T>(-q.x, -q.y, -q.z, q.w);
        T len2 = LengthSquared(q.v4);
        result.v4 = result.v4 * len2;

        return result;
    }

}

namespace std
{
    template <class T>
    struct hash<Be::Quaternion<T>>
    {
        [[nodiscard]] std::size_t operator()(const Be::Quaternion<T> &v) const noexcept
        {
            Be::HashValue h{};
            for (uint8_t i = 0; i < 4; i++)
            {
                h += Be::HashOf(v[i]);
            }
            return size_t(h);
        }
    };
}