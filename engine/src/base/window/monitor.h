#pragma once

struct GLFWmonitor;

namespace Be
{

    class Window;

    struct VideoMode final
    {
        int32_t width;
        int32_t height;
        int32_t red_bits;
        int32_t green_bits;
        int32_t blue_bits;
        int32_t refresh_rate;
    };

    struct GammaRamp final
    {
        uint16_t *red;
        uint16_t *green;
        uint16_t *blue;
        uint32_t size;
    };

    class Monitor final : public Noncopyable
    {
        friend class Window;

    public:
        explicit Monitor(GLFWmonitor *monitor = nullptr);

    public:
        GLFWmonitor *Handle() const noexcept
        {
            return m_handle;
        }

        bool IsPrimary() const noexcept;

        UInt2 GetWorkareaSize() const noexcept;

        UInt2 GetWorkareaPosition() const noexcept;

        UInt2 GetSize() const noexcept;

        Float2 GetContentScale() const noexcept;

        Int2 GetPosition() const noexcept;

        String GetName() const noexcept;

        Array<VideoMode> GetVideoModes() const noexcept;

        VideoMode GetVideoMode() const noexcept;

        GammaRamp GetGammaRamp() const noexcept;

        void SetGammaRamp(const GammaRamp &GammaRamp) const noexcept;

    private:
        GLFWmonitor *m_handle;
    };

    UNIQUE(Monitor);

}
