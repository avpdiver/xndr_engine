#pragma once

namespace Be
{

    template <typename T>
    struct BoundingBox
    {
    public:
        Vector<3, T> min;
        Vector<3, T> max;

    public:
        BoundingBox() = default;
        BoundingBox(const Vector<3, T> &min, const Vector<3, T> &max) : min{min}, max{max} {}
        BoundingBox(const Vector<3, T> *points, size_t num_points)
        {
            Vector<3, T> vmin(MaxValue);
            Vector<3, T> vmax(MinValue);

            for (size_t i = 0; i != num_points; i++)
            {
                vmin = Math::Min(vmin, points[i]);
                vmax = Math::Max(vmax, points[i]);
            }
            min = vmin;
            max = vmax;
        }

    public:
        template <typename O>
        constexpr BoundingBox<T> &operator=(const BoundingBox<O> &o)
        {
            min = o.min;
            max = o.max;
            return *this;
        }

    public:
        void CombinePoint(const Vector<3, T> &p)
        {
            min = Math::Min(min, p);
            max = Math::Max(max, p);
        }

    public:
        [[nodiscard]] Vector<3, T> GetSize() const
        {
            return Vector<3, T>(max[0] - min[0], max[1] - min[1], max[2] - min[2]);
        }

        [[nodiscard]] Vector<3, T> GetCenter() const
        {
            return T(0.5) * Vector<3, T>(max[0] + min[0], max[1] + min[1], max[2] + min[2]);
        }

    public:
        void Transform(const Matrix<4, 4, T> &t)
        {
            Vector<3, T> corners[] = {
                {min.x, min.y, min.z},
                {min.x, max.y, min.z},
                {min.x, min.y, max.z},
                {min.x, max.y, max.z},
                {max.x, min.y, min.z},
                {max.x, max.y, min.z},
                {max.x, min.y, max.z},
                {max.x, max.y, max.z},
            };
            for (auto &v : corners)
            {
                v = t * Vector<4, T>(v, T(1));
            }
            *this = BoundingBox<T>(corners, 8);
        }

        [[nodiscard]] BoundingBox<T> GetTransformed(const Matrix<4, 4, T> &t) const
        {
            BoundingBox<T> b = *this;
            b.Transform(t);
            return b;
        }
    };

    using BBoxf = BoundingBox<float>;
    using BBoxd = BoundingBox<double>;
    using BBox = BoundingBox<BE_MATH_PRECISION>;

}