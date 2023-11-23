#pragma once

namespace Be::Platform
{

    usize_t GetPageSize() noexcept;
    usize_t GetVirtualMemorySize() noexcept;
    void *AllocateVirtualMemory(usize_t size) noexcept;
    void FreeVirtualMemory(void *ptr, usize_t size) noexcept;

}