#pragma once

namespace Be
{
    // clang-format off
    template <typename T>
    concept MemoryArena = requires(T t) {
        {t.Begin()} -> std::same_as<void *>;
        {t.End()} -> std::same_as<void *>;
        {t.Size()} -> std::same_as<usize_t>;
    };
    // clang-format on
}

#include "base/memory/arenas/malloc_mem_arena.h"
#include "base/memory/arenas/virtual_mem_arena.h"