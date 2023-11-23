#pragma once

namespace Be::Framework::RHI::RhiInstance
{
    void Create() noexcept;
    void Destroy() noexcept;

    [[nodiscard]] vk::Instance VkHandle() noexcept;
    [[nodiscard]] const RhiAdapter &ChooseAdapter(const Array<const char *> &required_extensions) noexcept;
    [[nodiscard]] bool IsDebugUtilsSupported() noexcept;
}