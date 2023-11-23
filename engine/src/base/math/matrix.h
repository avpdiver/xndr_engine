#pragma once

namespace Be
{

    template <uint8_t R, uint8_t C, typename T>
    struct Matrix
    {
    public:
        typedef T value_type;

    public:
        [[nodiscard]] static forceinline constexpr uint8_t Rows() { return R; }
        [[nodiscard]] static forceinline constexpr uint8_t Columns() { return C; }

    public:
        union
        {
            T data[R][C];
            struct
            {
                Vector<C, T> rows[R];
            };
        };

    public:
        constexpr Matrix() noexcept
        {
            for (uint8_t row = 0; row < R; ++row)
            {
                for (uint8_t col = 0; col < C; ++col)
                {
                    data[row][col] = (row == col) ? static_cast<T>(1) : static_cast<T>(0);
                }
            }
        }

        explicit constexpr Matrix(EUninitialized) noexcept {}

        explicit constexpr Matrix(const Vector<C, T> row_vecs[R]) noexcept
        {
            for (uint8_t row = 0; row < R; ++row)
            {
                rows[row] = row_vecs[row];
            }
        }

        template <typename O>
        constexpr Matrix(const Matrix<R, C, O> &o) noexcept
        {
            for (uint8_t row = 0; row < R; ++row)
            {
                rows[row] = o[row];
            }
        }

    public:
        [[nodiscard]] forceinline constexpr Vector<C, T> operator[](const uint8_t row) const noexcept
        {
            ASSERT_MSG(row < R, "Row index is greater then {}.", R);
            return rows[row];
        }

        [[nodiscard]] forceinline constexpr Vector<C, T> &operator[](const uint8_t row) noexcept
        {
            ASSERT_MSG(row < R, "Row index is greater then {}.", R);
            return rows[row];
        }
    };

    using Float2x2 = Matrix<2, 2, float>;
    using Float3x3 = Matrix<3, 3, float>;
    using Float4x4 = Matrix<4, 4, float>;

    using Double2x2 = Matrix<2, 2, double>;
    using Double3x3 = Matrix<3, 3, double>;
    using Double4x4 = Matrix<4, 4, double>;

    using Matrix2x2 = Matrix<2, 2, BE_MATH_PRECISION>;
    using Matrix3x3 = Matrix<3, 3, BE_MATH_PRECISION>;
    using Matrix4x4 = Matrix<4, 4, BE_MATH_PRECISION>;

    // Matrix operators
    template <uint8_t R, uint8_t C, class T>
    [[nodiscard]] forceinline Matrix<R, C, T> operator+(const Matrix<R, C, T> &lhs, const Matrix<R, C, T> &rhs) noexcept
    {
        Matrix<R, C, T> result;
        for (uint8_t col = 0; col < C; ++col)
        {
            for (uint8_t row = 0; row < R; ++row)
            {
                result[col][row] = lhs[col][row] + rhs[col][row];
            }
        }
        return result;
    }

    template <uint8_t R, uint8_t C, class T>
    [[nodiscard]] forceinline Matrix<R, C, T> operator-(const Matrix<R, C, T> &lhs, const Matrix<R, C, T> &rhs) noexcept
    {
        Matrix<R, C, T> result;
        for (uint8_t col = 0; col < C; ++col)
        {
            for (uint8_t row = 0; row < R; ++row)
            {
                result[col][row] = lhs[col][row] - rhs[col][row];
            }
        }
        return result;
    }

    // multiplication: lhs rows * rhs cols
    template <uint8_t R, uint8_t C, class T>
    [[nodiscard]] forceinline Matrix<R, R, T> operator*(const Matrix<R, C, T> &lhs, const Matrix<C, R, T> &rhs) noexcept
    {
        Matrix<R, R, T> result;
        for (uint8_t col = 0; col < R; ++col)
        {
            for (uint8_t row = 0; row < R; ++row)
            {
                // dot product
                T value{0};
                for (uint8_t j = 0; j < C; ++j)
                {
                    value += lhs[row][j] * rhs[j][col];
                }
                result[row][col] = value;
            }
        }
        return result;
    }

    // Vector - Matrix multiplication
    template <uint8_t R, uint8_t C, class T>
    [[nodiscard]] forceinline Vector<C, T> operator*(const Matrix<R, C, T> &lhs, const Vector<C, T> &rhs) noexcept
    {
        Vector<C, T> result{0};
        for (uint8_t row = 0; row < R; ++row)
        {
            result[row] = Math::Dot(lhs[row], rhs);
        }
        return result;
    }

    // scalar - Matrix multiplication
    template <uint8_t R, uint8_t C, class T>
    [[nodiscard]] forceinline Matrix<R, C, T> operator*(const Matrix<R, C, T> &lhs, const T rhs) noexcept
    {
        Matrix<R, C, T> result;
        for (uint8_t row = 0; row < R; ++row)
        {
            result[row] = lhs[row] * rhs;
        }
        return result;
    }

    namespace Math
    {
        template <class T>
        [[nodiscard]] forceinline Matrix<3, 3, T> CreateFromMat4(const Matrix<4, 4, T> &lhs) noexcept
        {
            Matrix<3, 3, T> ret(uninitialized);
            ret.rows[0] = lhs.rows[0].xyz;
            ret.rows[1] = lhs.rows[1].xyz;
            ret.rows[2] = lhs.rows[2].xyz;
            return ret;
        }

        template <class T>
        [[nodiscard]] forceinline Matrix<4, 4, T> CreateFromMat3(const Matrix<3, 3, T> &lhs) noexcept
        {
            Matrix<4, 4, T> ret(uninitialized);
            ret.rows[0] = Vector<4, T>(lhs.rows[0], 0);
            ret.rows[1] = Vector<4, T>(lhs.rows[1], 0);
            ret.rows[2] = Vector<4, T>(lhs.rows[2], 0);
            ret.rows[3] = Vector<4, T>(0, 0, 0, 1);
            return ret;
        }

        template <class T>
        [[nodiscard]] forceinline Matrix<4, 4, T> DiscardW(const Matrix<4, 4, T> &lhs) noexcept
        {
            Matrix<4, 4, T> ret(uninitialized);
            ret.rows[0] = Vector<4, T>(lhs.rows[0].xyz, 0);
            ret.rows[1] = Vector<4, T>(lhs.rows[1].xyz, 0);
            ret.rows[2] = Vector<4, T>(lhs.rows[2].xyz, 0);
            ret.rows[3] = Vector<4, T>(0, 0, 0, 1);
            return ret;
        }

        template <uint8_t R, uint8_t C, class T>
        [[nodiscard]] forceinline bool Equals(const Matrix<R, C, T> &lhs, const Matrix<R, C, T> &rhs, const T epsilon = BE_DEFAULT_EPSILON_T) noexcept
        {
            for (uint8_t i = 0; i < R; ++i)
            {
                if (!Equals(lhs[i], rhs[i], epsilon))
                {
                    return false;
                }
            }
            return true;
        }

        template <uint8_t R, uint8_t C, class T>
        [[nodiscard]] forceinline Matrix<R, C, T> Transpose(const Matrix<R, C, T> &mat) noexcept
        {
            Matrix<C, R, T> result;

            for (uint8_t colIdx = 0; colIdx < R; ++colIdx)
            {
                for (uint8_t rowIdx = 0; rowIdx < C; ++rowIdx)
                {
                    result[rowIdx][colIdx] = mat[colIdx][rowIdx];
                }
            }
            return result;
        }

        // determinant
        template <class T>
        [[nodiscard]] forceinline T Determinant(const Matrix<2, 2, T> &mat) noexcept
        {
            return mat[0][0] * mat[1][1] - mat[1][0] * mat[0][1];
        }

        template <class T>
        [[nodiscard]] forceinline T Determinant(const Matrix<3, 3, T> &mat) noexcept
        {
            return +mat[0][0] * (mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2]) - mat[1][0] * (mat[0][1] * mat[2][2] - mat[2][1] * mat[0][2]) + mat[2][0] * (mat[0][1] * mat[1][2] - mat[1][1] * mat[0][2]);
        }

        template <class T>
        [[nodiscard]] forceinline T Determinant(const Matrix<4, 4, T> &mat) noexcept
        {
            T sub_factor00 = mat[2][2] * mat[3][3] - mat[3][2] * mat[2][3];
            T sub_factor01 = mat[2][1] * mat[3][3] - mat[3][1] * mat[2][3];
            T sub_factor02 = mat[2][1] * mat[3][2] - mat[3][1] * mat[2][2];
            T sub_factor03 = mat[2][0] * mat[3][3] - mat[3][0] * mat[2][3];
            T sub_factor04 = mat[2][0] * mat[3][2] - mat[3][0] * mat[2][2];
            T sub_factor05 = mat[2][0] * mat[3][1] - mat[3][0] * mat[2][1];

            Vector<4, T> det_cof{+(mat[1][1] * sub_factor00 - mat[1][2] * sub_factor01 + mat[1][3] * sub_factor02),
                                 -(mat[1][0] * sub_factor00 - mat[1][2] * sub_factor03 + mat[1][3] * sub_factor04),
                                 +(mat[1][0] * sub_factor01 - mat[1][1] * sub_factor03 + mat[1][3] * sub_factor05),
                                 -(mat[1][0] * sub_factor02 - mat[1][1] * sub_factor04 + mat[1][2] * sub_factor05)};

            return mat[0][0] * det_cof[0] + mat[0][1] * det_cof[1] +
                   mat[0][2] * det_cof[2] + mat[0][3] * det_cof[3];
        }

        // inverse -- not 100% safe as we don't check the determinant if the determinant is zero all values will be +infinity
        template <class T>
        [[nodiscard]] forceinline Matrix<2, 2, T> Inverse(const Matrix<2, 2, T> &mat) noexcept
        {
            T det_frac = T(1) / Determinant(mat);

            Matrix<2, 2, T> result({+mat[1][1] * det_frac,
                                    -mat[0][1] * det_frac,
                                    -mat[1][0] * det_frac,
                                    +mat[0][0] * det_frac});

            return result;
        }

        template <class T>
        [[nodiscard]] forceinline Matrix<3, 3, T> Inverse(const Matrix<3, 3, T> &mat) noexcept
        {
            T det_frac = T(1) / Determinant(mat);

            Matrix<3, 3, T> result({+(mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1]) * det_frac,
                                    -(mat[0][1] * mat[2][2] - mat[0][2] * mat[2][1]) * det_frac,
                                    +(mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1]) * det_frac,
                                    -(mat[1][0] * mat[2][2] - mat[1][2] * mat[2][0]) * det_frac,
                                    +(mat[0][0] * mat[2][2] - mat[0][2] * mat[2][0]) * det_frac,
                                    -(mat[0][0] * mat[1][2] - mat[0][2] * mat[1][0]) * det_frac,
                                    +(mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]) * det_frac,
                                    -(mat[0][0] * mat[2][1] - mat[0][1] * mat[2][0]) * det_frac,
                                    +(mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0]) * det_frac});

            return result;
        }

        template <class T>
        [[nodiscard]] forceinline Matrix<4, 4, T> Inverse(const Matrix<4, 4, T> &mat) noexcept
        {
            T coef00 = mat[2][2] * mat[3][3] - mat[3][2] * mat[2][3];
            T coef02 = mat[1][2] * mat[3][3] - mat[3][2] * mat[1][3];
            T coef03 = mat[1][2] * mat[2][3] - mat[2][2] * mat[1][3];

            T coef04 = mat[2][1] * mat[3][3] - mat[3][1] * mat[2][3];
            T coef06 = mat[1][1] * mat[3][3] - mat[3][1] * mat[1][3];
            T coef07 = mat[1][1] * mat[2][3] - mat[2][1] * mat[1][3];

            T coef08 = mat[2][1] * mat[3][2] - mat[3][1] * mat[2][2];
            T coef10 = mat[1][1] * mat[3][2] - mat[3][1] * mat[1][2];
            T coef11 = mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2];

            T coef12 = mat[2][0] * mat[3][3] - mat[3][0] * mat[2][3];
            T coef14 = mat[1][0] * mat[3][3] - mat[3][0] * mat[1][3];
            T coef15 = mat[1][0] * mat[2][3] - mat[2][0] * mat[1][3];

            T coef16 = mat[2][0] * mat[3][2] - mat[3][0] * mat[2][2];
            T coef18 = mat[1][0] * mat[3][2] - mat[3][0] * mat[1][2];
            T coef19 = mat[1][0] * mat[2][2] - mat[2][0] * mat[1][2];

            T coef20 = mat[2][0] * mat[3][1] - mat[3][0] * mat[2][1];
            T coef22 = mat[1][0] * mat[3][1] - mat[3][0] * mat[1][1];
            T coef23 = mat[1][0] * mat[2][1] - mat[2][0] * mat[1][1];

            Vector<4, T> fac0(coef00, coef00, coef02, coef03);
            Vector<4, T> fac1(coef04, coef04, coef06, coef07);
            Vector<4, T> fac2(coef08, coef08, coef10, coef11);
            Vector<4, T> fac3(coef12, coef12, coef14, coef15);
            Vector<4, T> fac4(coef16, coef16, coef18, coef19);
            Vector<4, T> fac5(coef20, coef20, coef22, coef23);

            Vector<4, T> vec0(mat[1][0], mat[0][0], mat[0][0], mat[0][0]);
            Vector<4, T> vec1(mat[1][1], mat[0][1], mat[0][1], mat[0][1]);
            Vector<4, T> vec2(mat[1][2], mat[0][2], mat[0][2], mat[0][2]);
            Vector<4, T> vec3(mat[1][3], mat[0][3], mat[0][3], mat[0][3]);

            Vector<4, T> inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
            Vector<4, T> inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
            Vector<4, T> inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
            Vector<4, T> inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

            Vector<4, T> sign_a(+1, -1, +1, -1);
            Vector<4, T> sign_b(-1, +1, -1, +1);
            Matrix<4, 4, T> result(uninitialized);
            result[0] = inv0 * sign_a;
            result[1] = inv1 * sign_b;
            result[2] = inv2 * sign_a;
            result[3] = inv3 * sign_b;

            Vector<4, T> row0(result[0][0], result[1][0], result[2][0], result[3][0]);

            Vector<4, T> dot0(mat[0] * row0);
            T dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

            T det_frac = T(1) / dot1;

            return result * det_frac;
        }

        // scaling

        template <uint8_t N, class T>
        forceinline void Scale(Matrix<N, N, T> &result, const Vector<N, T> &scale_vec) noexcept
        {
            for (uint8_t i = 0; i < N; ++i)
            {
                result[i][i] *= scale_vec[i];
            }
        }

        template <uint8_t N, class T>
        [[nodiscard]] forceinline Matrix<N, N, T> Scale(const Vector<N, T> &scale_vec) noexcept
        {
            Matrix<N, N, T> mat;
            Scale(mat, scale_vec);
            return mat;
        }

        template <uint8_t N, class T>
        [[nodiscard]] forceinline Matrix<N, N, T> Scale(const T &scale) noexcept
        {
            Matrix<N, N, T> mat;
            Scale(mat, {scale});
            return mat;
        }

        // specialization for 3 dimensions
        template <class T>
        [[nodiscard]] forceinline Matrix<4, 4, T> Scale(const Vector<3, T> &scale_vec) noexcept
        {
            return Scale(Vector<4, T>(scale_vec, 1));
        }

        template <class T>
        [[nodiscard]] forceinline Matrix<4, 4, T> Scale(const T &scale) noexcept
        {
            return Scale(Vector<4, T>{scale, scale, scale, 1});
        }

        template <class T>
        forceinline void Scale(Matrix<4, 4, T> &result, const Vector<3, T> &scale_vec) noexcept
        {
            Scale(result, Vector<4, T>(scale_vec, 1));
        }

        // rotation

        template <class T>
        [[nodiscard]] forceinline Matrix<4, 4, T> Rotate(const Vector<3, T> &axis, const T &angle) noexcept
        {
            Matrix<4, 4, T> result;

            const float c = std::cos(angle);
            const float s = std::sin(angle);
            const float t = T(1) - c;

            result[0][0] = t * axis.x * axis.x + c;
            result[1][0] = t * axis.x * axis.y - s * axis.z;
            result[2][0] = t * axis.x * axis.z + s * axis.y;

            result[0][1] = t * axis.x * axis.y + s * axis.z;
            result[1][1] = t * axis.y * axis.y + c;
            result[2][1] = t * axis.y * axis.z - s * axis.x;

            result[0][2] = t * axis.x * axis.z - s * axis.y;
            result[1][2] = t * axis.y * axis.z + s * axis.x;
            result[2][2] = t * axis.z * axis.z + c;

            return result;
        }

        template <class T>
        forceinline void Rotate(Matrix<4, 4, T> &result, const Vector<3, T> &axis, const T &angle) noexcept
        {
            Matrix<4, 4, T> rot = Rotate(axis, angle);
            result = result * rot;
        }

        // translation

        template <class T>
        [[nodiscard]] forceinline Matrix<4, 4, T> Translate(const Vector<3, T> &translation) noexcept
        {
            Matrix<4, 4, T> mat;
            mat[3] = Vector<4, T>(translation, T(1));
            return mat;
        }

        template <class T>
        forceinline void Translate(Matrix<4, 4, T> &result, const Vector<3, T> &translation) noexcept
        {
            Matrix<4, 4, T> mat;
            mat[0][3] = translation.x;
            mat[1][3] = translation.y;
            mat[2][3] = translation.z;
            mat[3][3] = T(1);

            result = result * mat;
        }

        // look at

        template <class T>
        [[nodiscard]] forceinline Matrix<4, 4, T> LookAt(const Vector<3, T> &position, const Vector<3, T> &target, const Vector<3, T> &world_up) noexcept
        {
            Vector<3, T> forward = normalize(target - position);
            Vector<3, T> right = normalize(cross(world_up, forward));
            Vector<3, T> up = cross(forward, right);

            Matrix<4, 4, T> frame(uninitialized);
            frame[0] = Vector<4, T>(right.x, up.x, forward.x, 0);
            frame[1] = Vector<4, T>(right.y, up.y, forward.y, 0);
            frame[2] = Vector<4, T>(right.z, up.z, forward.z, 0);
            frame[3] = Vector<4, T>(-dot(right, position), -dot(up, position), -dot(forward, position), 1);

            return frame;
        }

        // projection

        template <class T>
        [[nodiscard]] forceinline Matrix<4, 4, T> Orthographic(const T &left, const T &right, const T &top, const T &bottom, const T &near_z, const T &far_z) noexcept
        {
            Matrix<4, 4, T> result;

            result[0][0] = T(2) / (right - left);
            result[1][1] = T(2) / (top - bottom);
            result[3][0] = -(right + left) / (right - left);
            result[3][1] = -(top + bottom) / (top - bottom);

            result[2][2] = T(2) / (far_z - near_z);
            result[3][2] = -(far_z + near_z) / (far_z - near_z);

            return result;
        }

        template <class T>
        [[nodiscard]] forceinline Matrix<4, 4, T> Perspective(const T &fov, const T &aspect, const T &near_z, const T &far_z) noexcept
        {
            Matrix<4, 4, T> result;

            T const tan_half_fov = std::tan(fov / T(2));

            result[0][0] = T(1) / (aspect * tan_half_fov);
            result[1][1] = T(1) / (tan_half_fov);
            result[2][3] = T(1);

            result[2][2] = (far_z + near_z) / (far_z - near_z);
            result[3][2] = (T(-2) * far_z * near_z) / (far_z - near_z);
            result[3][3] = T(0);

            return result;
        }

        template <typename T>
        [[nodiscard]] forceinline Matrix<4, 4, T> PerspectiveInfReversedZ(const T &fov, const T &aspect, const T &near_z) noexcept
        {
            Matrix<4, 4, T> result;

            T const tan_half_fov = std::tan(fov / T(2));

            result[0][0] = T(1) / (aspect * tan_half_fov);
            result[1][1] = T(1) / (tan_half_fov);
            result[2][2] = T(0);
            result[2][3] = T(1);
            result[3][2] = near_z;
            result[3][3] = T(0);

            return result;
        }

        template <typename T>
        forceinline void ExtractFrustum(Vector<4, T> (&planes)[6], const Matrix<4, 4, T> &mtx, bool inf_reversed_z) noexcept
        {
            auto m = Transpose(mtx);
            planes[0] = m[3] + m[0];                                        // left
            planes[1] = m[3] - m[0];                                        // right
            planes[2] = m[3] + m[1];                                        // bottom
            planes[3] = m[3] - m[1];                                        // top
            planes[4] = m[3] + m[2];                                        // near
            planes[5] = inf_reversed_z ? Vector<4, T>{0.f} : (m[3] - m[2]); // far
        }

        // decompose

        template <class T>
        [[nodiscard]] forceinline Vector<3, T> DecomposePosition(Matrix<4, 4, T> const &mat) noexcept
        {
            return mat[3].xyz;
        }

        template <class T>
        [[nodiscard]] forceinline Vector<3, T> DecomposeScale(Matrix<4, 4, T> const &mat) noexcept
        {
            Vector<3, T> result;

            result.x = Length(mat[0].xyz);
            result.y = Length(mat[1].xyz);
            result.z = Length(mat[2].xyz);

            return result;
        }

    }

    // operators
    template <uint8_t R, uint8_t C, class T>
    [[nodiscard]] forceinline bool operator==(const Matrix<R, C, T> &lhs, const Matrix<R, C, T> &rhs) noexcept
    {
        return Math::Equals(lhs, rhs);
    }

}