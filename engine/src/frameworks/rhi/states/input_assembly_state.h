#pragma once

namespace Be::Framework::RHI
{
    struct RhiInputAssemblyState
    {
    public:
        forceinline void SetTopology(vk::PrimitiveTopology v) noexcept
        {
            if (topology == v)
            {
                return;
            }
            topology = v;
            hash = 0;
        }

        [[nodiscard]] forceinline vk::PrimitiveTopology GetTopology() const noexcept
        {
            return topology;
        }

    public:
        forceinline void SetPrimitiveRestart(bool v) noexcept
        {
            if (primitive_restart == v)
            {
                return;
            }
            primitive_restart = v;
            hash = 0;
        }

        [[nodiscard]] forceinline bool IsPrimitiveRestart() const noexcept
        {
            return primitive_restart;
        }

    public:
        [[nodiscard]] HashValue Hash() const noexcept
        {
            if (size_t(hash) == 0)
            {
                hash = HashOf(topology);
                hash += HashOf(primitive_restart);
            }
            return hash;
        }

    private:
        vk::PrimitiveTopology topology{vk::PrimitiveTopology::eTriangleList};
        bool primitive_restart{false};

    private:
        mutable HashValue hash{0};
    };

}

namespace std
{

    template <>
    struct hash<Be::Framework::RHI::RhiInputAssemblyState>
    {
        std::size_t operator()(const Be::Framework::RHI::RhiInputAssemblyState &v) const noexcept
        {
            return size_t(Be::HashOf(&v, sizeof(Be::Framework::RHI::RhiInputAssemblyState)));
        }
    };

}
