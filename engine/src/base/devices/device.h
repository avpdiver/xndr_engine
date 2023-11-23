#pragma once

namespace Be
{

    class IDevice : public Noncopyable
    {
    public:
        virtual ~IDevice() noexcept
        {
        }

    public:
        virtual void Create() noexcept {}
        virtual void Destroy() noexcept {}
        virtual void Update(const EngineTime &engine_time) noexcept {}
    };

    UNIQUE(IDevice);

}