#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{

    RhiResource::RhiResource(RhiDriver &driver) noexcept
        : m_device{driver.GetDevice()},
          m_driver{driver}
    {
    }

}