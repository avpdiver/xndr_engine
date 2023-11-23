#pragma once

namespace Be::Framework::RHI
{

    constexpr auto PIPELINE_GRAPHICS_STAGES =
        vk::PipelineStageFlagBits2::eVertexShader |
        vk::PipelineStageFlagBits2::eFragmentShader |
        vk::PipelineStageFlagBits2::eVertexInput |
        vk::PipelineStageFlagBits2::eEarlyFragmentTests |
        vk::PipelineStageFlagBits2::eLateFragmentTests |
        vk::PipelineStageFlagBits2::eColorAttachmentOutput |
        vk::PipelineStageFlagBits2::eAllGraphics;

    constexpr auto PIPELINE_COMPUTE_STAGES =
        vk::PipelineStageFlagBits2::eDrawIndirect |
        vk::PipelineStageFlagBits2::eComputeShader;

    constexpr auto PIPELINE_STAGE_ALL_SHADERS =
        vk::PipelineStageFlagBits2::eVertexShader |
        vk::PipelineStageFlagBits2::eTessellationControlShader |
        vk::PipelineStageFlagBits2::eTessellationEvaluationShader |
        vk::PipelineStageFlagBits2::eGeometryShader |
        vk::PipelineStageFlagBits2::eFragmentShader |
        vk::PipelineStageFlagBits2::eComputeShader |
        vk::PipelineStageFlagBits2::eTaskShaderEXT |
        vk::PipelineStageFlagBits2::eMeshShaderEXT;
        // vk::PipelineStageFlagBits2::eRayTracingShaderKHR |
        
    constexpr auto PIPELINE_STAGE_ALL_TRANSFER =
        vk::PipelineStageFlagBits2::eTopOfPipe |
        vk::PipelineStageFlagBits2::eTransfer |
        vk::PipelineStageFlagBits2::eBottomOfPipe |
        vk::PipelineStageFlagBits2::eHost |
        vk::PipelineStageFlagBits2::eAllCommands;

    constexpr auto PIPELINE_GRAPHICS_ACCESS_MASK =
        vk::AccessFlagBits2::eIndexRead |
        vk::AccessFlagBits2::eVertexAttributeRead |
        vk::AccessFlagBits2::eInputAttachmentRead |
        vk::AccessFlagBits2::eColorAttachmentRead |
        vk::AccessFlagBits2::eColorAttachmentWrite |
        vk::AccessFlagBits2::eDepthStencilAttachmentRead |
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite;

    constexpr auto PIPELINE_COMPUTE_ACCESS_MASK =
        vk::AccessFlagBits2::eIndirectCommandRead |
        vk::AccessFlagBits2::eUniformRead |
        vk::AccessFlagBits2::eShaderRead |
        vk::AccessFlagBits2::eShaderWrite;

    constexpr auto PIPELINE_TRANSFER_ACCESS_MASK =
        vk::AccessFlagBits2::eTransferRead |
        vk::AccessFlagBits2::eTransferWrite |
        vk::AccessFlagBits2::eHostRead |
        vk::AccessFlagBits2::eHostWrite;

    [[nodiscard]] vk::IndexType ImageFormatToIndexType(vk::Format format) noexcept;

    [[nodiscard]] vk::ImageAspectFlags ImageFormatToImageAspect(vk::Format format) noexcept;

    [[nodiscard]] vk::BufferUsageFlags ResourceBindToBufferUsage(ERhiBindFlagBits bind_flags) noexcept;

    [[nodiscard]] vk::ImageUsageFlags ResourceBindToImageUsage(ERhiBindFlagBits bind_flags) noexcept;

    [[nodiscard]] vk::ImageLayout ResourceStateToImageLayout(ERhiResourceState state, bool inside_render_pass = false, bool density_instead_shading_rate = false) noexcept;

    [[nodiscard]] vk::PipelineStageFlags2 ResourceStateToPipelineStage(ERhiResourceState state) noexcept;

    [[nodiscard]] vk::PipelineStageFlags2 ResourceStateToPipelineStages(ERhiResourceState state) noexcept;

    [[nodiscard]] vk::AccessFlags2 ResourceStateToAccessFlag(ERhiResourceState state) noexcept;

    [[nodiscard]] vk::AccessFlags2 ResourceStateToAccessFlags(ERhiResourceState state) noexcept;

    [[nodiscard]] bool ResourceStateHasWriteAccess(ERhiResourceState state) noexcept;

    [[nodiscard]] vk::AccessFlags2 DstAccessMaskFromImageLayout(vk::ImageLayout layout) noexcept;

    [[nodiscard]] vk::AccessFlags2 SrcAccessMaskFromImageLayout(vk::ImageLayout layout) noexcept;

    [[nodiscard]] forceinline bool IsSurfaceTransformSwapsXY(vk::SurfaceTransformFlagBitsKHR transform) noexcept
    {
        return uint32_t(transform & (vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90 |
                                     vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270 |
                                     vk::SurfaceTransformFlagBitsKHR::eRotate90 |
                                     vk::SurfaceTransformFlagBitsKHR::eRotate270)) != 0;
    }

}
