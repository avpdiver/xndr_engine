#pragma once

namespace Be::Framework::RHI
{

    class RhiResource : public RefCounter, public Noncopyable
    {
    public:
        RhiResource(RhiDriver &driver) noexcept;

        virtual ~RhiResource() noexcept
        {
        }

    public:
        [[nodiscard]] forceinline RhiDriver &GetDriver() noexcept
        {
            return m_driver;
        }

    public:
        [[nodiscard]] virtual ERhiResourceType GetResourceType() const noexcept = 0;

    protected:
        RhiDevice &m_device;
        RhiDriver &m_driver;
    };

    DEFINE_RHI_HANDLE(RhiResource);

}