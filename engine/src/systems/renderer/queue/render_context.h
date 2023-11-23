#pragma once

namespace Be::System::Renderer
{

    class RenderContext final
    {
    public:
        forceinline void SetCamera(const Matrix4x4 &projection, const Matrix4x4 &view,
                                   const Vector3 &position, const Vector3 &direction)
        {
            m_projection_matrix = projection;
            m_view_matrix = view;
            m_camera_position = position;
            m_camera_direction = direction;
        }

        [[nodiscard]] forceinline const Matrix4x4 &GetProjectionMatrix() const
        {
            return m_projection_matrix;
        }

        [[nodiscard]] forceinline const Matrix4x4 &GetViewMatrix() const
        {
            return m_view_matrix;
        }

        [[nodiscard]] forceinline const Vector3 &GetCameraPosition() const
        {
            return m_camera_position;
        }

        [[nodiscard]] forceinline const Vector3 &GetCameraDirection() const
        {
            return m_camera_direction;
        }

    private:
        Matrix4x4 m_projection_matrix;
        Matrix4x4 m_view_matrix;
        Vector3 m_camera_position;
        Vector3 m_camera_direction;
    };

}