#pragma once

namespace Be::Framework::RHI
{

    struct RhiSubresourceData
    {
        const void *data{nullptr}; // pointer to the beginning of the subresource data (pointer to beginning of resource + subresource offset)
        usize_t row_bytes{0u};     // bytes between two rows of a texture (2D and 3D textures)
        usize_t num_bytes{0u};     // bytes between two depth slices of a texture (3D textures only)
        usize_t level{0u};
    };

}

#include "frameworks/rhi/resources/base/resource_state_tracker.h"
#include "frameworks/rhi/resources/base/resource.h"
#include "frameworks/rhi/resources/base/mem_resource.h"
#include "frameworks/rhi/resources/base/resource_view.h"

#include "frameworks/rhi/resources/semaphore.h"
#include "frameworks/rhi/resources/fence.h"
#include "frameworks/rhi/resources/buffer.h"
#include "frameworks/rhi/resources/buffer_view.h"
#include "frameworks/rhi/resources/sampler.h"
#include "frameworks/rhi/resources/image.h"
#include "frameworks/rhi/resources/image_view.h"
#include "frameworks/rhi/resources/shader_reflection.h"
#include "frameworks/rhi/resources/shader.h"
#include "frameworks/rhi/resources/program.h"