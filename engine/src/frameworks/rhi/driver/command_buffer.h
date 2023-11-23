#pragma once

namespace Be::Framework::RHI
{

    struct RhiBufferBarrier
    {
        RhiBuffer &buffer;

        uint64_t offset{0};
        uint64_t size{VK_WHOLE_SIZE};

        ERhiResourceState old_state{ERhiResourceState::eUnknown};
        ERhiResourceState new_state{ERhiResourceState::eUnknown};

        ERhiQueueType old_queue{INVALID_GPU_QUEUE};
        ERhiQueueType new_queue{INVALID_GPU_QUEUE};
    };

    struct RhiImageBarrier
    {
        RhiImage &image;

        uint32_t base_mip_level{0};
        uint32_t mip_levels{VK_REMAINING_MIP_LEVELS};
        uint32_t base_array_layer{0};
        uint32_t array_layers{VK_REMAINING_ARRAY_LAYERS};

        ERhiResourceState old_state{ERhiResourceState::eUnknown};
        ERhiResourceState new_state{ERhiResourceState::eUnknown};

        ERhiQueueType old_queue{INVALID_GPU_QUEUE};
        ERhiQueueType new_queue{INVALID_GPU_QUEUE};
    };

    struct TextureOffset
    {
        int32_t x;
        int32_t y;
        int32_t z;
    };

    struct TextureExtent2D
    {
        uint32_t width;
        uint32_t height;
    };

    struct TextureExtent3D
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
    };

    struct RhiBufferToImageCopyRegion
    {
        uint64_t buffer_offset;
        uint32_t buffer_row_pitch;
        uint32_t texture_mip_level;
        uint32_t texture_array_layer;
        TextureOffset texture_offset;
        TextureExtent3D texture_extent;
    };

    struct RhiTextureCopyRegion
    {
        TextureExtent3D extent;
        uint32_t src_mip_level;
        uint32_t src_array_layer;

        TextureOffset src_offset;
        uint32_t dst_mip_level;
        uint32_t dst_array_layer;

        TextureOffset dst_offset;
    };

    struct RhiBufferCopyRegion
    {
        uint64_t src_offset{0};
        uint64_t dst_offset{0};
        uint64_t num_bytes{0};
    };

    struct DrawIndirectCommand
    {
        uint32_t vertex_count;
        uint32_t instance_count;
        uint32_t first_vertex;
        uint32_t first_instance;
    };

    struct DrawIndexedIndirectCommand
    {
        uint32_t index_count;
        uint32_t instance_count;
        uint32_t first_index;
        int32_t vertex_offset;
        uint32_t first_instance;
    };

    struct DispatchIndirectCommand
    {
        uint32_t thread_group_count_x;
        uint32_t thread_group_count_y;
        uint32_t thread_group_count_z;
    };

    struct RenderingAttachmentDesc
    {
        vk::Format format{vk::Format::eUndefined};
        vk::RenderingAttachmentInfo attachment_info{};
    };

    struct RenderingBeginDesc
    {
        RectUint32 area{};
        Array<RenderingAttachmentDesc> color_attachments;
        RenderingAttachmentDesc depth_attachment;
        RenderingAttachmentDesc stencil_attachment;
    };

    class RhiCommandBuffer final : public RefCounter, public Noncopyable
    {
    public:
        RhiCommandBuffer(RhiDriver &driver, vk::CommandBuffer handle, ERhiQueueType type);

    public:
        ~RhiCommandBuffer();

    public:
        [[nodiscard]] forceinline vk::CommandBuffer VkHandle() const
        {
            return m_handle;
        }

    public:
        [[nodiscard]] forceinline ERhiQueueType GetQueueType() const
        {
            return m_queue_type;
        }

        // Command list begin/end
    public:
        void Begin();
        void End();

        // Rendering
    public:
        void BeginRendering(const RenderingBeginDesc &desc);
        void EndRendering();

        // Pipeline state
    public:
        void BindProgram(const RhiProgramHandle &prog);

    public:
        void SetInputAssemblyState(const RhiInputAssemblyState &state);
        void SetRasterizerState(const RhiRasterizerState &state);
        void SetBlendState(const RhiBlendState &state);
        void SetDepthStencilState(const RhiDepthStencilState &state);
        void SetViewport(float x, float y, float width, float height);
        void SetScissorRect(int32_t left, int32_t top, uint32_t right, uint32_t bottom);

        // Bind resources
    public:
        void BindResource(uint8_t set, uint8_t binding, const RhiResourceView &resource_view);

    public:
        void SetIndexBuffer(const RhiBuffer &resource, vk::Format format = vk::Format::eR32Uint);
        void SetVertexBuffer(uint32_t binding, const RhiBuffer &resource);

        // Drawing
    public:
        void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
        void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);
        void DrawIndirect(const RhiBuffer &argument_buffer, uint64_t argument_buffer_offset);
        void DrawIndexedIndirect(const RhiBuffer &argument_buffer, uint64_t argument_buffer_offset);
        void DrawIndirectCount(const RhiBuffer &argument_buffer,
                               uint64_t argument_buffer_offset,
                               const RhiBuffer *count_buffer,
                               uint64_t count_buffer_offset,
                               uint32_t max_draw_count,
                               uint32_t stride);
        void DrawIndexedIndirectCount(const RhiBuffer &argument_buffer,
                                      uint64_t argument_buffer_offset,
                                      const RhiBuffer *count_buffer,
                                      uint64_t count_buffer_offset,
                                      uint32_t max_draw_count,
                                      uint32_t stride);
        void DrawMeshTask(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);

        // Computing
    public:
        void Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z);
        void DispatchIndirect(const RhiBuffer &argument_buffer, uint64_t argument_buffer_offset);

        // Barriers
    private:
        void SetImageLayout(RhiImage &image,
                            vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                            vk::PipelineStageFlags2 src_stages, vk::PipelineStageFlags2 dst_stages,
                            ERhiQueueType src_queue, ERhiQueueType dst_queue,
                            const vk::ImageSubresourceRange &subres = {});

    public:
        void SetImageBarrier(const RhiImageBarrier &barrier);
        void SetImageBarriers(const Array<RhiImageBarrier> &barriers);

        void SetBufferBarrier(const RhiBufferBarrier &barrier);
        void SetBufferBarriers(const Array<RhiBufferBarrier> &barriers);

        void SetMemoryBarrier(vk::AccessFlags2 src_access_mask,
                              vk::AccessFlags2 dst_access_mask,
                              vk::PipelineStageFlags2 src_stages,
                              vk::PipelineStageFlags2 dst_stages);

        // Resource utils
    public:
        void CopyBuffer(const RhiBuffer &src_buffer, const RhiBuffer &dst_buffer, const Array<RhiBufferCopyRegion> &regions);

        void CopyBufferToImage(const RhiBuffer &src_buffer, const RhiImage &dst_image, const Array<RhiBufferToImageCopyRegion> &regions);

        void CopyImage(const RhiImage &src_image, const RhiImage &dst_image, const Array<RhiTextureCopyRegion> &regions);

        void BlitImage(const RhiImage &src_image, const RhiImage &dst_image,
                       vk::Filter filter = vk::Filter::eLinear,
                       const Array<vk::ImageBlit2> &regions = {});

    public:
        void UpdateSubresource(const RhiImage &image, uint32_t level, const void *data,
                               uint64_t row_pitch, uint64_t depth_pitch,
                               RhiBuffer *upload_buffer = nullptr, uint64_t buffer_offset = 0u);

        // Debugging
    public:
        void BeginEvent(const String &name);
        void EndEvent();

    private:
        void SetImageBarrierImpl(const RhiImageBarrier &barrier);
        void FlushBarriers();
        void FlushState();
        void FlushDescriptorSet(uint32_t set, Array<RhiWriteBindingDesc> &writes);

    private:
        RhiDriver &m_driver;

    private:
        vk::CommandBuffer m_handle{VK_NULL_HANDLE};

    private:
        ERhiQueueType m_queue_type;

    private:
        struct PipelineBarrier
        {
            vk::PipelineStageFlags2 memory_src_stages{0U};
            vk::PipelineStageFlags2 memory_dst_stages{0U};
            vk::AccessFlags2 memory_src_access{0U};
            vk::AccessFlags2 memory_dst_access{0U};

            vk::PipelineStageFlags2 image_src_stages{0U};
            vk::PipelineStageFlags2 image_dst_stages{0U};
        };

        PipelineBarrier m_pipeline_barrier{};
        Array<vk::ImageMemoryBarrier2> m_image_barriers{};
        Array<vk::BufferMemoryBarrier2> m_buffer_barriers{};

    private:
        struct ResourceBinding
        {
            const RhiResourceView *view;
        };

        struct InternalState
        {
            bool is_rendering{false};

            RhiProgramHandle program;
            ResourceBinding bindings[MAX_DESCRIPTOR_SETS][MAX_BINDINGS]; // [set, binding]

            bool program_dirty{false};
            bool flags_set_dirty{0};
        } m_internal_state{};

    private:
        bool m_ended{true};

        friend class RhiDriver;
    };

    DEFINE_RHI_HANDLE(RhiCommandBuffer);

}
