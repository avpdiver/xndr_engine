#include "base/base.h"

#include <GLFW/glfw3.h>

namespace Be
{

    Monitor::Monitor(GLFWmonitor *monitor) : m_handle(monitor)
    {
    }

    bool Monitor::IsPrimary() const noexcept
    {
        return m_handle == glfwGetPrimaryMonitor();
    }

    UInt2 Monitor::GetWorkareaSize() const noexcept
    {
        int32_t width;
        int32_t height;
        glfwGetMonitorWorkarea(m_handle, nullptr, nullptr, &width, &height);
        return {static_cast<UInt2::scalar_type>(width), static_cast<UInt2::scalar_type>(height)};
    }

    UInt2 Monitor::GetWorkareaPosition() const noexcept
    {
        int32_t x;
        int32_t y;
        glfwGetMonitorWorkarea(m_handle, &x, &y, nullptr, nullptr);
        return {static_cast<UInt2::scalar_type>(x), static_cast<UInt2::scalar_type>(y)};
    }

    UInt2 Monitor::GetSize() const noexcept
    {
        int32_t width;
        int32_t height;
        glfwGetMonitorPhysicalSize(m_handle, &width, &height);
        return {static_cast<UInt2::scalar_type>(width), static_cast<UInt2::scalar_type>(height)};
    }

    Float2 Monitor::GetContentScale() const noexcept
    {
        float x;
        float y;
        glfwGetMonitorContentScale(m_handle, &x, &y);
        return {x, y};
    }

    Int2 Monitor::GetPosition() const noexcept
    {
        int32_t x;
        int32_t y;
        glfwGetMonitorPos(m_handle, &x, &y);
        return {x, y};
    }

    String Monitor::GetName() const noexcept
    {
        return glfwGetMonitorName(m_handle);
    }

    Array<VideoMode> Monitor::GetVideoModes() const noexcept
    {
        int32_t count;
        auto videoModes = glfwGetVideoModes(m_handle, &count);
        Array<VideoMode> modes(static_cast<uint32_t>(count));
        for (uint32_t i = 0; i < static_cast<uint32_t>(count); i++)
        {
            modes[i] = *reinterpret_cast<const VideoMode *>(&videoModes[i]);
        }
        return modes;
    }

    VideoMode Monitor::GetVideoMode() const noexcept
    {
        auto mode = glfwGetVideoMode(m_handle);
        return *reinterpret_cast<const VideoMode *>(mode);
    }

    GammaRamp Monitor::GetGammaRamp() const noexcept
    {
        auto ramp = glfwGetGammaRamp(m_handle);
        return *reinterpret_cast<const GammaRamp *>(ramp);
    }

    void Monitor::SetGammaRamp(const GammaRamp &gammaRamp) const noexcept
    {
        auto ramp = reinterpret_cast<const GLFWgammaramp *>(&gammaRamp);
        glfwSetGammaRamp(m_handle, ramp);
    }
}
