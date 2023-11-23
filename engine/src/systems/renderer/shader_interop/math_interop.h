#pragma once

namespace Be::System::Renderer::ShaderInterop
{

    using vec2 = Vector<2, float>;
    using vec3 = Vector<3, float>;
    using vec4 = Vector<4, float>;

    using dvec2 = Vector<2, double>;
    using dvec3 = Vector<3, double>;
    using dvec4 = Vector<4, double>;

    using bvec2 = Vector<2, bool>;
    using bvec3 = Vector<3, bool>;
    using bvec4 = Vector<4, bool>;

    using ivec2 = Vector<2, int32_t>;
    using ivec3 = Vector<3, int32_t>;
    using ivec4 = Vector<4, int32_t>;

    using uvec2 = Vector<2, uint32_t>;
    using uvec3 = Vector<3, uint32_t>;
    using uvec4 = Vector<4, uint32_t>;

    using mat2 = Matrix<2, 2, float>;
    using mat3 = Matrix<3, 3, float>;
    using mat4 = Matrix<4, 4, float>;
    using mat2x2 = Matrix<2, 2, float>;
    using mat3x3 = Matrix<3, 3, float>;
    using mat4x4 = Matrix<3, 3, float>;

    using dmat2 = Matrix<2, 2, double>;
    using dmat3 = Matrix<3, 3, double>;
    using dmat4 = Matrix<4, 4, double>;
    using dmat2x2 = Matrix<2, 2, double>;
    using dmat3x3 = Matrix<3, 3, double>;
    using dmat4x4 = Matrix<3, 3, double>;

}