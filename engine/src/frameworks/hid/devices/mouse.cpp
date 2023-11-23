#include "frameworks/hid/hid.h"

#include <GLFW/glfw3.h>

#define WINDOW_MOUSE(GLFW_HANDLE) ((Mouse *)(WINDOWS_DATA(GLFW_HANDLE)->hid_devices.at(HidMouseHandle.Hash())))

namespace Be::Framework::HID
{

    void CallbackMouseButton(WindowHandle handle, int32_t button, int32_t action, int32_t mods) noexcept
    {
        Unused(handle);
    }

    void CallbackCursorPos(WindowHandle handle, double xpos, double ypos) noexcept
    {
        auto mouse = WINDOW_MOUSE(handle);

        mouse->m_position = {xpos, ypos};
        mouse->m_position_delta = (mouse->m_last_position - mouse->m_position);
        mouse->m_last_position = mouse->m_position;
    }

    void CallbackCursorEnter(WindowHandle handle, int32_t entered) noexcept
    {
        WINDOW_MOUSE(handle)->m_window_selected = (entered == GLFW_TRUE);
    }

    void CallbackScroll(WindowHandle handle, double xoffset, double yoffset) noexcept
    {
        auto mouse = WINDOW_MOUSE(handle);

        mouse->m_scroll = {xoffset, yoffset};
        mouse->m_scroll_delta = (mouse->m_last_scroll - mouse->m_scroll);
        mouse->m_last_scroll = mouse->m_scroll;
    }

    void CallbackDrop(WindowHandle handle, int32_t count, const char **paths) noexcept
    {
        Unused(handle);

        Array<String> files(static_cast<uint32_t>(count));
        for (uint32_t i = 0; i < static_cast<uint32_t>(count); i++)
        {
            files[i] = paths[i];
        }
    }

    Mouse::Mouse(Window &window) noexcept
        : m_window{window}
    {
    }

    Mouse::~Mouse() noexcept
    {
        Destroy();
    }

    void Mouse::Create() noexcept
    {
        const auto handle = m_window.Handle();

        glfwSetMouseButtonCallback(handle, CallbackMouseButton);
        glfwSetCursorPosCallback(handle, CallbackCursorPos);
        glfwSetCursorEnterCallback(handle, CallbackCursorEnter);
        glfwSetScrollCallback(handle, CallbackScroll);
        glfwSetDropCallback(handle, CallbackDrop);

        WINDOWS_DATA(handle)->hid_devices[HidMouseHandle.Hash()] = this;

        LOG_INFO("Mouse init.");
    }

    void Mouse::Destroy() noexcept
    {
        const auto handle = m_window.Handle();
        glfwSetInputMode(handle, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwDestroyCursor(m_cursor);

        LOG_INFO("\tMouse is destroyed.");
    }

    void Mouse::SetCursor(CursorStandard standard) noexcept
    {
        if (m_current_standard == standard)
        {
            return;
        }

        glfwDestroyCursor(m_cursor);

        m_cursor = glfwCreateStandardCursor(static_cast<int32_t>(standard));

        glfwSetCursor(m_window.Handle(), m_cursor);
        m_current_cursor = EmptyOptional;
        m_current_standard = standard;
    }

    String Mouse::GetClipboard() const noexcept
    {
        return glfwGetClipboardString(m_window.Handle());
    }

    void Mouse::SetClipboard(const String &string) const noexcept
    {
        glfwSetClipboardString(m_window.Handle(), string.c_str());
    }

    HidAction Mouse::GetButton(MouseButton mouse_button) const noexcept
    {
        auto state = glfwGetMouseButton(m_window.Handle(), static_cast<int32_t>(mouse_button));
        return static_cast<HidAction>(state);
    }

    void Mouse::SetPosition(const Double2 &position) noexcept
    {
        m_last_position = position;
        m_position = position;
        glfwSetCursorPos(m_window.Handle(), m_position.x, m_position.y);
    }

    void Mouse::SetScroll(const Double2 &scroll) noexcept
    {
        m_last_scroll = scroll;
        m_scroll = scroll;
    }

    void Mouse::SetCursorHidden(bool hidden) noexcept
    {
        if (m_cursor_hidden != hidden)
        {
            glfwSetInputMode(m_window.Handle(), GLFW_CURSOR, hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            glfwSetInputMode(m_window.Handle(), GLFW_RAW_MOUSE_MOTION, hidden ? GLFW_TRUE : GLFW_FALSE);

            if (!hidden && m_cursor_hidden)
            {
                SetPosition(m_position);
            }
        }
        m_cursor_hidden = hidden;
    }

    double Mouse::SmoothScrollWheel(double value, float delta) noexcept
    {
        if (value != 0.0)
        {
            value -= static_cast<double>(delta) * std::copysign(3.0, value);
            value = Math::DeadZone(0.08, value);
            return value;
        }
        return 0.0;
    }

}
