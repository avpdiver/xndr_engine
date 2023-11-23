#include "frameworks/hid/hid.h"

#include <GLFW/glfw3.h>

#define WINDOW_KEYBOARD(GLFW_HANDLE) ((Keyboard *)(WINDOWS_DATA(GLFW_HANDLE)->hid_devices.at(HidKeyboardHandle.Hash())))

namespace Be::Framework::HID
{

    void CallbackKey(WindowHandle handle, int32_t key,
                     [[maybe_unused]] int32_t scancode,
                     int32_t action, int32_t mods) noexcept
    {
    }

    void CallbackChar(WindowHandle handle, uint32_t codepoint) noexcept
    {
    }

    Keyboard::Keyboard(Window &window) noexcept
        : m_window{window}
    {
    }

    Keyboard::~Keyboard() noexcept
    {
        Destroy();
    }

    void Keyboard::Create() noexcept
    {
        const auto handle = m_window.Handle();

        glfwSetKeyCallback(handle, CallbackKey);
        glfwSetCharCallback(handle, CallbackChar);

        WINDOWS_DATA(handle)->hid_devices[HidKeyboardHandle.Hash()] = this;
    }

    void Keyboard::Destroy() noexcept
    {
        LOG_INFO("\tKeyboard is destroyed.");
    }

    void Keyboard::Update(const EngineTime &engine_time) noexcept
    {
    }

    HidAction Keyboard::GetKey(EKey key) const noexcept
    {
        auto state = glfwGetKey(m_window.Handle(), static_cast<int32_t>(key));
        return static_cast<HidAction>(state);
    }

    String Keyboard::ToString(EKey key) noexcept
    {
        switch (key)
        {
        case EKey::Space:
            return "Space";
        case EKey::Apostrophe:
            return "Apostrophe";
        case EKey::Comma:
            return "Comma";
        case EKey::Minus:
            return "Minus";
        case EKey::Period:
            return "Period";
        case EKey::Slash:
            return "Slash";
        case EKey::_0:
            return "0";
        case EKey::_1:
            return "1";
        case EKey::_2:
            return "2";
        case EKey::_3:
            return "3";
        case EKey::_4:
            return "4";
        case EKey::_5:
            return "5";
        case EKey::_6:
            return "6";
        case EKey::_7:
            return "7";
        case EKey::_8:
            return "8";
        case EKey::_9:
            return "9";
        case EKey::Semicolon:
            return "Semicolon";
        case EKey::Equal:
            return "Equal";
        case EKey::A:
            return "A";
        case EKey::B:
            return "B";
        case EKey::C:
            return "C";
        case EKey::D:
            return "D";
        case EKey::E:
            return "E";
        case EKey::F:
            return "F";
        case EKey::G:
            return "G";
        case EKey::H:
            return "H";
        case EKey::I:
            return "I";
        case EKey::J:
            return "J";
        case EKey::K:
            return "K";
        case EKey::L:
            return "L";
        case EKey::M:
            return "M";
        case EKey::N:
            return "N";
        case EKey::O:
            return "O";
        case EKey::P:
            return "P";
        case EKey::Q:
            return "Q";
        case EKey::R:
            return "R";
        case EKey::S:
            return "S";
        case EKey::T:
            return "T";
        case EKey::U:
            return "U";
        case EKey::V:
            return "V";
        case EKey::W:
            return "W";
        case EKey::X:
            return "X";
        case EKey::Y:
            return "Y";
        case EKey::Z:
            return "Z";
        case EKey::LeftBracket:
            return "Left Bracket";
        case EKey::Backslash:
            return "Backslash";
        case EKey::RightBracket:
            return "Right Bracket";
        case EKey::GraveAccent:
            return "Grave Accent";
        case EKey::World1:
            return "World1";
        case EKey::World2:
            return "World2";
        case EKey::Escape:
            return "Escape";
        case EKey::Enter:
            return "Enter";
        case EKey::Tab:
            return "Tab";
        case EKey::Backspace:
            return "Backspace";
        case EKey::Insert:
            return "Insert";
        case EKey::Delete:
            return "Delete";
        case EKey::Right:
            return "Right";
        case EKey::Left:
            return "Left";
        case EKey::Down:
            return "Down";
        case EKey::Up:
            return "Up";
        case EKey::PageUp:
            return "Page Up";
        case EKey::PageDown:
            return "Page Down";
        case EKey::Home:
            return "Home";
        case EKey::End:
            return "End";
        case EKey::CapsLock:
            return "Caps Lock";
        case EKey::ScrollLock:
            return "Scroll Lock";
        case EKey::NumLock:
            return "Num Lock";
        case EKey::PrintScreen:
            return "Print Screen";
        case EKey::Pause:
            return "Pause";
        case EKey::F1:
            return "F1";
        case EKey::F2:
            return "F2";
        case EKey::F3:
            return "F3";
        case EKey::F4:
            return "F4";
        case EKey::F5:
            return "F5";
        case EKey::F6:
            return "F6";
        case EKey::F7:
            return "F7";
        case EKey::F8:
            return "F8";
        case EKey::F9:
            return "F9";
        case EKey::F10:
            return "F10";
        case EKey::F11:
            return "F11";
        case EKey::F12:
            return "F12";
        case EKey::F13:
            return "F13";
        case EKey::F14:
            return "F14";
        case EKey::F15:
            return "F15";
        case EKey::F16:
            return "F16";
        case EKey::F17:
            return "F17";
        case EKey::F18:
            return "F18";
        case EKey::F19:
            return "F19";
        case EKey::F20:
            return "F20";
        case EKey::F21:
            return "F21";
        case EKey::F22:
            return "F22";
        case EKey::F23:
            return "F23";
        case EKey::F24:
            return "F24";
        case EKey::F25:
            return "F25";
        case EKey::Numpad0:
            return "Numpad 0";
        case EKey::Numpad1:
            return "Numpad 1";
        case EKey::Numpad2:
            return "Numpad 2";
        case EKey::Numpad3:
            return "Numpad 3";
        case EKey::Numpad4:
            return "Numpad 4";
        case EKey::Numpad5:
            return "Numpad 5";
        case EKey::Numpad6:
            return "Numpad 6";
        case EKey::Numpad7:
            return "Numpad 7";
        case EKey::Numpad8:
            return "Numpad 8";
        case EKey::Numpad9:
            return "Numpad 9";
        case EKey::NumpadDecimal:
            return "Numpad Decimal";
        case EKey::NumpadDivide:
            return "Numpad Divide";
        case EKey::NumpadMultiply:
            return "Numpad Multiply";
        case EKey::NumpadSubtract:
            return "Numpad Subtract";
        case EKey::NumpadAdd:
            return "Numpad Add";
        case EKey::NumpadEnter:
            return "Numpad Enter";
        case EKey::NumpadEqual:
            return "Numpad Equal";
        case EKey::ShiftLeft:
            return "Shift Left";
        case EKey::ControlLeft:
            return "Control Left";
        case EKey::AltLeft:
            return "Alt Left";
        case EKey::SuperLeft:
            return "Super Left";
        case EKey::ShiftRight:
            return "Shift Right";
        case EKey::ControlRight:
            return "Control Right";
        case EKey::AltRight:
            return "Alt Right";
        case EKey::SuperRight:
            return "Super Right";
        case EKey::Menu:
            return "Menu";
        default:
            return "Undefined";
        }
    }
}
