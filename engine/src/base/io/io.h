#pragma once

namespace Be
{
    class IStream;
    class OStream;

    class Stream : public MovableOnly
    {
    public:
        virtual ~Stream() noexcept = default;

    public:
        [[nodiscard]] virtual usize_t GetPosition() const = 0;
        virtual void SetPosition(usize_t pos) = 0;

    public:
        [[nodiscard]] virtual bool IsMarkSupported() const = 0;
        virtual void Mark() = 0;
        virtual void Reset() = 0;
    };
}

#include "base/io/istream.h"
#include "base/io/ostream.h"