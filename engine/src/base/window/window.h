#pragma once

#include "base/window/monitor.h"

struct GLFWwindow;

#define WINDOWS_DATA(GLFW_HANDLE) static_cast<WindowData *>(glfwGetWindowUserPointer(GLFW_HANDLE))

namespace Be
{

    struct WindowData
    {
        Window *window{nullptr};
        Map<HashValue, IDevice *> hid_devices;
    };

    using WindowHandle = GLFWwindow *;

    struct WindowCreateInfo
    {
        uint32_t width{0};
        uint32_t height{0};
        String title;
    };

    class Window final : public IDevice
    {
    public:
        explicit Window(const WindowCreateInfo &create_info) noexcept;
        ~Window() noexcept;

    public:
        [[nodiscard]] forceinline WindowHandle Handle() const noexcept
        {
            return m_handle;
        }

    public:
        void Create() noexcept override;
        void Destroy() noexcept override;
        void Update(const EngineTime &engine_time) noexcept override;

    public:
        [[nodiscard]] forceinline const String &GetTitle() const noexcept
        {
            return m_title;
        }

        void SetTitle(const String &title) noexcept;

    public:
        [[nodiscard]] forceinline const UInt2 &GetSize(bool check_fullscreen = true) const noexcept
        {
            return (m_fullscreen && check_fullscreen) ? m_fullscreen_size : m_size;
        }

        [[nodiscard]] forceinline float GetAspectRatio() const noexcept
        {
            return float(GetSize().x) / float(GetSize().y);
        }

        void SetSize(const UInt2 &size) noexcept;

    public:
        [[nodiscard]] forceinline const Int2 &GetPosition() const noexcept
        {
            return m_position;
        }

        void SetPosition(const Int2 &position) noexcept;

    public:
        [[nodiscard]] forceinline bool IsBorderless() const noexcept
        {
            return m_borderless;
        }

        void SetBorderless(bool borderless) noexcept;

    public:
        [[nodiscard]] forceinline bool IsResizable() const noexcept
        {
            return m_resizable;
        }

        void SetResizable(bool resizable) noexcept;

    public:
        [[nodiscard]] forceinline bool IsFloating() const noexcept
        {
            return m_floating;
        }

        void SetFloating(bool floating) noexcept;

    public:
        [[nodiscard]] forceinline bool IsFullscreen() const noexcept
        {
            return m_fullscreen;
        }

        void SetFullscreen(bool fullscreen, Monitor *monitor = nullptr) noexcept;

        [[nodiscard]] forceinline bool IsClosed() const noexcept
        {
            return m_closed;
        }

    public:
        [[nodiscard]] forceinline bool IsFocused() const noexcept
        {
            return m_focused;
        }

    public:
        [[nodiscard]] forceinline bool IsIconified() const noexcept
        {
            return m_iconified;
        }

        void SetIconified(bool iconify) noexcept;

    public:
        [[nodiscard]] forceinline const Array<MonitorUnique> &GetMonitors() const noexcept
        {
            return m_monitors;
        }

        [[nodiscard]] forceinline const Monitor *GetPrimaryMonitor() const noexcept;

        [[nodiscard]] forceinline const Monitor *GetCurrentMonitor() const noexcept;

    private:
        friend void CallbackError(int32_t error, const char *description) noexcept;
        friend void CallbackMonitor(GLFWmonitor *monitor, int32_t event) noexcept;
        friend void CallbackWindowPosition(WindowHandle window, int32_t xpos, int32_t ypos) noexcept;
        friend void CallbackWindowSize(WindowHandle window, int32_t width, int32_t height) noexcept;
        friend void CallbackWindowClose(WindowHandle window) noexcept;
        friend void CallbackWindowFocus(WindowHandle window, int32_t focused) noexcept;
        friend void CallbackWindowIconify(WindowHandle window, int32_t iconified) noexcept;
        friend void CallbackFramebufferSize(WindowHandle window, int32_t width, int32_t height) noexcept;

    private:
        UInt2 m_size;
        UInt2 m_fullscreen_size;
        Int2 m_position;

    private:
        String m_title;

        bool m_focused{true};

        bool m_borderless{false};
        bool m_resizable{false};
        bool m_floating{false};
        bool m_fullscreen{false};

        bool m_closed{true};
        bool m_iconified{false};

    private:
        WindowHandle m_handle{nullptr};
        Array<MonitorUnique> m_monitors;

    private:
        WindowData m_data;
    };

    UNIQUE(Window);

}
