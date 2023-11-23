#pragma once

namespace Be::Framework::RHI
{
    using RhiUploaderReadyCallback = std::function<void()>;

    class RhiResourceUploader final : public Noncopyable
    {
    public:
        void UploadBuffer(RhiBuffer &buffer, const Data &data,
                          ERhiResourceState final_state = ERhiResourceState::eShaderResource,
                          const RhiUploaderReadyCallback &callback = {}) noexcept;
        void UploadImage(RhiImage &image, const Array<RhiSubresourceData> &data,
                         ERhiResourceState final_state = ERhiResourceState::eShaderResource,
                         const RhiUploaderReadyCallback &callback = {}) noexcept;

    private:
        RhiResourceUploader() noexcept = default;
        ~RhiResourceUploader() noexcept;

    private:
        void Init(RhiDriver *driver) noexcept;

    private:
        void BeginFrame() noexcept;
        void EndFrame() noexcept;

    private:
        RhiDriver *m_driver{nullptr};
        RhiQueue *m_transfer_queue{nullptr};

    private:
        struct UploadInfo
        {
            RhiCommandBufferHandle transfer_cmd{};
            RhiCommandBufferHandle target_cmd{};
            RhiBufferHandle upload_buffer{};
            uint64_t fence_wait_value{0u};
            bool unified_queue{false};

            RhiUploaderReadyCallback callback{};
        };

    private:
        void AllocateUploadInfo(uint64_t staging_size, UploadInfo &upload_info) noexcept;
        void SubmitUpload(UploadInfo &upload_info) noexcept;

    private:
        Array<UploadInfo> m_free_list;
        Array<UploadInfo> m_wait_callback_list;
        Array<UploadInfo> m_wait_transfer_list;

        RhiFenceHandle m_fence;
        uint64_t m_fence_value{0u};

        MUTEX(m_mutex);

        friend class RhiDriver;
    };

}