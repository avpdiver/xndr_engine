#pragma once

#define VULKAN_HPP_FLAGS_MASK_TYPE_AS_PUBLIC
#define VK_NO_PROTOTYPES
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_TO_STRING
#if defined(BE_PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XCB_KHR
#endif

#define VMA_VULKAN_VERSION 1003000
#ifdef BE_VMA_DEBUG
#define VMA_DEBUG_LOG(format, ...)             \
    printf(format __VA_OPT__(, ) __VA_ARGS__); \
    printf("\n")
#endif
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#define VK_VERIFY(R, ...) VERIFY(static_cast<vk::Result>(R) == vk::Result::eSuccess __VA_OPT__(, ) __VA_ARGS__)

#include "base/base.h"

#include "frameworks/rhi/rhi_format.h"
#include "frameworks/rhi/rhi_constant.h"
#include "frameworks/rhi/rhi_types.h"

#include "frameworks/rhi/instance/adapter.h"
#include "frameworks/rhi/instance/instance.h"

#include "frameworks/rhi/states/states.h"
#include "frameworks/rhi/descriptors/bindless_descriptor_pool.h"
#include "frameworks/rhi/resources/resources.h"

#include "frameworks/rhi/driver/swapchain.h"
#include "frameworks/rhi/driver/command_buffer.h"
#include "frameworks/rhi/driver/queue.h"
#include "frameworks/rhi/driver/device.h"
#include "frameworks/rhi/driver/resource_uploader.h"
#include "frameworks/rhi/driver/driver.h"