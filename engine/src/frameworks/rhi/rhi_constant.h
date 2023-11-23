#pragma once

namespace Be::Framework::RHI
{    
    constexpr uint32_t MAX_GPU_BUFFERS = 128;
    constexpr uint32_t MAX_GPU_IMAGES = 128;
    constexpr uint32_t MAX_COLOR_ATTACHMENTS = 8;
    constexpr uint32_t MAX_DESCRIPTOR_SETS = 4;
    constexpr uint32_t MAX_BINDINGS = 32;
    constexpr uint32_t MAX_BINDINGS_BINDLESS_VARYING = 16 * 1024;
    constexpr uint32_t MAX_BINDLESS_HEAP_SIZE = 1000;
}