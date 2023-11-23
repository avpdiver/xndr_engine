#pragma once

namespace Be::Framework::HID
{
    enum class HidAction : uint8_t
    {
        eRelease = 0,
        ePress = 1,
        eRepeat = 2
    };

    enum class HidModifier : uint8_t
    {
        eNone = 0,
        eShift = 1,
        eControl = 2,
        eAlt = 4,
        eSuper = 8
    };

    class IHidDevice : public IDevice
    {
    public:
        virtual ~IHidDevice() = default;
    };

    namespace details::hid
    {
        static constexpr uint32_t NamedIDStart = 1 << 24;
    }

    using HidDeviceName = NamedHandle<IHidDevice, 64, BE_OPTIMIZE_IDS>;

}

#include "frameworks/hid/devices/mouse.h"
#include "frameworks/hid/devices/keyboard.h"
