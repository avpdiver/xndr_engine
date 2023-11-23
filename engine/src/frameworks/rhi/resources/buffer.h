#pragma once

namespace Be::Framework::RHI
{

    struct RhiBufferDesc
    {
        uint32_t bind_flag{0};
        uint64_t size{0};
        ERhiResourceUsage mem_usage{ERhiResourceUsage::eDefault};
        ERhiResourceState initial_state{ERhiResourceState::eUndefined};

        Data data{};

        String debug_name{};
    };

    class RhiBuffer final : public RhiMemoryResource
    {
    public:
        RhiBuffer(RhiDriver &driver, const RhiBufferDesc &desc) noexcept;

    public:
        ~RhiBuffer() noexcept;

    public:
        [[nodiscard]] forceinline vk::Buffer VkHandle() const noexcept
        {
            return m_handle;
        }

    public:
        [[nodiscard]] forceinline ERhiResourceType GetResourceType() const noexcept override
        {
            return ERhiResourceType::eBuffer;
        }

    public:
        [[nodiscard]] forceinline uint64_t GetSize() const noexcept
        {
            return m_size;
        }

    public:
        void Update(uint64_t buffer_offset, const Data &data) noexcept;
        void UpdateWithTextureData(uint64_t buffer_offset, uint64_t buffer_row_pitch, uint64_t buffer_depth_pitch,
                                   const void *src_data, uint64_t src_row_pitch, uint64_t src_depth_pitch,
                                   uint32_t num_rows, uint32_t num_slices) noexcept;

    private:
        vk::Buffer m_handle{VK_NULL_HANDLE};
        uint64_t m_size{0u};

        friend class RhiDriver;
    };

    DEFINE_RHI_HANDLE(RhiBuffer);

}