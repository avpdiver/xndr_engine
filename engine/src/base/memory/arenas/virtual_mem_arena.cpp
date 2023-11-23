#include "base/base.h"

namespace Be
{

    VirtualMemoryArena::VirtualMemoryArena(usize_t size) noexcept
        : m_size{size}
    {
        m_begin = Platform::AllocateVirtualMemory(size);
        m_end = static_cast<byte_t *>(m_begin) + m_size;
    }

    VirtualMemoryArena::~VirtualMemoryArena() noexcept
    {
        Platform::FreeVirtualMemory(m_begin, m_size);
    }

}