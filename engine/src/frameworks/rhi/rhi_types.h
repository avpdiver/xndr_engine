#pragma once

#define DEFINE_RHI_HANDLE(T) using T##Handle = RefCountPtr<T>

namespace Be::Framework::RHI
{

    ITERABLE_ENUM(ERhiQueueType, uint8_t,
                  eGraphics,
                  eAsyncCompute,
                  eAsyncTransfer,
                  eAsyncVideo);

    constexpr auto INVALID_GPU_QUEUE = ERhiQueueTypeEnum::Count;

    enum class ERhiResourceType : uint8_t
    {
        eImage,
        eImageView,
        eBuffer,
        eBufferView,
        eSampler,
        eShader,
        eProgram,
        eSemaphore,
        eFence,
        ePipeline,
        eDescriptorSet,
        eQueryHeap,
    };

    enum class ERhiResourceUsage : uint8_t
    {
        eDefault,  // CPU no access, GPU read/write
        eUpload,   // CPU write, GPU read
        eReadback, // CPU read, GPU write
    };

    BIT_ENUM(ERhiResourceState, uint32_t,
             eUnknown = 0,
             eUndefined = BIT(0),
             eVertexBuffer = BIT(1),
             eUniformBuffer = BIT(2),
             eIndexBuffer = BIT(3),
             eRenderTarget = BIT(4),
             eUnorderedAccess = BIT(5),
             eDepthTarget = BIT(6),
             eDepthRead = BIT(7),
             eShaderResource = BIT(8),
             eStreamOut = BIT(9),
             eIndirectArgument = BIT(10),
             eCopyDest = BIT(11),
             eCopySource = BIT(12),
             eResolveDest = BIT(13),
             eResolveSource = BIT(14),
             eInputAttachment = BIT(15),
             ePresent = BIT(16),
             eShadingRate = BIT(17),
             eCommon = BIT(18));

    BIT_ENUM(ERhiBindFlag, uint16_t,
             eUnknown = 0,
             eRenderTarget = BIT(0),
             eDepthStencil = BIT(1),
             eShaderResource = BIT(2),
             eUnorderedAccess = BIT(3),
             eUniformBuffer = BIT(4),
             eIndexBuffer = BIT(5),
             eVertexBuffer = BIT(6),
             eAccelerationStructure = BIT(7),
             eRayTracing = BIT(8),
             eCopyDest = BIT(9),
             eCopySource = BIT(10),
             eShadingRateSource = BIT(11),
             eShaderTable = BIT(12),
             eIndirectBuffer = BIT(13));

    enum class ERhiViewDimension
    {
        eUnknown,
        eBuffer,
        eTexture1D,
        eTexture1DArray,
        eTexture2D,
        eTexture2DArray,
        eTexture2DMS,
        eTexture2DMSArray,
        eTexture3D,
        eTextureCube,
        eTextureCubeArray,
    };

    enum class ERhiImageType
    {
        e1D,
        e2D,
        e3D,
    };

    class RhiAdapter;
    class RhiDevice;
    class RhiQueue;
    class RhiBindlessDescriptorPool;
    class RhiDescriptorPoolRange;
    class RhiBindlessDescriptorPool;
    class RhiDriver;
    class RhiResourceStateTracker;
    class RhiResource;
    class RhiMemoryResource;
    class RhiResourceView;
    class RhiSampler;
    class RhiBuffer;
    class RhiBufferView;
    class RhiImage;
    class RhiImageView;
    class RhiSwapchain;
    class RhiShader;
    class RhiProgram;
    class RhiCommandBuffer;

}