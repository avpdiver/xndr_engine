#pragma once

struct GLFWcursor;

namespace Be::Framework::HID
{

    enum class MouseButton : uint8_t
    {
        e1 = 0,
        e2 = 1,
        e3 = 2,
        e4 = 3,
        e5 = 4,
        e6 = 5,
        e7 = 6,
        e8 = 7,
        eLeft = e1,
        eRight = e2,
        eMiddle = e3
    };

    enum class CursorHotspot : uint8_t
    {
        eUpperLeft,
        eUpperRight,
        eBottomLeft,
        eBottomRight,
        eCentered
    };

    enum class CursorStandard : uint32_t
    {
        eArrow = 0x00036001,
        eIBeam = 0x00036002,
        eCrosshair = 0x00036003,
        eHand = 0x00036004,
        eResizeX = 0x00036005,
        eResizeY = 0x00036006
    };

    class Mouse final : public IHidDevice
    {
    public:
        Mouse(Window &window) noexcept;
        ~Mouse() noexcept;

    public:
        void Create() noexcept override;
        void Destroy() noexcept override;

    public:
        HidAction GetButton(MouseButton mouseButton) const noexcept;

        void SetCursor(const Path &filename, CursorHotspot hotspot) noexcept;

        void SetCursor(CursorStandard standard) noexcept;

        String GetClipboard() const noexcept;

        void SetClipboard(const String &string) const noexcept;

    public:
        [[nodiscard]] forceinline const Double2 &GetPosition() const noexcept
        {
            return m_position;
        }

        [[nodiscard]] forceinline const Double2 &GetPositionDelta() const noexcept
        {
            return m_position_delta;
        }

        void SetPosition(const Double2 &position) noexcept;

    public:
        [[nodiscard]] forceinline const Double2 &GetScroll() const noexcept
        {
            return m_scroll;
        }

        [[nodiscard]] forceinline const Double2 &GetScrollDelta() const noexcept
        {
            return m_scroll_delta;
        }

        void SetScroll(const Double2 &scroll) noexcept;

    public:
        [[nodiscard]] forceinline bool IsWindowSelected() const noexcept
        {
            return m_window_selected;
        }

        [[nodiscard]] forceinline bool IsCursorHidden() const noexcept
        {
            return m_cursor_hidden;
        }

        void SetCursorHidden(bool hidden) noexcept;

    private:
        static double SmoothScrollWheel(double value, float delta) noexcept;

    private:
        friend void CallbackMouseButton(WindowHandle window, int32_t button, int32_t action, int32_t mods) noexcept;
        friend void CallbackCursorPos(WindowHandle window, double xpos, double ypos) noexcept;
        friend void CallbackCursorEnter(WindowHandle window, int32_t entered) noexcept;
        friend void CallbackScroll(WindowHandle window, double xoffset, double yoffset) noexcept;
        friend void CallbackDrop(WindowHandle window, int32_t count, const char **paths) noexcept;

    private:
        Optional<Pair<Path, CursorHotspot>> m_current_cursor;
        Optional<CursorStandard> m_current_standard;
        GLFWcursor *m_cursor{nullptr};

    private:
        Double2 m_last_position;
        Double2 m_position;
        Double2 m_position_delta;
        Double2 m_last_scroll;
        Double2 m_scroll;
        Double2 m_scroll_delta;

    private:
        bool m_window_selected{false};
        bool m_cursor_hidden{false};

    private:
        Window &m_window;
    };

    UNIQUE(Mouse);

    inline constexpr HidDeviceName HidMouseHandle{"Mouse"};

}
