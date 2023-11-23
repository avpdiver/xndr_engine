#include "systems/renderer/renderer.h"

#define KHRONOS_STATIC
#include <ktxvulkan.h>

namespace Be::System::Renderer
{

    static const char *KtxErrorToStr(const ktx_error_code_e err) noexcept
    {
        switch (err)
        {
        case KTX_SUCCESS:
            return "Operation was successful";
        case KTX_FILE_DATA_ERROR:
            return "The data in the file is inconsistent with the spec";
        case KTX_FILE_ISPIPE:
            return "The file is a pipe or named pipe";
        case KTX_FILE_OPEN_FAILED:
            return "The target file could not be opened";
        case KTX_FILE_OVERFLOW:
            return "The operation would exceed the max file size";
        case KTX_FILE_READ_ERROR:
            return "An error occurred while reading from the file";
        case KTX_FILE_SEEK_ERROR:
            return "An error occurred while seeking in the file";
        case KTX_FILE_UNEXPECTED_EOF:
            return "File does not have enough data to satisfy request";
        case KTX_FILE_WRITE_ERROR:
            return "An error occurred while writing to the file";
        case KTX_GL_ERROR:
            return "GL operations resulted in an error";
        case KTX_INVALID_OPERATION:
            return "The operation is not allowed in the current state";
        case KTX_INVALID_VALUE:
            return "A parameter value was not valid ";
        case KTX_NOT_FOUND:
            return "Requested key was not found";
        case KTX_OUT_OF_MEMORY:
            return "Not enough memory to complete the operation";
        case KTX_TRANSCODE_FAILED:
            return "Transcoding of block compressed texture failed";
        case KTX_UNKNOWN_FILE_FORMAT:
            return "The file not a KTX file";
        case KTX_UNSUPPORTED_TEXTURE_TYPE:
            return "The KTX file specifies an unsupported texture type";
        case KTX_UNSUPPORTED_FEATURE:
            return "Feature not included in in-use library or not yet implemented";
        case KTX_LIBRARY_NOT_LINKED:
            return "Library dependency (OpenGL or Vulkan) not linked into application";
        default:
            return "Unknown";
        };
    }

    static KTX_error_code Ktx2InputStreamRead(ktxStream *str, void *dst, const ktx_size_t count)
    {
        auto &is = *reinterpret_cast<InputStream *>(str->data.custom_ptr.address);
        is.Read(dst, count);
        return KTX_SUCCESS;
    }

    static KTX_error_code Ktx2InputStreamSkip(ktxStream *str, const ktx_size_t count)
    {
        auto &is = *reinterpret_cast<InputStream *>(str->data.custom_ptr.address);
        is.Skip(count);
        return KTX_SUCCESS;
    }

    static KTX_error_code Ktx2InputStreamWrite(ktxStream *str, const void *src, const ktx_size_t size, const ktx_size_t count)
    {
        auto &os = *reinterpret_cast<OutputStream *>(str->data.custom_ptr.address);
        os.Write(src, size * count);
        return KTX_SUCCESS;
    }

    static KTX_error_code Ktx2InputStreamGetPos(ktxStream *str, ktx_off_t *const offset)
    {
        auto &s = *reinterpret_cast<Stream *>(str->data.custom_ptr.address);
        *offset = s.GetPosition();
        return KTX_SUCCESS;
    }

    static KTX_error_code Ktx2InputStreamSetPos(ktxStream *str, const ktx_off_t offset)
    {
        auto &s = *reinterpret_cast<Stream *>(str->data.custom_ptr.address);
        s.SetPosition(offset);
        return KTX_SUCCESS;
    }

    static KTX_error_code Ktx2InputStreamGetSize(ktxStream *str, ktx_size_t *const size)
    {
        auto &is = *reinterpret_cast<InputStream *>(str->data.custom_ptr.address);
        *size = is.GetSize();
        return KTX_SUCCESS;
    }

    static void Ktx2InputStreamDestruct(ktxStream *str)
    {
        (void)str;
    }

    static void InputStream2KtxStream(InputStream &stream, ktxStream &ktx_stream) noexcept
    {
        ktx_stream.type = streamType::eStreamTypeCustom;
        ktx_stream.data.custom_ptr.address = &stream;
        ktx_stream.read = &Ktx2InputStreamRead;
        ktx_stream.skip = &Ktx2InputStreamSkip;
        ktx_stream.write = &Ktx2InputStreamWrite;
        ktx_stream.getpos = &Ktx2InputStreamGetPos;
        ktx_stream.setpos = &Ktx2InputStreamSetPos;
        ktx_stream.getsize = &Ktx2InputStreamGetSize;
        ktx_stream.destruct = &Ktx2InputStreamDestruct;
        ktx_stream.closeOnDestruct = KTX_FALSE;
    }

    TextureManager::TextureManager(RhiDriver &driver) noexcept
        : m_driver{driver}
    {
        CreateDummyTextures();
    }

    TextureHandle TextureManager::Load(const String &key, InputStream &stream, ETextureManagerFlag flags) noexcept
    {
        PROFILER_SCOPE;

        const Path path{key};

        RhiImageDesc desc{
            .type = ERhiImageType::e2D,
            .bind_flag = ERhiBindFlag::eShaderResource | ERhiBindFlag::eCopyDest,
            .depth = 1,
            .sample_count = 1,
            .initial_state = ERhiResourceState::eUndefined,
            .debug_name = key,
        };

        ktxStream ktx_stream;
        InputStream2KtxStream(stream, ktx_stream);

        ktxTexture2 *ktx{nullptr};
        auto result = ktxTexture2_CreateFromStream(&ktx_stream, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx);
        VERIFY(result == KTX_SUCCESS, "Failed load texture: {}", KtxErrorToStr(result));

        // Transcode
        if (ktxTexture2_NeedsTranscoding(ktx))
        {
            result = ktxTexture2_TranscodeBasis(ktx, KTX_TTF_BC7_RGBA, 0);
            VERIFY(result == KTX_SUCCESS, "Failed transcode texture: {}", KtxErrorToStr(result));
        }

        desc.format = (vk::Format)ktxTexture2_GetVkFormat(ktx);
        desc.width = ktx->baseWidth;
        desc.height = ktx->baseHeight;
        desc.mip_levels = ktx->numLevels;

        auto texture = MakeRefCounter<Texture>();
        texture->SetImage(m_dummy_image);
        texture->m_data.reserve(desc.mip_levels);

        auto level_width = desc.width;
        auto level_height = desc.height;

        for (uint32_t level = 0; level < desc.mip_levels; ++level)
        {
            auto &subres_data = texture->m_data.emplace_back();

            ktx_size_t offset;
            result = ktxTexture_GetImageOffset((ktxTexture *)ktx, level, 0, 0, &offset);
            VERIFY(result == KTX_SUCCESS, "Failed get mip level {} offset: {}", level, KtxErrorToStr(result));

            subres_data.data = ktxTexture_GetData((ktxTexture *)ktx) + offset;
            subres_data.level = level;

            RhiFormat::GetSizes(desc.format, level_width, level_height, subres_data.num_bytes, subres_data.row_bytes);

            level_width >>= 1;
            level_height >>= 1;
        }

        auto image = m_driver.CreateImage(desc);
        m_driver.UploadImage(*image, texture->m_data, ERhiResourceStateEnum::eShaderResource,
                             [texture, ktx, image]()
                             {
                                 texture->SetImage(image);
                                 texture->m_data.clear();
                                 ktxTexture_Destroy((ktxTexture *)ktx);
                             });

        return texture;
    }

    TextureHandle TextureManager::Load(const String &key, const Path &path, ETextureManagerFlag flags) noexcept
    {
        PROFILER_SCOPE;

        std::ifstream file{path, std::ios::in | std::ios::binary};
        VERIFY(file, "Failed to open texture file: {}", path.string());

        FileInputStream stream{file};
        return Load(key, stream, flags);
    }

    void TextureManager::CreateDummyTextures() noexcept
    {
        RhiImageDesc tex_desc{
            .type = ERhiImageType::e2D,
            .bind_flag = ERhiBindFlag::eShaderResource | ERhiBindFlag::eCopyDest,
            .format = vk::Format::eR32G32B32A32Sfloat,
            .width = 1,
            .height = 1,
            .depth = 1,
            .mip_levels = 1,
            .sample_count = 1,
            .initial_state = ERhiResourceState::eShaderResource,
            .generate_mips = false,
            .debug_name = "dummy_white_texture_1x1",
        };

        Colorf4 white_color(1.0f, 1.0f, 1.0f, 0.0f);
        auto &subres = tex_desc.data.emplace_back();
        subres.data = &white_color;
        subres.row_bytes = sizeof(Colorf4);
        subres.num_bytes = sizeof(Colorf4);
        subres.level = 0;

        m_dummy_image = m_driver.CreateImage(tex_desc);
        m_dummy_texture = MakeRefCounter<Texture>();
        m_dummy_texture->SetImage(m_dummy_image);
    }

}