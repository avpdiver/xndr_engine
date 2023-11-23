#pragma once

namespace Be
{
    namespace MemSize
    {
        [[nodiscard]] forceinline usize_t Kb(usize_t value) noexcept { return value >> 10; }
        [[nodiscard]] forceinline usize_t Mb(usize_t value) noexcept { return value >> 20; }
        [[nodiscard]] forceinline usize_t Gb(usize_t value) noexcept { return value >> 30; }

        [[nodiscard]] forceinline usize_t FromBits(usize_t value) noexcept { return (value >> 3); }
        [[nodiscard]] forceinline usize_t FromKb(usize_t value) noexcept { return (value << 10); }
        [[nodiscard]] forceinline usize_t FromMb(usize_t value) noexcept { return (value << 20); }
        [[nodiscard]] forceinline usize_t FromGb(usize_t value) noexcept { return (value << 30); }
    }

    [[nodiscard]] constexpr usize_t operator"" _b(unsigned long long value) noexcept { return value; }
    [[nodiscard]] constexpr usize_t operator"" _Kb(unsigned long long value) noexcept { return MemSize::FromKb(value); }
    [[nodiscard]] constexpr usize_t operator"" _Mb(unsigned long long value) noexcept { return MemSize::FromMb(value); }
    [[nodiscard]] constexpr usize_t operator"" _Gb(unsigned long long value) noexcept { return MemSize::FromGb(value); }

}