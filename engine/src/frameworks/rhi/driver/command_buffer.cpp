#include "frameworks/rhi/rhi.h"
#include "frameworks/rhi/utils/vulkan_helpers.h"

namespace Be::Framework::RHI
{
    FixedArray<vk::PipelineStageFlags2, ERhiQueueTypeEnum::Count> SUPPORTED_STAGES_MASK = {
        PIPELINE_GRAPHICS_STAGES | PIPELINE_COMPUTE_STAGES | PIPELINE_STAGE_ALL_TRANSFER,
        PIPELINE_COMPUTE_STAGES | PIPELINE_STAGE_ALL_TRANSFER,
        PIPELINE_STAGE_ALL_TRANSFER,
    };

    FixedArray<vk::AccessFlags2, ERhiQueueTypeEnum::Count> SUPPORTED_ACCESS_MASK = {
        PIPELINE_GRAPHICS_ACCESS_MASK | PIPELINE_COMPUTE_ACCESS_MASK | PIPELINE_TRANSFER_ACCESS_MASK,
        PIPELINE_COMPUTE_ACCESS_MASK | PIPELINE_TRANSFER_ACCESS_MASK,
        PIPELINE_TRANSFER_ACCESS_MASK,
    };

    RhiCommandBuffer::RhiCommandBuffer(RhiDriver &driver, vk::CommandBuffer handle, ERhiQueueType type)
        : m_driver{driver},
          m_handle{handle},
          m_queue_type{type},
          m_ended{true}
    {
        PROFILER_SCOPE;

        m_image_barriers.reserve(32);
    }

    RhiCommandBuffer::~RhiCommandBuffer()
    {
        PROFILER_SCOPE;

        End();
        m_internal_state = {};

        if (m_handle)
        {
            m_driver.Delete(m_queue_type, m_handle);
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Begin / End

    void RhiCommandBuffer::Begin()
    {
        PROFILER_SCOPE;

        End();
        m_handle.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

        m_internal_state.program_dirty = false;
        m_internal_state.flags_set_dirty = 0;
        ZeroMem(m_internal_state.bindings);

        vk::CommandBufferBeginInfo begin_info{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        m_handle.begin(begin_info);
        m_ended = false;

        LOG_DEBUG("RhiCommandBuffer::Begin()");
    }

    void RhiCommandBuffer::End()
    {
        PROFILER_SCOPE;

        if (!m_ended)
        {
            FlushBarriers();
            m_handle.end();
            m_ended = true;
            LOG_DEBUG("RhiCommandBuffer::End()");
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Rendering

    void RhiCommandBuffer::BeginRendering(const RenderingBeginDesc &desc)
    {
        PROFILER_SCOPE;

        Array<vk::RenderingAttachmentInfo> attachment_infos;
        attachment_infos.reserve(desc.color_attachments.size());
        uint32_t i{0};
        for (const auto &a : desc.color_attachments)
        {
            attachment_infos.emplace_back(a.attachment_info);
            i++;
        }

        vk::RenderingInfo rendering_info{};
        rendering_info.layerCount = 1;
        rendering_info.renderArea.offset.x = int32_t(desc.area.x);
        rendering_info.renderArea.offset.y = int32_t(desc.area.y);
        rendering_info.renderArea.extent.width = desc.area.width;
        rendering_info.renderArea.extent.height = desc.area.height;
        rendering_info.colorAttachmentCount = uint32_t(attachment_infos.size());
        rendering_info.pColorAttachments = attachment_infos.data();
        rendering_info.pDepthAttachment = (desc.depth_attachment.format == vk::Format::eUndefined ? nullptr : &desc.depth_attachment.attachment_info);
        rendering_info.pStencilAttachment = (desc.stencil_attachment.format == vk::Format::eUndefined ? nullptr : &desc.stencil_attachment.attachment_info);

        FlushBarriers();
        FlushState();

        m_internal_state.is_rendering = true;
        m_handle.beginRendering(rendering_info);

        LOG_DEBUG("RhiCommandBuffer::BeginRendering()");
    }

    void RhiCommandBuffer::EndRendering()
    {
        PROFILER_SCOPE;

        if (m_internal_state.is_rendering)
        {
            m_handle.endRendering();
            m_internal_state.is_rendering = false;
            LOG_DEBUG("RhiCommandBuffer::EndRendering()");
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Pipeline state

    void RhiCommandBuffer::BindProgram(const RhiProgramHandle &prog)
    {
        PROFILER_SCOPE;

        if (m_internal_state.program && (m_internal_state.program->Hash() == prog->Hash()))
        {
            return;
        }
        m_internal_state.program = prog;
        m_internal_state.program_dirty = true;
    }

    void RhiCommandBuffer::SetRasterizerState(const RhiRasterizerState &state)
    {
        PROFILER_SCOPE;

        m_handle.setPolygonModeEXT(state.GetFillMode());
        m_handle.setCullMode(state.GetCullMode());
        m_handle.setFrontFace(state.GetFrontFace());

        m_handle.setDepthBiasEnable(state.GetDepthBias() != 0.0f);
        m_handle.setDepthClampEnableEXT(state.GetDepthBiasClamp() != 0.0f);
        m_handle.setDepthBias(state.GetDepthBias(), state.GetDepthBiasClamp(), 0.0f);
    }

    void RhiCommandBuffer::SetBlendState(const RhiBlendState &state)
    {
        PROFILER_SCOPE;

        m_handle.setLogicOpEnableEXT(state.IsLogicOpEnabled());
        m_handle.setLogicOpEXT(state.GetLogicOp());

        m_handle.setColorBlendEnableEXT(0, MAX_COLOR_ATTACHMENTS, state.IsBlendsEnabled());
        m_handle.setColorBlendEquationEXT(0, MAX_COLOR_ATTACHMENTS, state.GetBlendEquations());
        m_handle.setColorWriteMaskEXT(0, MAX_COLOR_ATTACHMENTS, state.GetColorWriteMasks());
    }

    void RhiCommandBuffer::SetDepthStencilState(const RhiDepthStencilState &state)
    {
        PROFILER_SCOPE;

        m_handle.setDepthTestEnable(state.IsDepthTest());
        m_handle.setDepthWriteEnable(state.IsDepthWrite());
        m_handle.setDepthBoundsTestEnable(state.IsDepthBoundsTest());
        m_handle.setDepthCompareOp(state.GetDepthFunc());

        m_handle.setStencilTestEnable(state.IsStencil());
    }

    void RhiCommandBuffer::SetViewport(float x, float y, float width, float height)
    {
        PROFILER_SCOPE;

        vk::Viewport viewport{};
        viewport.x = x;
        viewport.y = y;
        viewport.width = width;
        viewport.height = height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1.0;

        m_handle.setViewport(0, 1, &viewport);
    }

    void RhiCommandBuffer::SetScissorRect(int32_t left, int32_t top, uint32_t right, uint32_t bottom)
    {
        PROFILER_SCOPE;

        vk::Rect2D rect{};
        rect.offset.x = left;
        rect.offset.y = top;
        rect.extent.width = right;
        rect.extent.height = bottom;

        m_handle.setScissor(0, 1, &rect);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Bind resources

    void RhiCommandBuffer::BindResource(uint8_t set, uint8_t binding, const RhiResourceView &resource_view)
    {
        PROFILER_SCOPE;

        m_internal_state.bindings[set][binding].view = &resource_view;
        m_internal_state.flags_set_dirty |= BIT(set);
    }

    void RhiCommandBuffer::SetIndexBuffer(const RhiBuffer &resource, vk::Format format)
    {
        PROFILER_SCOPE;

        vk::IndexType index_type = ImageFormatToIndexType(format);
        m_handle.bindIndexBuffer(resource.VkHandle(), 0, index_type);
    }

    void RhiCommandBuffer::SetVertexBuffer(uint32_t binding, const RhiBuffer &resource)
    {
        PROFILER_SCOPE;

        vk::Buffer vertex_buffers[] = {resource.VkHandle()};
        vk::DeviceSize offsets[] = {0};
        m_handle.bindVertexBuffers(binding, 1, vertex_buffers, offsets);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Drawing

    void RhiCommandBuffer::Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
    {
        PROFILER_SCOPE;

        m_handle.draw(vertex_count, instance_count, first_vertex, first_instance);
    }

    void RhiCommandBuffer::DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
    {
        PROFILER_SCOPE;

        m_handle.drawIndexed(index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    void RhiCommandBuffer::DrawIndirect(const RhiBuffer &argument_buffer, uint64_t argument_buffer_offset)
    {
        PROFILER_SCOPE;

        DrawIndirectCount(argument_buffer, argument_buffer_offset, nullptr, 0, 1, sizeof(DrawIndirectCommand));
    }

    void RhiCommandBuffer::DrawIndexedIndirect(const RhiBuffer &argument_buffer, uint64_t argument_buffer_offset)
    {
        PROFILER_SCOPE;

        DrawIndexedIndirectCount(argument_buffer, argument_buffer_offset, nullptr, 0, 1, sizeof(DrawIndexedIndirectCommand));
    }

    void RhiCommandBuffer::DrawIndirectCount(const RhiBuffer &argument_buffer,
                                             uint64_t argument_buffer_offset,
                                             const RhiBuffer *count_buffer,
                                             uint64_t count_buffer_offset,
                                             uint32_t max_draw_count,
                                             uint32_t stride)
    {
        PROFILER_SCOPE;

        if (count_buffer)
        {
            m_handle.drawIndirectCount(argument_buffer.VkHandle(),
                                       argument_buffer_offset,
                                       count_buffer->VkHandle(),
                                       count_buffer_offset,
                                       max_draw_count,
                                       stride);
        }
        else
        {
            assert(count_buffer_offset == 0);
            m_handle.drawIndirect(argument_buffer.VkHandle(),
                                  argument_buffer_offset,
                                  max_draw_count,
                                  stride);
        }
    }

    void RhiCommandBuffer::DrawIndexedIndirectCount(const RhiBuffer &argument_buffer,
                                                    uint64_t argument_buffer_offset,
                                                    const RhiBuffer *count_buffer,
                                                    uint64_t count_buffer_offset,
                                                    uint32_t max_draw_count,
                                                    uint32_t stride)
    {
        PROFILER_SCOPE;

        if (count_buffer)
        {
            m_handle.drawIndexedIndirectCount(argument_buffer.VkHandle(),
                                              argument_buffer_offset,
                                              count_buffer->VkHandle(),
                                              count_buffer_offset,
                                              max_draw_count,
                                              stride);
        }
        else
        {
            assert(count_buffer_offset == 0);
            m_handle.drawIndexedIndirect(argument_buffer.VkHandle(),
                                         argument_buffer_offset,
                                         max_draw_count,
                                         stride);
        }
    }

    void RhiCommandBuffer::DrawMeshTask(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
    {
        PROFILER_SCOPE;
        m_handle.drawMeshTasksEXT(group_count_x, group_count_y, group_count_z);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Computing

    void RhiCommandBuffer::Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z)
    {
        PROFILER_SCOPE;

        FlushBarriers();
        FlushState();
        m_handle.dispatch(thread_group_count_x, thread_group_count_y, thread_group_count_z);
    }

    void RhiCommandBuffer::DispatchIndirect(const RhiBuffer &argument_buffer, uint64_t argument_buffer_offset)
    {
        PROFILER_SCOPE;

        FlushBarriers();
        FlushState();
        m_handle.dispatchIndirect(argument_buffer.VkHandle(), argument_buffer_offset);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Barriers

    void RhiCommandBuffer::SetImageLayout(RhiImage &image,
                                          vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                                          vk::PipelineStageFlags2 src_stages, vk::PipelineStageFlags2 dst_stages,
                                          ERhiQueueType src_queue, ERhiQueueType dst_queue,
                                          const vk::ImageSubresourceRange &subres)
    {
        PROFILER_SCOPE;

        // Image layout transitions within a render pass execute
        // dependencies between attachments
        EndRendering();

        VERIFY(src_stages & SUPPORTED_STAGES_MASK[src_queue], "Unsupported src pipeline stage.");
        VERIFY(dst_stages & SUPPORTED_STAGES_MASK[dst_queue], "Unsupported dst pipeline stage.");

        src_stages = (src_queue != m_queue_type ? vk::PipelineStageFlagBits2::eNone : src_stages);
        dst_stages = (dst_queue != m_queue_type ? vk::PipelineStageFlagBits2::eNone : dst_stages);

        if (old_layout == new_layout)
        {
            m_pipeline_barrier.memory_src_stages |= src_stages;
            m_pipeline_barrier.memory_dst_stages |= dst_stages;

            m_pipeline_barrier.memory_src_access |= SrcAccessMaskFromImageLayout(old_layout);
            m_pipeline_barrier.memory_dst_access |= DstAccessMaskFromImageLayout(new_layout);
            return;
        }

        // Check overlapping subresources
        for (const auto &ib : m_image_barriers)
        {
            if (ib.image != image.VkHandle())
            {
                continue;
            }

            const auto &other_range = ib.subresourceRange;

            const auto start_layer0 = subres.baseArrayLayer;
            const auto end_layer0 = subres.layerCount != VK_REMAINING_ARRAY_LAYERS ? (subres.baseArrayLayer + subres.layerCount) : ~0u;
            const auto start_layer1 = other_range.baseArrayLayer;
            const auto end_layer1 = other_range.layerCount != VK_REMAINING_ARRAY_LAYERS ? (other_range.baseArrayLayer + other_range.layerCount) : ~0u;

            const auto start_mip0 = subres.baseMipLevel;
            const auto end_mip0 = subres.levelCount != VK_REMAINING_MIP_LEVELS ? (subres.baseMipLevel + subres.levelCount) : ~0u;
            const auto start_mip1 = other_range.baseMipLevel;
            const auto end_mip1 = other_range.levelCount != VK_REMAINING_MIP_LEVELS ? (other_range.baseMipLevel + other_range.levelCount) : ~0u;

            const auto layers_overlap = Math::RangesOverlap(start_layer0, end_layer0, start_layer1, end_layer1);
            const auto mips_overlap = Math::RangesOverlap(start_mip0, end_mip0, start_mip1, end_mip1);

            // If the range overlaps with any of the existing barriers, we need to
            // flush them.
            if (layers_overlap && mips_overlap)
            {
                FlushBarriers();
                break;
            }
        }

        m_pipeline_barrier.image_src_stages |= src_stages;
        m_pipeline_barrier.image_dst_stages |= dst_stages;

        auto &b = m_image_barriers.emplace_back();

        // stages
        b.srcStageMask = src_stages;
        b.dstStageMask = dst_stages;

        // access
        b.srcAccessMask = (src_queue != m_queue_type ? vk::AccessFlagBits2::eNone : SrcAccessMaskFromImageLayout(old_layout) & SUPPORTED_ACCESS_MASK[src_queue]);
        b.dstAccessMask = (dst_queue != m_queue_type ? vk::AccessFlagBits2::eNone : DstAccessMaskFromImageLayout(new_layout) & SUPPORTED_ACCESS_MASK[dst_queue]);

        // layout
        b.oldLayout = old_layout;
        b.newLayout = new_layout;

        // queue
        b.srcQueueFamilyIndex = m_driver.GetQueue(src_queue).GetQueueFamilyIndex();
        b.dstQueueFamilyIndex = m_driver.GetQueue(dst_queue).GetQueueFamilyIndex();

        b.image = image.VkHandle();
        b.subresourceRange.aspectMask = subres.aspectMask ? subres.aspectMask : vk::ImageAspectFlagBits::eColor;
        b.subresourceRange.baseArrayLayer = subres.baseArrayLayer;
        b.subresourceRange.baseMipLevel = subres.baseMipLevel;
        b.subresourceRange.layerCount = (subres.layerCount == 0 ? VK_REMAINING_ARRAY_LAYERS : subres.layerCount);
        b.subresourceRange.levelCount = (subres.levelCount == 0 ? VK_REMAINING_MIP_LEVELS : subres.levelCount);
    }

    void RhiCommandBuffer::SetImageBarrierImpl(const RhiImageBarrier &barrier)
    {
        PROFILER_SCOPE;

        auto &image = barrier.image;

        auto old_state = barrier.old_state;
        auto new_state = barrier.new_state;

        auto old_queue = (barrier.old_queue == INVALID_GPU_QUEUE ? m_queue_type : barrier.old_queue);
        auto new_queue = (barrier.new_queue == INVALID_GPU_QUEUE ? m_queue_type : barrier.new_queue);

        if (old_state == ERhiResourceState::eUnknown)
        {
            old_state = image.GetStateTracker().GetResourceState();
        }
        VERIFY(old_state != ERhiResourceState::eUnknown, "Image state is unknown.");

        auto old_layout = ResourceStateToImageLayout(old_state);
        auto new_layout = ResourceStateToImageLayout(new_state);

        auto after_write = ResourceStateHasWriteAccess(old_state);

        if (((old_state & new_state) == new_state) &&
            old_layout == new_layout &&
            old_queue == new_queue &&
            !after_write)
        {
            return;
        }

        auto src_stage = ResourceStateToPipelineStages(old_state);
        auto dst_stage = ResourceStateToPipelineStages(new_state);

        // auto src_access = SrcAccessMaskFromImageLayout(old_layout);
        // auto dst_access = DstAccessMaskFromImageLayout(new_layout);

        vk::ImageSubresourceRange subresource{};
        subresource.aspectMask = ImageFormatToImageAspect(image.GetFormat());
        subresource.baseMipLevel = barrier.base_mip_level;
        subresource.levelCount = barrier.mip_levels;
        subresource.baseArrayLayer = barrier.base_array_layer;
        subresource.layerCount = barrier.array_layers;

        SetImageLayout(image, old_layout, new_layout, src_stage, dst_stage, old_queue, new_queue, subresource);
    }

    void RhiCommandBuffer::SetImageBarrier(const RhiImageBarrier &barrier)
    {
        PROFILER_SCOPE;

        auto &image = barrier.image;
        if (!image.VkHandle())
        {
            FATAL("Invalid GPU image.");
            return;
        }

        auto &state_tracker = image.GetStateTracker();

        if (state_tracker.HasResourceState() &&
            barrier.base_mip_level == 0 &&
            (barrier.mip_levels == VK_REMAINING_MIP_LEVELS || barrier.mip_levels == image.GetMipLevels()) &&
            barrier.base_array_layer == 0 &&
            (barrier.array_layers == VK_REMAINING_ARRAY_LAYERS || image.GetArrayLayers()))
        {
            SetImageBarrierImpl(barrier);

            if (barrier.new_queue == INVALID_GPU_QUEUE || barrier.new_queue == m_queue_type)
            {
                state_tracker.SetResourceState(barrier.new_state);
            }
        }
        else
        {
            auto mips = barrier.mip_levels;
            auto arrays = barrier.array_layers;

            if (mips == VK_REMAINING_MIP_LEVELS)
            {
                mips = image.GetMipLevels() - barrier.base_mip_level;
            }

            if (arrays == VK_REMAINING_ARRAY_LAYERS)
            {
                arrays = image.GetArrayLayers() - barrier.base_array_layer;
            }

            for (uint32_t m = 0; m < mips; ++m)
            {
                for (uint32_t a = 0; a < arrays; ++a)
                {
                    RhiImageBarrier b{
                        .image = barrier.image,
                        .base_mip_level = barrier.base_mip_level + m,
                        .mip_levels = 1,
                        .base_array_layer = barrier.base_array_layer + a,
                        .array_layers = 1,
                        .old_state = barrier.old_state == ERhiResourceState::eUnknown
                                         ? state_tracker.GetSubresourceState(b.base_mip_level, b.base_array_layer)
                                         : barrier.old_state,
                        .new_state = barrier.new_state,
                        .new_queue = barrier.new_queue,
                    };

                    SetImageBarrierImpl(b);

                    if (barrier.new_queue == INVALID_GPU_QUEUE || barrier.new_queue == m_queue_type)
                    {
                        state_tracker.SetSubresourceState(b.base_mip_level, b.base_array_layer, b.new_state);
                    }
                }
            }
        }
    }

    void RhiCommandBuffer::SetImageBarriers(const Array<RhiImageBarrier> &barriers)
    {
        PROFILER_SCOPE;

        VERIFY(!m_internal_state.is_rendering, "State transitions are not allowed inside a rendering.");

        for (const auto &b : barriers)
        {
            SetImageBarrier(b);
        }
    }

    void RhiCommandBuffer::SetBufferBarrier(const RhiBufferBarrier &barrier)
    {
        PROFILER_SCOPE;

        VERIFY(!m_internal_state.is_rendering, "State transitions are not allowed inside a rendering.");

        auto &buffer = barrier.buffer;
        if (!buffer.VkHandle())
        {
            assert(false);
            return;
        }

        auto old_state = barrier.old_state;
        auto new_state = barrier.new_state;

        auto old_queue = (barrier.old_queue == INVALID_GPU_QUEUE ? m_queue_type : barrier.old_queue);
        auto new_queue = (barrier.new_queue == INVALID_GPU_QUEUE ? m_queue_type : barrier.new_queue);

        if (old_state == ERhiResourceState::eUnknown)
        {
            old_state = buffer.GetStateTracker().GetResourceState();
        }
        VERIFY(old_state != ERhiResourceState::eUnknown, "Buffer state is unknown.");

        // Always add barrier after writes.
        const auto after_write = ResourceStateHasWriteAccess(old_state);

        if (((old_state & new_state) == new_state) &&
            old_queue == new_queue &&
            !after_write)
        {
            return;
        }

        auto &b = m_buffer_barriers.emplace_back();

        b.srcStageMask = ResourceStateToPipelineStages(old_state);
        b.srcAccessMask = ResourceStateToAccessFlags(old_state);
        b.dstStageMask = ResourceStateToPipelineStages(new_state);
        b.dstAccessMask = ResourceStateToAccessFlags(new_state);
        b.srcQueueFamilyIndex = m_driver.GetQueue(old_queue).GetQueueFamilyIndex();
        b.dstQueueFamilyIndex = m_driver.GetQueue(new_queue).GetQueueFamilyIndex();
        b.buffer = buffer.VkHandle();
        b.offset = barrier.offset;
        b.size = barrier.size;

        if (barrier.new_queue == INVALID_GPU_QUEUE || barrier.new_queue == m_queue_type)
        {
            buffer.GetStateTracker().SetResourceState(new_state);
        }
    }

    void RhiCommandBuffer::SetBufferBarriers(const Array<RhiBufferBarrier> &barriers)
    {
        PROFILER_SCOPE;

        for (const auto &b : barriers)
        {
            SetBufferBarrier(b);
        }
    }

    void RhiCommandBuffer::SetMemoryBarrier(vk::AccessFlags2 src_access_mask,
                                            vk::AccessFlags2 dst_access_mask,
                                            vk::PipelineStageFlags2 src_stages,
                                            vk::PipelineStageFlags2 dst_stages)
    {
        PROFILER_SCOPE;

        EndRendering();

        VERIFY(src_stages & SUPPORTED_STAGES_MASK[m_queue_type], "Unsupported source pipeline stage.");
        VERIFY(dst_stages & SUPPORTED_STAGES_MASK[m_queue_type], "Unsupported dest pipeline stage.");

        m_pipeline_barrier.memory_src_stages |= src_stages;
        m_pipeline_barrier.memory_dst_stages |= dst_stages;

        m_pipeline_barrier.memory_src_access |= src_access_mask;
        m_pipeline_barrier.memory_dst_access |= dst_access_mask;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Resource utils

    void RhiCommandBuffer::CopyBuffer(const RhiBuffer &src_buffer, const RhiBuffer &dst_buffer,
                                      const Array<RhiBufferCopyRegion> &regions)
    {
        PROFILER_SCOPE;

        FlushBarriers();

        Array<vk::BufferCopy> vk_regions;
        for (const auto &region : regions)
        {
            vk_regions.emplace_back(region.src_offset, region.dst_offset, region.num_bytes);
        }
        m_handle.copyBuffer(src_buffer.VkHandle(), dst_buffer.VkHandle(), vk_regions);
    }

    void RhiCommandBuffer::CopyBufferToImage(const RhiBuffer &src_buffer, const RhiImage &dst_image,
                                             const Array<RhiBufferToImageCopyRegion> &regions)
    {
        PROFILER_SCOPE;

        FlushBarriers();

        Array<vk::BufferImageCopy> vk_regions;
        auto format = dst_image.GetFormat();
        const auto &format_info = vkuGetFormatInfo(VkFormat(format));

        for (const auto &region : regions)
        {
            auto &vk_region = vk_regions.emplace_back();

            vk_region.bufferOffset = region.buffer_offset;

            if (vkuFormatIsCompressed(VkFormat(format)))
            {
                const auto &extent = format_info.block_extent;
                vk_region.bufferRowLength = region.buffer_row_pitch / format_info.block_size * extent.width;
                vk_region.bufferImageHeight = ((region.texture_extent.height + extent.height - 1) / extent.height) * extent.width;
            }
            else
            {
                vk_region.bufferRowLength = region.buffer_row_pitch / RhiFormat::BytesPerPixel(format_info);
                vk_region.bufferImageHeight = region.texture_extent.height;
            }

            vk_region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            vk_region.imageSubresource.mipLevel = region.texture_mip_level;
            vk_region.imageSubresource.baseArrayLayer = region.texture_array_layer;
            vk_region.imageSubresource.layerCount = 1;
            vk_region.imageOffset.x = region.texture_offset.x;
            vk_region.imageOffset.y = region.texture_offset.y;
            vk_region.imageOffset.z = region.texture_offset.z;
            vk_region.imageExtent.width = region.texture_extent.width;
            vk_region.imageExtent.height = region.texture_extent.height;
            vk_region.imageExtent.depth = region.texture_extent.depth;
        }

        m_handle.copyBufferToImage(src_buffer.VkHandle(),
                                   dst_image.VkHandle(),
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk_regions);
    }

    void RhiCommandBuffer::CopyImage(const RhiImage &src_image, const RhiImage &dst_image,
                                     const Array<RhiTextureCopyRegion> &regions)
    {
        PROFILER_SCOPE;

        FlushBarriers();

        Array<vk::ImageCopy> vk_regions;

        for (const auto &region : regions)
        {
            auto &vk_region = vk_regions.emplace_back();

            vk_region.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            vk_region.srcSubresource.mipLevel = region.src_mip_level;
            vk_region.srcSubresource.baseArrayLayer = region.src_array_layer;
            vk_region.srcSubresource.layerCount = 1;
            vk_region.srcOffset.x = region.src_offset.x;
            vk_region.srcOffset.y = region.src_offset.y;
            vk_region.srcOffset.z = region.src_offset.z;

            vk_region.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            vk_region.dstSubresource.mipLevel = region.dst_mip_level;
            vk_region.dstSubresource.baseArrayLayer = region.dst_array_layer;
            vk_region.dstSubresource.layerCount = 1;
            vk_region.dstOffset.x = region.dst_offset.x;
            vk_region.dstOffset.y = region.dst_offset.y;
            vk_region.dstOffset.z = region.dst_offset.z;

            vk_region.extent.width = region.extent.width;
            vk_region.extent.height = region.extent.height;
            vk_region.extent.depth = region.extent.depth;
        }
        m_handle.copyImage(src_image.VkHandle(), vk::ImageLayout::eTransferSrcOptimal,
                           dst_image.VkHandle(), vk::ImageLayout::eTransferDstOptimal,
                           vk_regions);
    }

    void RhiCommandBuffer::BlitImage(const RhiImage &src_image,
                                     const RhiImage &dst_image,
                                     vk::Filter filter,
                                     const Array<vk::ImageBlit2> &regions)
    {
        PROFILER_SCOPE;

        FlushBarriers();

        uint32_t rs{0};
        const vk::ImageBlit2 *r{nullptr};
        vk::ImageBlit2 blit{};

        if (regions.empty())
        {
            rs = 1;
            r = &blit;

            blit.srcSubresource.aspectMask = ImageFormatToImageAspect(src_image.GetFormat());
            blit.srcSubresource.mipLevel = 0;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = src_image.GetArrayLayers();
            blit.srcOffsets[1].x = src_image.GetWidth();
            blit.srcOffsets[1].y = src_image.GetHeight();
            blit.srcOffsets[1].z = 1;

            blit.dstSubresource.aspectMask = ImageFormatToImageAspect(dst_image.GetFormat());
            blit.dstSubresource.mipLevel = 0;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = dst_image.GetArrayLayers();
            blit.dstOffsets[1].x = dst_image.GetWidth();
            blit.dstOffsets[1].y = dst_image.GetHeight();
            blit.dstOffsets[1].z = 1;
        }
        else
        {
            rs = uint32_t(regions.size());
            r = regions.data();
        }

        vk::BlitImageInfo2 info{};
        info.srcImage = src_image.VkHandle();
        info.srcImageLayout = (src_image.VkHandle() == dst_image.VkHandle() ? vk::ImageLayout::eTransferSrcOptimal : src_image.GetLayout());
        info.dstImage = dst_image.VkHandle();
        info.dstImageLayout = (src_image.VkHandle() == dst_image.VkHandle() ? vk::ImageLayout::eTransferDstOptimal : dst_image.GetLayout());
        info.regionCount = rs;
        info.pRegions = r;
        info.filter = filter;

        m_handle.blitImage2(info);
    }

    void RhiCommandBuffer::UpdateSubresource(const RhiImage &image,
                                             uint32_t level,
                                             const void *data, uint64_t row_pitch, uint64_t depth_pitch,
                                             RhiBuffer *upload_buffer,
                                             uint64_t buffer_offset)
    {
        PROFILER_SCOPE;

        Array<RhiBufferToImageCopyRegion> regions;

        auto &region = regions.emplace_back();

        region.buffer_offset = buffer_offset;
        region.texture_mip_level = level % image.GetMipLevels();
        region.texture_array_layer = level / image.GetMipLevels();
        region.texture_extent.width = std::max<uint32_t>(1, image.GetWidth() >> region.texture_mip_level);
        region.texture_extent.height = std::max<uint32_t>(1, image.GetHeight() >> region.texture_mip_level);
        region.texture_extent.depth = 1;

        uint64_t num_bytes{0};
        uint64_t row_bytes{0};
        uint32_t num_rows{0};

        RhiFormat::GetSizes(image.GetFormat(),
                            region.texture_extent.width, region.texture_extent.height,
                            num_bytes, row_bytes, num_rows, 1);

        region.buffer_row_pitch = uint32_t(row_bytes);

        RhiBufferHandle tmp_buffer{};
        if (!upload_buffer)
        {
            RhiBufferDesc desc{
                .bind_flag = ERhiBindFlag::eCopySource,
                .size = uint32_t(num_bytes),
                .mem_usage = ERhiResourceUsage::eUpload,
            };
            tmp_buffer = m_driver.CreateBuffer(desc);
            upload_buffer = tmp_buffer;
        }

        upload_buffer->UpdateWithTextureData(buffer_offset, row_bytes, num_bytes,
                                             data, row_pitch, depth_pitch,
                                             num_rows, region.texture_extent.depth);

        CopyBufferToImage(*upload_buffer, image, regions);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Flushing

    void RhiCommandBuffer::FlushBarriers()
    {
        PROFILER_SCOPE;

        if (!m_pipeline_barrier.memory_src_stages &&
            !m_pipeline_barrier.memory_dst_stages &&
            m_image_barriers.empty() &&
            m_buffer_barriers.empty())
        {
            return;
        }

        EndRendering();

        const bool has_mem_barrier =
            m_pipeline_barrier.memory_src_stages && m_pipeline_barrier.memory_dst_stages &&
            m_pipeline_barrier.memory_src_access && m_pipeline_barrier.memory_dst_access;

        vk::MemoryBarrier2 mem_barrier{};
        if (has_mem_barrier)
        {
            mem_barrier.srcAccessMask = m_pipeline_barrier.memory_src_access & SUPPORTED_ACCESS_MASK[m_queue_type];
            mem_barrier.dstAccessMask = m_pipeline_barrier.memory_dst_access & SUPPORTED_ACCESS_MASK[m_queue_type];
        }

        // const vk::PipelineStageFlags2 src_stages = (m_pipeline_barrier.image_src_stages | m_pipeline_barrier.memory_src_stages) & SUPPORTED_STAGES_MASK[m_queue_type];
        // const vk::PipelineStageFlags2 dst_stages = (m_pipeline_barrier.image_dst_stages | m_pipeline_barrier.memory_dst_stages) & SUPPORTED_STAGES_MASK[m_queue_type];
        // VERIFY(src_stages && dst_stages, "Invalid pipeline stages.");

        vk::DependencyInfo dep_info{
            .dependencyFlags = vk::DependencyFlagBits::eByRegion,
            .memoryBarrierCount = has_mem_barrier ? 1u : 0u,
            .pMemoryBarriers = has_mem_barrier ? &mem_barrier : nullptr,
            .bufferMemoryBarrierCount = uint32_t(m_buffer_barriers.size()),
            .pBufferMemoryBarriers = m_buffer_barriers.empty() ? nullptr : m_buffer_barriers.data(),
            .imageMemoryBarrierCount = uint32_t(m_image_barriers.size()),
            .pImageMemoryBarriers = m_image_barriers.empty() ? nullptr : m_image_barriers.data(),
        };
        m_handle.pipelineBarrier2(dep_info);

        m_image_barriers.clear();
        m_buffer_barriers.clear();

        m_pipeline_barrier.image_src_stages.m_mask = 0u;
        m_pipeline_barrier.image_dst_stages.m_mask = 0u;
        m_pipeline_barrier.memory_src_stages.m_mask = 0u;
        m_pipeline_barrier.memory_dst_stages.m_mask = 0u;
        m_pipeline_barrier.memory_src_access.m_mask = 0u;
        m_pipeline_barrier.memory_dst_access.m_mask = 0u;
    }

    void RhiCommandBuffer::FlushState()
    {
        PROFILER_SCOPE;

        if (!m_internal_state.program)
        {
            return;
        }

        if (m_internal_state.program_dirty)
        {
            const auto &descriptor_sets = m_internal_state.program->GetDescriptorSets();
            if (descriptor_sets.empty())
            {
                m_handle.bindDescriptorSets(m_internal_state.program->GetPipelineBindPoint(),
                                            m_internal_state.program->GetPipelineLayout(),
                                            0, uint32_t(descriptor_sets.size()), descriptor_sets.data(),
                                            0, nullptr);
            }
            m_handle.bindShadersEXT(static_cast<uint32_t>(RhiProgram::SHADER_STAGES.size()),
                                    RhiProgram::SHADER_STAGES.data(),
                                    m_internal_state.program->GetShaders().data());

            m_internal_state.program_dirty = false;
        }

        if (m_internal_state.flags_set_dirty != 0)
        {
            Array<RhiWriteBindingDesc> writes;

            ForEachBit(m_internal_state.flags_set_dirty,
                       [this, &writes](uint32_t set)
                       {
                           FlushDescriptorSet(set, writes);
                       });

            m_internal_state.program->WriteBindings(writes);
            m_internal_state.flags_set_dirty = 0;
        }
    }

    void RhiCommandBuffer::FlushDescriptorSet(uint32_t set, Array<RhiWriteBindingDesc> &writes)
    {
        PROFILER_SCOPE;

        const auto set_mask = m_internal_state.program->GetBindingMask(set);
        const auto &bindings = m_internal_state.bindings[set];

        ForEachBit(set_mask,
                   [this, set, bindings, &writes](uint32_t binding)
                   {
                        const auto& b = m_internal_state.program->GetBinding(set, binding);
                        auto &w = writes.emplace_back();
                        w.view = bindings[binding].view;
                        w.binding = b; });
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Debugging

    void RhiCommandBuffer::BeginEvent(const String &name)
    {
        PROFILER_SCOPE;

        if (RhiInstance::IsDebugUtilsSupported())
        {
            vk::DebugUtilsLabelEXT label = {};
            label.pLabelName = name.c_str();
            m_handle.beginDebugUtilsLabelEXT(&label);
        }
    }

    void RhiCommandBuffer::EndEvent()
    {
        PROFILER_SCOPE;
        
        if (RhiInstance::IsDebugUtilsSupported())
        {
            m_handle.endDebugUtilsLabelEXT();
        }
    }
}
