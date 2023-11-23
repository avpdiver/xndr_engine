#include "frameworks/rhi/rhi.h"
#include "frameworks/rhi/utils/vulkan_helpers.h"

namespace Be::Framework::RHI
{

    RhiResourceUploader::~RhiResourceUploader() noexcept
    {
        m_free_list.clear();
        m_wait_callback_list.clear();
        m_wait_transfer_list.clear();
        m_fence.Reset();
    }

    void RhiResourceUploader::Init(RhiDriver *driver) noexcept
    {
        m_driver = driver;
        m_transfer_queue = &(m_driver->GetQueue(ERhiQueueType::eAsyncTransfer));

        m_fence = m_driver->CreateFence();

        m_free_list.reserve(32);
        m_wait_callback_list.reserve(32);
        m_wait_transfer_list.reserve(32);
    }

    void RhiResourceUploader::UploadBuffer(RhiBuffer &buffer, const Data &data, ERhiResourceState final_state, const RhiUploaderReadyCallback &callback) noexcept
    {
        PROFILER_SCOPE;

        if (data.empty())
        {
            return;
        }

        UploadInfo upload_info{.callback = callback};
        AllocateUploadInfo(data.size_bytes(), upload_info);

        upload_info.upload_buffer->Update(0u, data);
        upload_info.transfer_cmd->SetBufferBarrier({.buffer = buffer,
                                                    .new_state = ERhiResourceState::eCopyDest});

        RhiBufferCopyRegion region{.num_bytes = data.size_bytes()};

        upload_info.transfer_cmd->CopyBuffer(*upload_info.upload_buffer, buffer, {region});

        final_state = (final_state != ERhiResourceState::eUnknown ? final_state : ERhiResourceState::eCommon);

        if (upload_info.unified_queue)
        {
            upload_info.transfer_cmd->SetBufferBarrier({.buffer = buffer,
                                                        .new_state = final_state});
        }
        else
        {
            upload_info.transfer_cmd->SetBufferBarrier({.buffer = buffer,
                                                        .new_state = final_state,
                                                        .old_queue = ERhiQueueType::eAsyncTransfer,
                                                        .new_queue = ERhiQueueType::eGraphics});

            upload_info.target_cmd->SetBufferBarrier({.buffer = buffer,
                                                      .new_state = final_state,
                                                      .old_queue = ERhiQueueType::eAsyncTransfer,
                                                      .new_queue = ERhiQueueType::eGraphics});
        }

        SubmitUpload(upload_info);
    }

    void RhiResourceUploader::UploadImage(RhiImage &image, const Array<RhiSubresourceData> &data,
                                          ERhiResourceState final_state, const RhiUploaderReadyCallback &callback) noexcept
    {
        PROFILER_SCOPE;

        if (data.empty())
        {
            return;
        }

        UploadInfo upload_info{.callback = callback};
        AllocateUploadInfo(image.GetMemorySize(), upload_info);

        const auto mip_levels = image.GetMipLevels();

        RhiImageBarrier barrier{.image = image,
                                .mip_levels = mip_levels,
                                .array_layers = 1,
                                .new_state = ERhiResourceState::eCopyDest};

        uint64_t offset{0};
        for (const auto &d : data)
        {
            barrier.base_mip_level = d.level % mip_levels;
            barrier.base_array_layer = d.level / mip_levels;

            upload_info.transfer_cmd->SetImageBarrier(barrier);
            upload_info.transfer_cmd->UpdateSubresource(image, d.level,
                                                        d.data, d.row_bytes, d.num_bytes,
                                                        upload_info.upload_buffer,
                                                        offset);

            offset += d.num_bytes;
        }

        final_state = (final_state != ERhiResourceState::eUnknown ? final_state : ERhiResourceState::eCommon);

        // transfer ownership from transfer to graphics queue
        if (!upload_info.unified_queue)
        {
            barrier.base_mip_level = 0;
            barrier.mip_levels = VK_REMAINING_MIP_LEVELS;
            barrier.base_array_layer = 0;
            barrier.array_layers = VK_REMAINING_MIP_LEVELS;
            barrier.old_state = ERhiResourceState::eCopyDest;
            barrier.new_state = final_state;
            barrier.old_queue = ERhiQueueType::eAsyncTransfer;
            barrier.new_queue = ERhiQueueType::eGraphics;

            // ownership release barrier
            upload_info.transfer_cmd->SetImageBarrier(barrier);

            // ownership aquire barrier
            upload_info.target_cmd->SetImageBarrier(barrier);
        }

        SubmitUpload(upload_info);
    }

    void RhiResourceUploader::SubmitUpload(UploadInfo &upload_info) noexcept
    {
        PROFILER_SCOPE;

        EXCLUSIVE_LOCK(m_mutex);

        upload_info.transfer_cmd->End();
        upload_info.target_cmd->End();

        if (upload_info.unified_queue)
        {
            m_driver->GetQueue(upload_info.target_cmd->GetQueueType()).AddCommandBuffer(*upload_info.target_cmd);
            if (upload_info.callback)
            {
                m_wait_callback_list.push_back(std::move(upload_info));
            }
            else
            {
                m_free_list.push_back(std::move(upload_info));
            }
        }
        else
        {
            m_transfer_queue->AddSignalFence(*m_fence, ++m_fence_value, vk::PipelineStageFlagBits2::eAllTransfer);
            m_transfer_queue->Submit();

            upload_info.fence_wait_value = m_fence_value;
            m_wait_transfer_list.emplace_back(std::move(upload_info));
        }
    }

    void RhiResourceUploader::BeginFrame() noexcept
    {
        PROFILER_SCOPE;

        EXCLUSIVE_LOCK(m_mutex);
        if (m_wait_transfer_list.empty())
        {
            return;
        }

        const auto value = m_fence->GetCurrentValue();

        for (auto &u : m_wait_transfer_list)
        {
            if (u.fence_wait_value <= value)
            {
                m_driver->GetQueue(u.target_cmd->GetQueueType()).AddCommandBuffer(*u.target_cmd);

                m_wait_transfer_list.pop_back();
                if (u.callback)
                {
                    m_wait_callback_list.push_back(std::move(u));
                }
                else
                {
                    m_free_list.push_back(std::move(u));
                }
            }
        }
    }

    void RhiResourceUploader::EndFrame() noexcept
    {
        PROFILER_SCOPE;

        EXCLUSIVE_LOCK(m_mutex);

        for (auto &u : m_wait_callback_list)
        {
            u.callback();
            u.callback = {};
            m_free_list.push_back(std::move(u));
        }
        m_wait_callback_list.clear();
    }

    void RhiResourceUploader::AllocateUploadInfo(uint64_t staging_size, UploadInfo &upload_info) noexcept
    {
        PROFILER_SCOPE;
        
        RhiBufferDesc desc{.bind_flag = ERhiBindFlag::eCopySource,
                           .size = staging_size,
                           .mem_usage = ERhiResourceUsage::eUpload,
                           .debug_name = "ResourceUploadBuffer"};

        // synchronized block
        {
            EXCLUSIVE_LOCK(m_mutex);

            // create a new command list if there are no free ones:
            if (m_free_list.empty())
            {
                auto &c = m_free_list.emplace_back();
                c.upload_buffer = m_driver->CreateBuffer(desc);
            }

            upload_info = m_free_list.back();
            if (upload_info.upload_buffer->GetSize() < staging_size)
            {
                // Try to search for a staging buffer that can fit the request:
                for (auto &c : m_free_list)
                {
                    if (c.upload_buffer->GetSize() >= staging_size)
                    {
                        upload_info = c;
                        std::swap(c, m_free_list.back());
                        break;
                    }
                }
            }
            m_free_list.pop_back();
        }

        // If no buffer was found that fits the data, create one:
        if (upload_info.upload_buffer->GetSize() < staging_size)
        {
            upload_info.upload_buffer = m_driver->CreateBuffer(desc);
        }

        if (!upload_info.transfer_cmd)
        {
            if (staging_size <= 1'000'000 || m_transfer_queue->GetQueueType() == ERhiQueueType::eGraphics)
            {
                upload_info.transfer_cmd = m_driver->CreateCommandBuffer(ERhiQueueType::eGraphics);
                upload_info.target_cmd = upload_info.transfer_cmd;
                upload_info.unified_queue = true;
            }
            else
            {
                upload_info.transfer_cmd = m_driver->CreateCommandBuffer(ERhiQueueType::eAsyncTransfer);
                upload_info.target_cmd = m_driver->CreateCommandBuffer(ERhiQueueType::eGraphics);
                upload_info.unified_queue = false;
            }
        }

        // begin command list in valid state:
        upload_info.transfer_cmd->Begin();
        upload_info.target_cmd->Begin();
    }

}