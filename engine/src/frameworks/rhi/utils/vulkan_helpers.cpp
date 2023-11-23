#include "frameworks/rhi/rhi.h"
#include "frameworks/rhi/utils/vulkan_helpers.h"

namespace Be::Framework::RHI
{

    vk::IndexType ImageFormatToIndexType(vk::Format format) noexcept
    {
        switch (format)
        {
        case vk::Format::eR16Uint:
            return vk::IndexType::eUint16;
        case vk::Format::eR32Uint:
            return vk::IndexType::eUint32;
        default:
            assert(false);
            return {};
        }
    }

    vk::ImageAspectFlags ImageFormatToImageAspect(vk::Format format) noexcept
    {
        switch (format)
        {
        case vk::Format::eD32SfloatS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD16UnormS8Uint:
            return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
        case vk::Format::eD16Unorm:
        case vk::Format::eD32Sfloat:
        case vk::Format::eX8D24UnormPack32:
            return vk::ImageAspectFlagBits::eDepth;
        case vk::Format::eS8Uint:
            return vk::ImageAspectFlagBits::eStencil;
        default:
            return vk::ImageAspectFlagBits::eColor;
        }
    }

    vk::BufferUsageFlags ResourceBindToBufferUsage(ERhiBindFlagBits bind_flags) noexcept
    {
        vk::BufferUsageFlags usage{0};

        if (bind_flags & ERhiBindFlag::eVertexBuffer)
        {
            usage |= vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        }
        if (bind_flags & ERhiBindFlag::eIndexBuffer)
        {
            usage |= vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        }
        if (bind_flags & ERhiBindFlag::eUniformBuffer)
        {
            usage |= vk::BufferUsageFlagBits::eUniformBuffer;
        }
        if (bind_flags & ERhiBindFlag::eUnorderedAccess)
        {
            usage |= vk::BufferUsageFlagBits::eStorageBuffer;
            usage |= vk::BufferUsageFlagBits::eStorageTexelBuffer;
        }
        if (bind_flags & ERhiBindFlag::eShaderResource)
        {
            usage |= vk::BufferUsageFlagBits::eStorageBuffer;
            usage |= vk::BufferUsageFlagBits::eUniformTexelBuffer;
        }
        if (bind_flags & ERhiBindFlag::eAccelerationStructure)
        {
            usage |= vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR;
        }
        if (bind_flags & ERhiBindFlag::eCopySource)
        {
            usage |= vk::BufferUsageFlagBits::eTransferSrc;
        }
        if (bind_flags & ERhiBindFlag::eCopyDest)
        {
            usage |= vk::BufferUsageFlagBits::eTransferDst;
        }
        if (bind_flags & ERhiBindFlag::eShaderTable)
        {
            usage |= vk::BufferUsageFlagBits::eShaderBindingTableKHR;
        }
        if (bind_flags & ERhiBindFlag::eIndirectBuffer)
        {
            usage |= vk::BufferUsageFlagBits::eIndirectBuffer;
        }

        return usage;
    }

    vk::ImageUsageFlags ResourceBindToImageUsage(ERhiBindFlagBits bind_flags) noexcept
    {
        vk::ImageUsageFlags usage{0};
        if (bind_flags & ERhiBindFlag::eDepthStencil)
        {
            usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        }
        if (bind_flags & ERhiBindFlag::eShaderResource)
        {
            usage |= vk::ImageUsageFlagBits::eSampled;
        }
        if (bind_flags & ERhiBindFlag::eRenderTarget)
        {
            usage |= vk::ImageUsageFlagBits::eColorAttachment;
        }
        if (bind_flags & ERhiBindFlag::eUnorderedAccess)
        {
            usage |= vk::ImageUsageFlagBits::eStorage;
        }
        if (bind_flags & ERhiBindFlag::eCopyDest)
        {
            usage |= vk::ImageUsageFlagBits::eTransferDst;
        }
        if (bind_flags & ERhiBindFlag::eCopySource)
        {
            usage |= vk::ImageUsageFlagBits::eTransferSrc;
        }
        if (bind_flags & ERhiBindFlag::eShadingRateSource)
        {
            usage |= vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR;
        }

        return usage;
    }

    vk::ImageLayout ResourceStateToImageLayout(ERhiResourceState state, bool inside_render_pass, bool density_instead_shading_rate) noexcept
    {
        if (state == ERhiResourceState::eUnknown)
        {
            return vk::ImageLayout::eUndefined;
        }

        switch (state)
        {
            // clang-format off
        case ERhiResourceState::eUndefined:
            return vk::ImageLayout::eUndefined;

        case ERhiResourceState::eRenderTarget:
            return vk::ImageLayout::eColorAttachmentOptimal;

        case ERhiResourceState::eUnorderedAccess:
            return vk::ImageLayout::eGeneral;

        case ERhiResourceState::eDepthTarget:
            return vk::ImageLayout::eDepthStencilAttachmentOptimal;

        case ERhiResourceState::eDepthRead:
            return vk::ImageLayout::eDepthStencilReadOnlyOptimal;

        case ERhiResourceState::eShaderResource:
            return vk::ImageLayout::eShaderReadOnlyOptimal;

        case ERhiResourceState::eCopyDest:
            return vk::ImageLayout::eTransferDstOptimal;

        case ERhiResourceState::eCopySource:
            return vk::ImageLayout::eTransferSrcOptimal;

        case ERhiResourceState::eResolveDest:
            return inside_render_pass
                ? vk::ImageLayout::eColorAttachmentOptimal
                : vk::ImageLayout::eTransferDstOptimal;

        case ERhiResourceState::eResolveSource:
            return vk::ImageLayout::eTransferSrcOptimal;

        case ERhiResourceState::eInputAttachment:
            return vk::ImageLayout::eShaderReadOnlyOptimal;

        case ERhiResourceState::ePresent:
            return vk::ImageLayout::ePresentSrcKHR;

        case ERhiResourceState::eCommon:
            return vk::ImageLayout::eGeneral;

        case ERhiResourceState::eShadingRate:
            return density_instead_shading_rate
                ? vk::ImageLayout::eFragmentDensityMapOptimalEXT
                : vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR;

        default:
            FATAL("Invalid resource state");
            return vk::ImageLayout::eUndefined;
        }
    }

    vk::PipelineStageFlags2 ResourceStateToPipelineStage(ERhiResourceState state) noexcept
    {
        switch (state)
        {
        case ERhiResourceState::eUndefined:
            return vk::PipelineStageFlagBits2::eTopOfPipe;

        case ERhiResourceState::eUniformBuffer:
            return PIPELINE_STAGE_ALL_SHADERS;

        case ERhiResourceState::eVertexBuffer:
            return vk::PipelineStageFlagBits2::eVertexInput;
        
        case ERhiResourceState::eIndexBuffer:
            return vk::PipelineStageFlagBits2::eVertexInput;

        case ERhiResourceState::eRenderTarget:
            return vk::PipelineStageFlagBits2::eColorAttachmentOutput;

        case ERhiResourceState::eUnorderedAccess:
            return PIPELINE_STAGE_ALL_SHADERS;

        case ERhiResourceState::eDepthTarget:
            return vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;

        case ERhiResourceState::eDepthRead:
            return vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;

        case ERhiResourceState::eShaderResource:
            return PIPELINE_STAGE_ALL_SHADERS;

        case ERhiResourceState::eStreamOut:
            return {};

        case ERhiResourceState::eIndirectArgument:
            return vk::PipelineStageFlagBits2::eDrawIndirect;

        case ERhiResourceState::eCopyDest:
            return vk::PipelineStageFlagBits2::eTransfer;

        case ERhiResourceState::eCopySource:
            return vk::PipelineStageFlagBits2::eTransfer;

        case ERhiResourceState::eResolveDest:
            return vk::PipelineStageFlagBits2::eTransfer;

        case ERhiResourceState::eResolveSource:
            return vk::PipelineStageFlagBits2::eTransfer;

        case ERhiResourceState::eInputAttachment:
            return vk::PipelineStageFlagBits2::eFragmentShader;

        case ERhiResourceState::ePresent:
            return vk::PipelineStageFlagBits2::eBottomOfPipe;

        case ERhiResourceState::eCommon:
            return vk::PipelineStageFlagBits2::eAllCommands; // resource may be used in multiple states

        case ERhiResourceState::eShadingRate:
            return vk::PipelineStageFlagBits2::eFragmentDensityProcessEXT |
                vk::PipelineStageFlagBits2::eFragmentShadingRateAttachmentKHR;

        default:
            FATAL("Invalid resource state.");
            return {};
        }
    }

    vk::PipelineStageFlags2 ResourceStateToPipelineStages(ERhiResourceState state) noexcept
    {
        vk::PipelineStageFlags2 stages{ 0U };
        while (state != ERhiResourceState::eUnknown)
        {
            auto state_bit = ExtractAndClearBit(state);
            stages |= ResourceStateToPipelineStage(state_bit);
        }
        return stages;
    }

    vk::AccessFlags2 ResourceStateToAccessFlag(ERhiResourceState state) noexcept
    {
        switch (state)
        {
        case ERhiResourceState::eUndefined:
            return {};

        case ERhiResourceState::eVertexBuffer:
            return vk::AccessFlagBits2::eVertexAttributeRead;

        case ERhiResourceState::eUniformBuffer:
            return vk::AccessFlagBits2::eUniformRead;

        case ERhiResourceState::eIndexBuffer:
            return vk::AccessFlagBits2::eIndexRead;

        case ERhiResourceState::eRenderTarget:
            return vk::AccessFlagBits2::eColorAttachmentRead |
                vk::AccessFlagBits2::eColorAttachmentWrite;

        case ERhiResourceState::eUnorderedAccess:
            return vk::AccessFlagBits2::eShaderWrite |
                vk::AccessFlagBits2::eShaderRead;

        case ERhiResourceState::eDepthTarget:
            return vk::AccessFlagBits2::eDepthStencilAttachmentRead |
                vk::AccessFlagBits2::eDepthStencilAttachmentWrite;

        case ERhiResourceState::eDepthRead:
            return vk::AccessFlagBits2::eDepthStencilAttachmentRead;

        case ERhiResourceState::eShaderResource:
            return vk::AccessFlagBits2::eShaderRead;

        case ERhiResourceState::eStreamOut:
            return vk::AccessFlagBits2::eTransformFeedbackCounterWriteEXT;

        case ERhiResourceState::eIndirectArgument:
            return vk::AccessFlagBits2::eIndirectCommandRead;

        case ERhiResourceState::eCopyDest:
            return vk::AccessFlagBits2::eTransferWrite;

        case ERhiResourceState::eCopySource:
            return vk::AccessFlagBits2::eTransferRead;

        case ERhiResourceState::eResolveDest:
            return vk::AccessFlagBits2::eTransferWrite;

        case ERhiResourceState::eResolveSource:
            return vk::AccessFlagBits2::eTransferRead;

        case ERhiResourceState::eInputAttachment:
            return vk::AccessFlagBits2::eInputAttachmentRead;

        case ERhiResourceState::ePresent:
            return {};

        case ERhiResourceState::eCommon:
            // eCommon state must be used for queue to queue transition, queue to queue synchronization via semaphore creates a memory dependency
            return {};

        case ERhiResourceState::eShadingRate:
            return vk::AccessFlagBits2::eFragmentDensityMapReadEXT |
                vk::AccessFlagBits2::eFragmentShadingRateAttachmentReadKHR;

        default:
            FATAL("Unexpected resource state flag");
            return {};
        }
    }

    vk::AccessFlags2 ResourceStateToAccessFlags(ERhiResourceState state) noexcept
    {
        vk::AccessFlags2 access_flags{ 0u };
        while (state != ERhiResourceState::eUnknown)
        {
            auto state_bit = ExtractAndClearBit(state);
            access_flags |= ResourceStateToAccessFlag(state_bit);
        }
        return access_flags;
    }

    bool ResourceStateHasWriteAccess(ERhiResourceState state) noexcept
    {
        static ERhiResourceState WriteAccessStates =
            ERhiResourceState::eRenderTarget |
            ERhiResourceState::eUnorderedAccess |
            ERhiResourceState::eCopyDest |
            ERhiResourceState::eResolveDest;

        return state & WriteAccessStates;
    }

    static vk::AccessFlags2 AccessMaskFromImageLayout(vk::ImageLayout layout, bool is_dst) noexcept
    {
        vk::AccessFlags2 access_mask{ 0U };

        switch (layout)
        {
        case vk::ImageLayout::eUndefined:
            if (is_dst)
            {
                FATAL("The new layout used in a transition must not be VK_IMAGE_LAYOUT_UNDEFINED. "
                    "This layout must only be used as the initialLayout member of VkImageCreateInfo "
                    "or VkAttachmentDescription, or as the oldLayout in an image transition. (11.4)");
            }
            break;

            // supports all types of device access
        case vk::ImageLayout::eGeneral:
            // VK_IMAGE_LAYOUT_GENERAL must be used for image load/store operations (13.1.1, 13.2.4)
            access_mask = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite;
            break;

            // must only be used as a color or resolve attachment in a VkFramebuffer (11.4)
        case vk::ImageLayout::eColorAttachmentOptimal:
            access_mask = vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
            break;

            // must only be used as a depth/stencil attachment in a VkFramebuffer (11.4)
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            access_mask = vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
            break;

            // must only be used as a read-only depth/stencil attachment in a VkFramebuffer 
            // and/or as a read-only image in a shader (11.4)
        case vk::ImageLayout::eDepthReadOnlyOptimal:
            access_mask = vk::AccessFlagBits2::eDepthStencilAttachmentRead;
            break;

            // must only be used as a read-only image in a shader (which can be read as a sampled image,
            // combined image/sampler and/or input attachment) (11.4)
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            access_mask = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eInputAttachmentRead;
            break;

            //  must only be used as a source image of a transfer command (11.4)
        case vk::ImageLayout::eTransferSrcOptimal:
            access_mask = vk::AccessFlagBits2::eTransferRead;
            break;

            // must only be used as a destination image of a transfer command (11.4)
        case vk::ImageLayout::eTransferDstOptimal:
            access_mask = vk::AccessFlagBits2::eTransferWrite;
            break;

            // does not support device access. This layout must only be used as the initialLayout member
            // of VkImageCreateInfo or VkAttachmentDescription, or as the oldLayout in an image transition.
            // When transitioning out of this layout, the contents of the memory are preserved. (11.4)
        case vk::ImageLayout::ePreinitialized:
            if (!is_dst)
            {
                access_mask = vk::AccessFlagBits2::eHostWrite;
            }
            else
            {
                FATAL("The new layout used in a transition must not be VK_IMAGE_LAYOUT_PREINITIALIZED. "
                    "This layout must only be used as the initialLayout member of VkImageCreateInfo "
                    "or VkAttachmentDescription, or as the oldLayout in an image transition. (11.4)");
            }
            break;

        case vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal:
            access_mask = vk::AccessFlagBits2::eDepthStencilAttachmentRead;
            break;

        case vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal:
            access_mask = vk::AccessFlagBits2::eDepthStencilAttachmentRead;
            break;

        case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
            access_mask = vk::AccessFlagBits2::eDepthStencilAttachmentRead;
            break;

            // When transitioning the image to VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR or VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            // there is no need to delay subsequent processing, or perform any visibility operations (as vkQueuePresentKHR
            // performs automatic visibility operations). To achieve this, the dst_access_mask member of the VkImageMemoryBarrier
            // should be set to 0, and the dstStageMask parameter should be set to VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT.
        case vk::ImageLayout::ePresentSrcKHR:
            break;

        case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
            access_mask = vk::AccessFlagBits2::eFragmentShadingRateAttachmentReadKHR;
            break;

        case vk::ImageLayout::eFragmentDensityMapOptimalEXT:
            access_mask = vk::AccessFlagBits2::eFragmentDensityMapReadEXT;
            break;

        default:
            FATAL("Invalid image layout");
            break;
        }

        return access_mask;
    }

    vk::AccessFlags2 DstAccessMaskFromImageLayout(vk::ImageLayout layout) noexcept
    {
        return AccessMaskFromImageLayout(layout, true);
    }

    vk::AccessFlags2 SrcAccessMaskFromImageLayout(vk::ImageLayout layout) noexcept
    {
        return AccessMaskFromImageLayout(layout, false);
    }

}
