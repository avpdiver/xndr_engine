#pragma once

namespace Be::Framework::RHI
{

    class RhiResourceView : public RhiResource
    {
    public:
        RhiResourceView(RhiDriver &driver) noexcept
            : RhiResource{driver}
        {
        }

        virtual ~RhiResourceView() noexcept = default;

    public:
        [[nodiscard]] forceinline const vk::WriteDescriptorSet &GetWriteDescriptorSet() const noexcept
        {
            return m_write_descriptor_set;
        }

        [[nodiscard]] forceinline RhiBindlessIndex GetDescriptorIndex() const noexcept
        {
            if (m_range)
            {
                return static_cast<RhiBindlessIndex>(m_range->GetOffset());
            }
            return MaxValue;
        }

    protected:
        UniquePtr<RhiDescriptorPoolRange> m_range;

    protected:
        vk::WriteDescriptorSet m_write_descriptor_set{};
    };

    DEFINE_RHI_HANDLE(RhiResourceView);

}