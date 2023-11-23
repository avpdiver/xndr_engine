#include "base/base.h"

#include <GLFW/glfw3.h>

#define GET_WINDOW static_cast<WindowData *>(glfwGetWindowUserPointer(glfw_window))->window

namespace Be
{

    static String StringifyResultGlfw(int32_t result) noexcept
    {
        switch (result)
        {
        case GLFW_TRUE:
            return "Success";
        case GLFW_NOT_INITIALIZED:
            return "GLFW has not been initialized";
        case GLFW_NO_CURRENT_CONTEXT:
            return "No context is current for this thread";
        case GLFW_INVALID_ENUM:
            return "One of the arguments to the function was an invalid enum value";
        case GLFW_INVALID_VALUE:
            return "One of the arguments to the function was an invalid value";
        case GLFW_OUT_OF_MEMORY:
            return "A memory allocation failed";
        case GLFW_API_UNAVAILABLE:
            return "GLFW could not find support for the requested API on the system";
        case GLFW_VERSION_UNAVAILABLE:
            return "The requested OpenGL or OpenGL ES version is not available";
        case GLFW_PLATFORM_ERROR:
            return "A platform-specific error occurred that does not match any of the more specific categories";
        case GLFW_FORMAT_UNAVAILABLE:
            return "The requested format is not supported or available";
        case GLFW_NO_WINDOW_CONTEXT:
            return "The specified window does not have an OpenGL or OpenGL ES context";
        default:
            return "ERROR: UNKNOWN GLFW ERROR";
        }
    }

    static void CheckGlfw(int32_t result) noexcept
    {
        VERIFY(result, "GLFW error: {0} - {1}", result, StringifyResultGlfw(result));
    }

    static int32_t OverlappingArea(Int2 l1, Int2 r1, Int2 l2, Int2 r2) noexcept
    {
        int area1 = std::abs(l1.x - r1.x) * std::abs(l1.y - r1.y);
        int area2 = std::abs(l2.x - r2.x) * std::abs(l2.y - r2.y);
        int areaI = (std::min(r1.x, r2.x) - std::max(l1.x, l2.x)) *
                    (std::min(r1.y, r2.y) - std::max(l1.y, l2.y));
        return area1 + area2 - areaI;
    }

    void CallbackError(int32_t error, const char *description) noexcept
    {
        CheckGlfw(error);
        LOG_ERROR("GLFW error: {0}, {1}", description, error);
    }

    /*
    void CallbackMonitor(GLFWmonitor* monitor, int32_t event) noexcept
    {
        auto& monitors = ENGINE.getWindow().monitors;

        if (event == GLFW_CONNECTED)
        {
            auto& it = monitors.emplace_back(std::make_unique<Monitor>(monitor));
            ENGINE.getWindow().onMonitorConnectSignal(it.get(), true);
        }
        else if (event == GLFW_DISCONNECTED)
        {
            for (auto& m : monitors)
            {
                if (m->GetMonitor() == monitor)
                {
                    ENGINE.getWindow().onMonitorConnectSignal(m.get(), false);
                }
            }

            std::erase_if(monitors,
                         [monitor](const auto& m)
                         { return monitor == m->GetMonitor(); });
        }
    }
    */

    void CallbackWindowPosition(WindowHandle glfw_window, int32_t xpos, int32_t ypos) noexcept
    {
        auto window = GET_WINDOW;

        if (window->m_fullscreen)
        {
            return;
        }

        window->m_position = {xpos, ypos};
    }

    void CallbackWindowSize(WindowHandle glfw_window, int32_t width, int32_t height) noexcept
    {
        auto window = GET_WINDOW;

        if (width <= 0 || height <= 0)
        {
            return;
        }

        if (window->m_fullscreen)
        {
            window->m_fullscreen_size = {uint32_t(width), uint32_t(height)};
        }
        else
        {
            window->m_size = {uint32_t(width), uint32_t(height)};
        }
    }

    void CallbackWindowClose(WindowHandle glfw_window) noexcept
    {
        auto window = GET_WINDOW;
        window->m_closed = true;
    }

    void CallbackWindowFocus(WindowHandle glfw_window, int32_t focused) noexcept
    {
        auto window = GET_WINDOW;
        window->m_focused = static_cast<bool>(focused);
    }

    void CallbackWindowIconify(WindowHandle glfw_window, int32_t iconified) noexcept
    {
        auto window = GET_WINDOW;
        window->m_iconified = (iconified == GLFW_TRUE);
    }

    void CallbackFramebufferSize(WindowHandle glfw_window, int32_t width, int32_t height) noexcept
    {
        auto window = GET_WINDOW;

        if (window->m_fullscreen)
        {
            window->m_fullscreen_size = {uint32_t(width), uint32_t(height)};
        }
        else
        {
            window->m_size = {uint32_t(width), uint32_t(height)};
        }
    }

    Window::Window(const WindowCreateInfo &create_info) noexcept
        : m_size{create_info.width, create_info.height},
          m_title{create_info.title}
    {
    }

    Window::~Window() noexcept
    {
        Destroy();
    }

    void Window::Create() noexcept
    {
        if (!m_closed)
        {
            return;
        }

        glfwSetErrorCallback(CallbackError);

        VERIFY(glfwInit() == GLFW_TRUE, "GLFW failed to initialize");
        VERIFY(glfwVulkanSupported() == GLFW_TRUE, "GLFW failed to find Vulkan support");

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);
        glfwWindowHint(GLFW_STEREO, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        int32_t monitorCount;
        auto glfwMonitors = glfwGetMonitors(&monitorCount);

        for (uint32_t i = 0; i < uint32_t(monitorCount); i++)
        {
            m_monitors.emplace_back(std::make_unique<Monitor>(glfwMonitors[i]));
        }

        auto video_mode = m_monitors[0]->GetVideoMode();

        m_handle = glfwCreateWindow(int(m_size.x),
                                    int(m_size.y),
                                    m_title.c_str(),
                                    nullptr, nullptr);
        if (!m_handle)
        {
            glfwTerminate();
            FATAL("GLFW failed to Create the window");
        }

        m_data.window = this;
        glfwSetWindowUserPointer(m_handle, &m_data);

        glfwSetWindowAttrib(m_handle, GLFW_DECORATED, !m_borderless);
        glfwSetWindowAttrib(m_handle, GLFW_RESIZABLE, m_resizable);
        glfwSetWindowAttrib(m_handle, GLFW_FLOATING, m_floating);

        m_position.x = (video_mode.width - int(m_size.x)) / 2;
        m_position.y = (video_mode.height - int(m_size.y)) / 2;
        glfwSetWindowPos(m_handle, m_position.x, m_position.y);

        if (m_fullscreen)
        {
            SetFullscreen(true);
        }

        glfwShowWindow(m_handle);

        // glfwSetMonitorCallback(CallbackMonitor);
        glfwSetWindowPosCallback(m_handle, CallbackWindowPosition);
        glfwSetWindowSizeCallback(m_handle, CallbackWindowSize);
        glfwSetWindowCloseCallback(m_handle, CallbackWindowClose);
        glfwSetWindowFocusCallback(m_handle, CallbackWindowFocus);
        glfwSetWindowIconifyCallback(m_handle, CallbackWindowIconify);
        glfwSetFramebufferSizeCallback(m_handle, CallbackFramebufferSize);

        m_closed = false;

        LOG_INFO("Window is created.");
    }

    void Window::Destroy() noexcept
    {
        glfwDestroyWindow(m_handle);
        glfwTerminate();
        m_closed = true;
        LOG_INFO("Window is destroyed.");
    }

    void Window::Update(const EngineTime &engine_time) noexcept
    {
        glfwPollEvents();
    }

    void Window::SetSize(const UInt2 &size) noexcept
    {
        glfwSetWindowSize(m_handle, size.x, size.y);
        if (m_fullscreen)
        {
            m_fullscreen_size = size;
        }
        else
        {
            m_size = size;
        }
    }

    void Window::SetPosition(const Int2 &position) noexcept
    {
        glfwSetWindowPos(m_handle, position.x, position.y);
        m_position = position;
    }

    void Window::SetTitle(const String &title) noexcept
    {
        m_title = title;
        glfwSetWindowTitle(m_handle, m_title.c_str());
    }

    void Window::SetBorderless(bool borderless) noexcept
    {
        m_borderless = borderless;
        glfwSetWindowAttrib(m_handle, GLFW_DECORATED, !m_borderless);
    }

    void Window::SetResizable(bool resizable) noexcept
    {
        m_resizable = resizable;
        glfwSetWindowAttrib(m_handle, GLFW_RESIZABLE, m_resizable);
    }

    void Window::SetFloating(bool floating) noexcept
    {
        m_floating = floating;
        glfwSetWindowAttrib(m_handle, GLFW_FLOATING, m_floating);
    }

    void Window::SetFullscreen(bool fullscreen, Monitor *monitor) noexcept
    {
        auto selected = monitor ? monitor : GetCurrentMonitor();
        auto video_mode = selected->GetVideoMode();

        m_fullscreen = fullscreen;

        if (m_fullscreen)
        {
            glfwSetWindowMonitor(m_handle,
                                 selected->Handle(),
                                 0, 0,
                                 video_mode.width,
                                 video_mode.height,
                                 GLFW_DONT_CARE);
        }
        else
        {
            Int2 pos{video_mode.width, video_mode.height};
            pos = (pos - Int2(m_size)) / 2 + selected->GetPosition();
            glfwSetWindowMonitor(m_handle,
                                 nullptr,
                                 pos.x, pos.y,
                                 m_size.x, m_size.y,
                                 GLFW_DONT_CARE);
        }
    }

    void Window::SetIconified(bool iconify) noexcept
    {
        if (!m_iconified && iconify)
        {
            glfwIconifyWindow(m_handle);
        }
        else if (m_iconified && !iconify)
        {
            glfwRestoreWindow(m_handle);
        }
    }

    const Monitor *Window::GetPrimaryMonitor() const noexcept
    {
        for (const auto &monitor : m_monitors)
        {
            if (monitor->IsPrimary())
            {
                return monitor.get();
            }
        }
        return nullptr;
    }

    const Monitor *Window::GetCurrentMonitor() const noexcept
    {
        if (m_fullscreen)
        {
            auto glfw_monitor = glfwGetWindowMonitor(m_handle);
            for (const auto &monitor : m_monitors)
            {
                if (monitor->m_handle == glfw_monitor)
                {
                    return monitor.get();
                }
            }
            return nullptr;
        }

        std::multimap<int32_t, const Monitor *> ranked_monitor;

        for (const auto &monitor : m_monitors)
        {
            ranked_monitor.emplace(
                OverlappingArea(
                    monitor->GetWorkareaPosition(),
                    monitor->GetWorkareaPosition() + monitor->GetWorkareaSize(),
                    m_position,
                    m_position + Int2{m_size}),
                monitor.get());
        }

        if (std::cbegin(ranked_monitor)->first > 0)
        {
            return std::cbegin(ranked_monitor)->second;
        }

        return nullptr;
    }

}
