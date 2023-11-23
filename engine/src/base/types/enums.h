#pragma once

#define BIT_ENUM(Name, T, ...)                                   \
    namespace BE_CONCAT(Name, Enum)                              \
    {                                                            \
        enum Enum : T                                            \
        {                                                        \
            __VA_ARGS__                                          \
        };                                                       \
        static constexpr auto Count = BE_ARGS_SIZE(__VA_ARGS__); \
    };                                                           \
    using Name = BE_CONCAT(Name, Enum)::Enum;                    \
    using BE_CONCAT(Name, Bits) = T;                             \
    BIT_OPERATORS(Name)

#define ITERABLE_ENUM(Name, Type, ...)                \
    namespace BE_CONCAT(Name, Enum)                   \
    {                                                 \
        enum Enum : Type                              \
        {                                             \
            __VA_ARGS__                               \
        };                                            \
        static constexpr Enum All[] = {__VA_ARGS__};  \
        static constexpr auto Count = std::size(All); \
        static constexpr auto First = All[0];         \
        static constexpr auto Last = All[Count - 1];  \
    };                                                \
    using Name = BE_CONCAT(Name, Enum)::Enum;
