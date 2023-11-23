#pragma once

namespace Be::Framework::RHI
{

    using RhiBindlessIndex = uint32_t;

    class RhiDescriptorPoolRange
    {
    public:
        RhiDescriptorPoolRange(RhiBindlessDescriptorPool &pool,
                               vk::DescriptorSet descriptor_set,
                               uint32_t offset,
                               uint32_t size,
                               vk::DescriptorType type) noexcept;

    public:
        [[nodiscard]] vk::DescriptorSet GetDescriptorSet() const noexcept;
        [[nodiscard]] uint32_t GetOffset() const noexcept;

    private:
        Ref<RhiBindlessDescriptorPool> m_pool;

    private:
        vk::DescriptorSet m_descriptor_set;
        vk::DescriptorType m_type;

    private:
        uint32_t m_offset;
        uint32_t m_size;

    private:
        UniquePtr<RhiDescriptorPoolRange, Function<void(RhiDescriptorPoolRange *)>> m_callback;
    };

    class RhiBindlessDescriptorPool final
    {
    public:
        RhiBindlessDescriptorPool(RhiDevice &device, vk::DescriptorType type) noexcept;

    public:
        [[nodiscard]] RhiDescriptorPoolRange Allocate(uint32_t count) noexcept;
        void OnRangeDestroy(uint32_t offset, uint32_t size) noexcept;

    public:
        [[nodiscard]] forceinline vk::DescriptorSet GetDescriptorSet() const noexcept
        {
            return m_descriptor.set.get();
        }

        [[nodiscard]] forceinline const vk::DescriptorSetLayout GetDescriptorSetLayout() const noexcept
        {
            return m_descriptor.set_layout.get();
        }

        [[nodiscard]] forceinline uint32_t GetBinding() const noexcept
        {
            return 10 + static_cast<uint32_t>(m_type);
        }

    private:
        void ResizeHeap(uint32_t req_size) noexcept;

    private:
        RhiDevice &m_device;

    private:
        vk::DescriptorType m_type;
        uint32_t m_size{0};
        uint32_t m_offset{0};

    private:
        std::multimap<uint32_t, uint32_t> m_empty_ranges;

    private:
        struct Descriptor
        {
            vk::UniqueDescriptorPool pool;
            vk::UniqueDescriptorSetLayout set_layout;
            vk::UniqueDescriptorSet set;
        } m_descriptor;
    };

}
