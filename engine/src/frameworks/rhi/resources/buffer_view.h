#pragma once

namespace Be::Framework::RHI
{

    struct RhiBufferViewDesc
    {
        RhiBufferHandle buffer;
        vk::DescriptorType view_type{};
        uint64_t offset{0};
        uint64_t size{MaxValue};
        vk::Format format{vk::Format::eUndefined};
        bool bindless{false};
        String debug_name;
    };

    class RhiBufferView final : public RhiResourceView
    {
    protected:
        RhiBufferView(RhiDriver &driver, const RhiBufferViewDesc &desc) noexcept;

    public:
        ~RhiBufferView() noexcept;

    public:
        [[nodiscard]] forceinline vk::BufferView VkHandle() const noexcept
        {
            return m_handle;
        }

    public:
        [[nodiscard]] forceinline ERhiResourceType GetResourceType() const noexcept override
        {
            return ERhiResourceType::eBufferView;
        }

    public:
        [[nodiscard]] forceinline const RhiBufferViewDesc &GetDesc() const noexcept
        {
            return m_desc;
        }

    public:
        [[nodiscard]] forceinline RhiBufferHandle GetBuffer() noexcept
        {
            return m_buffer;
        }

        vk::WriteDescriptorSet GetDescriptor() const noexcept;

    private:
        vk::BufferView m_handle{VK_NULL_HANDLE};
        vk::DescriptorBufferInfo m_descriptor_buffer_info{};

    private:
        RhiBufferHandle m_buffer;

    private:
        RhiBufferViewDesc m_desc{};

        friend class RhiDriver;
    };

    DEFINE_RHI_HANDLE(RhiBufferView);

}

namespace std
{

    template <>
    struct hash<Be::Framework::RHI::RhiBufferViewDesc>
    {
        std::size_t operator()(const Be::Framework::RHI::RhiBufferViewDesc &v) const
        {
            return size_t(Be::HashOf(&v, sizeof(Be::Framework::RHI::RhiBufferViewDesc)));
        }
    };

}
