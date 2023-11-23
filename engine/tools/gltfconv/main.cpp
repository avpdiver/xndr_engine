#include "systems/renderer/renderer.h"

#define KHRONOS_STATIC
#include <ktxvulkan.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <meshoptimizer.h>

#include "encoder/basisu_enc.h"
#include "encoder/jpgd.h"
#include "encoder/pvpngreader.h"
#include "encoder/basisu_comp.h"

using namespace Be;
using namespace Be::System::Renderer;

struct ModelLoadOptions
{
    struct
    {
        RenderGroupName opaque{"Opaque"};
        RenderGroupName mask{"AlphaMask"};
        RenderGroupName blend{"AlphaBlend"};
    } alpha_mode;

    struct
    {
        MaterialPropertyName base_color{"BaseColorTexture"};
        MaterialPropertyName occlusion_roughness_metallic{"OcclusionRoughnessMetallicTexture"};
        MaterialPropertyName normal{"NormalTexture"};
        MaterialPropertyName emissive{"EmissiveTexture"};
    } textures;

    struct
    {
        MaterialPropertyName base_color{"BaseColorFactor"};
        MaterialPropertyName metallic{"MetallicFactor"};
        MaterialPropertyName roughness{"RoughnessFactor"};
        MaterialPropertyName emissive{"EmissiveFactor"};
        MaterialPropertyName alpha_cutoff{"AlphaCutoff"};
    } factors;
};

struct InputMeshData
{
    uint32_t material_index{0};
    float scale_factor{1.0f};

    Array<Float3> positions;
    Array<Float3> normals;
    Array<Float4> tangents;
    Array<Colorf4> colors;
    Array<Float2> uvs;
    Array<uint32_t> indices;

    Array<ShaderInterop::Meshlet> meshlets;
    Array<uint32_t> meshlet_vertices;
    Array<ShaderInterop::MeshletTriangle> meshlet_triangles;
    Array<ShaderInterop::MeshletBounds> meshlet_bounds;
};

struct OutputModel
{
    Array<AssetName> textures{};
    Array<MaterialBlueprint> materials{};
    Array<SubMesh> meshes{};
    Array<SubMeshInstance> mesh_instances{};
    Array<uint32_t> mesh_materials{};

    ByteArray geometry_buffer;
};

static constexpr uint64_t BUFFER_ALIGNMENT{16};
static const Set<String> SUPPORTED_TEXTURES = {"png", "jpg", "jpeg", "tga", "ktx", "ktx2"};

// Model info
static Path GLTF_MODEL_PATH;
static String MODEL_NAME;
static ModelLoadOptions OPTIONS;

// Output info
static Path ASSET_PATH;
static Path OUTPUT_TEX_PATH;
static Path OUTPUT_MESH_PATH;
static Path OUTPUT_MODEL_PATH;

// Gltf info
static cgltf_data *GLTF{nullptr};
static Array<HashValue> GLTF_EXTENSTIONS{};

// Textures info
static uint32_t TEXTURE_COUNTS{0};
static Map<AssetName, uint32_t> TEXTURES{};
static Map<const cgltf_image *, AssetName> GLTF_TEXTURE_TO_NAME{};

// Meshes info
static Array<InputMeshData> MESHES{};

// Model info
static OutputModel OUT_MODEL{};

static const RenderGroupName &GetRenderGroup(cgltf_alpha_mode mode) noexcept
{
    switch (mode)
    {
    case cgltf_alpha_mode_blend:
        return OPTIONS.alpha_mode.blend;
    case cgltf_alpha_mode_mask:
        return OPTIONS.alpha_mode.mask;
    case cgltf_alpha_mode_opaque:
    default:
        return OPTIONS.alpha_mode.opaque;
    }
};

static void LoadPng(const Data &data, basisu::image &img) noexcept
{
    uint32_t width = 0, height = 0, num_chans = 0;
    auto image = pv_png::load_png(data.data(), data.size(), 4, width, height, num_chans);
    VERIFY(image, "Failed loading PNG image");

    img.grant_ownership(reinterpret_cast<basisu::color_rgba *>(image), width, height);
}

static void LoadTga(const Data &data, basisu::image &img) noexcept
{
    int w = 0, h = 0, n_chans = 0;
    auto image_data = basisu::read_tga((const uint8_t *)data.data(), data.size(), w, h, n_chans);

    if ((!image_data) || (!w) || (!h) || ((n_chans != 3) && (n_chans != 4)))
    {
        if (image_data)
        {
            free(image_data);
        }
        FATAL("Failed loading TGA image");
    }

    img.resize(w, h);

    const uint8_t *src = image_data;
    for (int y = 0; y < h; y++)
    {
        auto dst = &img(0, y);

        for (int x = 0; x < w; x++)
        {
            dst->r = src[0];
            dst->g = src[1];
            dst->b = src[2];
            dst->a = (n_chans == 3) ? 255 : src[3];

            src += n_chans;
            ++dst;
        }
    }

    free(image_data);
}

static void LoadJpg(const Data &data, basisu::image &img) noexcept
{
    int width = 0, height = 0, actual_comps = 0;
    auto image_data = jpgd::decompress_jpeg_image_from_memory(data.data(), data.size(), &width, &height, &actual_comps, 4, jpgd::jpeg_decoder::cFlagLinearChromaFiltering);
    VERIFY(image_data, "Failed to load JPG file");

    img.init(image_data, width, height, 4);
    free(image_data);
}

static void ProcessSimpleImage(const Path &filename, const Data &data, const String &ext) noexcept
{
    basisu::image img;
    if (ext == "png")
    {
        LoadPng(data, img);
    }
    else if (ext == "jpg" || ext == "jpeg")
    {
        LoadJpg(data, img);
    }
    else if (ext == "tga")
    {
        LoadTga(data, img);
    }
    else
    {
        FATAL("Unknown texture format: {}", ext);
    }

    basisu::basis_compressor_params compr_params;

    compr_params.m_source_images.push_back(img);
    compr_params.m_check_for_alpha = true;
    compr_params.m_perceptual = false;
    compr_params.m_mip_gen = true;
    compr_params.m_mip_srgb = false;
    compr_params.m_create_ktx2_file = true;

    compr_params.m_uastc = true;
    compr_params.m_rdo_uastc_multithreading = false;
    compr_params.m_multithreading = false;
    compr_params.m_debug = false;
    compr_params.m_status_output = false;
    compr_params.m_compute_stats = false;

    basisu::job_pool jpool(1);
    compr_params.m_pJob_pool = &jpool;

    basisu::basis_compressor compr;
    basisu::enable_debug_printf(true);

    VERIFY(compr.init(compr_params), "Failed to init BASISU compressor");

    auto result = compr.process();
    VERIFY(result == basisu::basis_compressor::cECSuccess, "Failed to BASISU compress file");

    const auto &ktx2 = compr.get_output_ktx2_file();

    std::fstream file;
    file.open(filename, std::ios::out | std::ios::binary);
    file.write((const char *)ktx2.data(), ktx2.size_in_bytes());
    file.flush();
    file.close();
}

static void ProcessKtx(const Path &filename, const Data &data) noexcept
{
    ktxTexture2 *ktx{nullptr};
    auto result = ktxTexture2_CreateFromMemory((const uint8_t *)data.data(), data.size(), KTX_TEXTURE_CREATE_NO_FLAGS, &ktx);
    VERIFY(result == KTX_SUCCESS, "Failed load texture");

    // Transcode
    if (ktxTexture2_NeedsTranscoding(ktx))
    {
        ktxTexture2_TranscodeBasis(ktx, KTX_TTF_BC7_RGBA, 0);
    }

    const auto format = ktxTexture2_GetVkFormat(ktx);
    const auto width = ktx->baseWidth;
    const auto height = ktx->baseHeight;
    const auto mip_levels = ktx->numLevels;

    ktx->generateMipmaps = (mip_levels < MipLevelsCount(width, height));

    result = ktxTexture_WriteToNamedFile((ktxTexture *)ktx, filename.string().c_str());
    VERIFY(result == KTX_SUCCESS, "Failed to save texture: {}", filename.string());
}

static void ProcessImage(const AssetName &name, const Data &data, const String &ext) noexcept
{
    VERIFY(SUPPORTED_TEXTURES.contains(ext), "Unsupported texture formate: {}", ext);

    const auto filename = ASSET_PATH / OUTPUT_TEX_PATH / name.c_str();
    LOG_INFO("Texture output filename: {}", filename.string());

    if (ext.starts_with("ktx"))
    {
        ProcessKtx(filename, data);
    }
    else
    {
        ProcessSimpleImage(filename, data, ext);
    }
}

static void ProcessImage(const AssetName &name, const String &path) noexcept
{
    Path filename{path};

    const auto ext = StringUtils::ToLower(filename.filename().extension().string());

    std::vector<byte_t> file;

    const auto iflags = std::ios::in | std::ios::binary | std::ios::ate;
    if (auto ifs = std::ifstream{filename, iflags})
    {
        file.resize(ifs.tellg());
        ifs.seekg(0, std::ios::beg);
        ifs.read(reinterpret_cast<char *>(file.data()), file.size());
    }

    ProcessImage(name, file, ext);
}

static uint32_t ProcessTexture(const cgltf_texture_view texture, const String &default_name, bool srgb) noexcept
{
    if (!texture.texture)
    {
        FATAL("Invalid GLTF texture");
        return 0;
    }

    const auto image = texture.texture->image;
    const auto it = GLTF_TEXTURE_TO_NAME.find(image);
    if (it != GLTF_TEXTURE_TO_NAME.end())
    {
        return TEXTURES.at(it->second);
    }

    String name;
    String filepath;

    if (image->uri)
    {
        filepath = image->uri;
        name = filepath;
    }
    else
    {
        name = default_name + ".ktx";
    }

    AssetName fixed_name{name.c_str()};
    if (image->buffer_view)
    {
        String ext{image->mime_type};
        StringUtils::ReplaceFirst(ext, "image/", "");
        ext = StringUtils::ToLower(ext);

        Data data{(byte_t *)(image->buffer_view->buffer->data) + image->buffer_view->offset, image->buffer_view->size};
        ProcessImage(fixed_name, data, ext);
    }
    else
    {
        fixed_name = name.c_str();
        ProcessImage(fixed_name, filepath);
    }

    TEXTURES[fixed_name] = TEXTURE_COUNTS;
    GLTF_TEXTURE_TO_NAME[image] = fixed_name;

    return TEXTURE_COUNTS++;
}

static void ProcessMaterials() noexcept
{
    const auto use_emissive_strength = (std::find_if(GLTF_EXTENSTIONS.begin(), GLTF_EXTENSTIONS.end(),
                                                     [](const HashValue &rhs)
                                                     { return rhs == ConstHashOf("KHR_materials_emissive_strength"); }) != GLTF_EXTENSTIONS.end());

    for (size_t i = 0; i < GLTF->materials_count; ++i)
    {
        const auto &gltf_material = GLTF->materials[i];
        const auto emissive_strenght = use_emissive_strength
                                           ? gltf_material.emissive_strength.emissive_strength
                                           : 1.0f;

        auto &mat = OUT_MODEL.materials.emplace_back();

        mat.render_group = GetRenderGroup(gltf_material.alpha_mode);

        // Emissive factor
        auto &prop = mat.properties.emplace_back();
        prop.name = OPTIONS.factors.emissive;
        prop.type = EMaterialProperyType::eVec4;
        prop.vec4_value[0] = emissive_strenght * gltf_material.emissive_factor[0];
        prop.vec4_value[1] = emissive_strenght * gltf_material.emissive_factor[1];
        prop.vec4_value[2] = emissive_strenght * gltf_material.emissive_factor[2];
        prop.vec4_value[3] = 0.0f;

        // Alpha cutoff
        prop = mat.properties.emplace_back();
        prop.name = OPTIONS.factors.alpha_cutoff;
        prop.type = EMaterialProperyType::eFloat;
        prop.float_value = gltf_material.alpha_cutoff;

        // Normal texture
        prop = mat.properties.emplace_back();
        prop.name = OPTIONS.textures.normal;
        prop.type = EMaterialProperyType::eTex2d;
        prop.uint_value = ProcessTexture(gltf_material.normal_texture, MODEL_NAME + "_normal_texture", false);

        // Emissive texture
        prop = mat.properties.emplace_back();
        prop.name = OPTIONS.textures.emissive;
        prop.type = EMaterialProperyType::eTex2d;
        prop.uint_value = ProcessTexture(gltf_material.emissive_texture, MODEL_NAME + "_emissive_texture", true);

        if (gltf_material.has_pbr_metallic_roughness)
        {
            LOG_INFO("PBR Metallic Roughness material");

            // Base color texture
            prop = mat.properties.emplace_back();
            prop.name = OPTIONS.textures.base_color;
            prop.type = EMaterialProperyType::eTex2d;
            prop.uint_value = ProcessTexture(gltf_material.pbr_metallic_roughness.base_color_texture, MODEL_NAME + "_base_texture", true);

            // Metallic roughness texture
            prop = mat.properties.emplace_back();
            prop.name = OPTIONS.textures.occlusion_roughness_metallic;
            prop.type = EMaterialProperyType::eTex2d;
            prop.uint_value = ProcessTexture(gltf_material.pbr_metallic_roughness.metallic_roughness_texture, MODEL_NAME + "_mr_texture", false);

            // Base color factor
            prop = mat.properties.emplace_back();
            prop.name = OPTIONS.factors.base_color;
            prop.type = EMaterialProperyType::eVec4;
            prop.vec4_value[0] = gltf_material.pbr_metallic_roughness.base_color_factor[0];
            prop.vec4_value[1] = gltf_material.pbr_metallic_roughness.base_color_factor[1];
            prop.vec4_value[2] = gltf_material.pbr_metallic_roughness.base_color_factor[2];
            prop.vec4_value[3] = gltf_material.pbr_metallic_roughness.base_color_factor[3];

            // Metallic factor
            prop = mat.properties.emplace_back();
            prop.name = OPTIONS.factors.metallic;
            prop.type = EMaterialProperyType::eFloat;
            prop.float_value = gltf_material.pbr_metallic_roughness.metallic_factor;

            // Roughness factor
            prop = mat.properties.emplace_back();
            prop.name = OPTIONS.factors.roughness;
            prop.type = EMaterialProperyType::eFloat;
            prop.float_value = gltf_material.pbr_metallic_roughness.roughness_factor;
        }
        else if (gltf_material.has_pbr_specular_glossiness)
        {
            LOG_INFO("PBR Specular Glossiness material");

            // Base color texture
            prop = mat.properties.emplace_back();
            prop.name = OPTIONS.textures.base_color;
            prop.type = EMaterialProperyType::eTex2d;
            prop.uint_value = ProcessTexture(gltf_material.pbr_specular_glossiness.diffuse_texture, MODEL_NAME + "_base_texture", true);

            // Base color factor
            prop = mat.properties.emplace_back();
            prop.name = OPTIONS.factors.base_color;
            prop.type = EMaterialProperyType::eVec4;
            prop.vec4_value[0] = gltf_material.pbr_specular_glossiness.diffuse_factor[0];
            prop.vec4_value[1] = gltf_material.pbr_specular_glossiness.diffuse_factor[1];
            prop.vec4_value[2] = gltf_material.pbr_specular_glossiness.diffuse_factor[2];
            prop.vec4_value[3] = gltf_material.pbr_specular_glossiness.diffuse_factor[3];

            // Roughness factor
            prop = mat.properties.emplace_back();
            prop.name = OPTIONS.factors.roughness;
            prop.type = EMaterialProperyType::eFloat;
            prop.float_value = (1.0f - gltf_material.pbr_specular_glossiness.glossiness_factor);
        }
    }
}

static void ProcessMeshes() noexcept
{
    static constexpr uint32_t index_map[] = {0, 2, 1};

    auto GetMaterialIndex = [&](const cgltf_material *mat) -> uint32_t
    {
        if (!mat)
        {
            return 0;
        }
        return (uint32_t)(mat - GLTF->materials);
    };

    Map<const cgltf_mesh *, Array<uint32_t>> mesh_to_primitives;
    uint32_t primitive_index{0};

    for (size_t mesh_idx = 0; mesh_idx < GLTF->meshes_count; ++mesh_idx)
    {
        const auto &mesh = GLTF->meshes[mesh_idx];
        auto &primitives = mesh_to_primitives[&mesh];

        for (size_t prim_idx = 0; prim_idx < mesh.primitives_count; ++prim_idx)
        {
            const auto &primitive = mesh.primitives[prim_idx];
            primitives.push_back(primitive_index++);

            auto &mesh_data = MESHES.emplace_back();

            mesh_data.material_index = GetMaterialIndex(primitive.material);
            mesh_data.indices.resize(primitive.indices->count);

            for (size_t i = 0; i < primitive.indices->count; i += 3)
            {
                mesh_data.indices[i + 0] = (uint32_t)cgltf_accessor_read_index(primitive.indices, i + index_map[0]);
                mesh_data.indices[i + 1] = (uint32_t)cgltf_accessor_read_index(primitive.indices, i + index_map[1]);
                mesh_data.indices[i + 2] = (uint32_t)cgltf_accessor_read_index(primitive.indices, i + index_map[2]);
            }

            for (size_t attr_idx = 0; attr_idx < primitive.attributes_count; ++attr_idx)
            {
                const auto &attribute = primitive.attributes[attr_idx];
                const auto name = attribute.name;

                auto ReadAttributeData = [&attribute, &name](const char *stream_name, auto &stream, uint32_t num_components)
                {
                    if (strcmp(name, stream_name) == 0)
                    {
                        stream.resize(attribute.data->count);
                        for (size_t i = 0; i < attribute.data->count; ++i)
                        {
                            VERIFY(cgltf_accessor_read_float(attribute.data, i, &stream[i].x, num_components), "Failed read GLTF accessor.");
                        }
                    }
                };
                ReadAttributeData("POSITION", mesh_data.positions, 3);
                ReadAttributeData("NORMAL", mesh_data.normals, 3);
                ReadAttributeData("TANGENT", mesh_data.tangents, 4);
                ReadAttributeData("TEXCOORD_0", mesh_data.uvs, 2);
                ReadAttributeData("COLOR_0", mesh_data.colors, 4);
            }

            for (const auto &position : mesh_data.positions)
            {
                mesh_data.scale_factor = std::max(std::abs(position.x), mesh_data.scale_factor);
                mesh_data.scale_factor = std::max(std::abs(position.y), mesh_data.scale_factor);
                mesh_data.scale_factor = std::max(std::abs(position.z), mesh_data.scale_factor);
            }

            for (auto &position : mesh_data.positions)
            {
                position /= mesh_data.scale_factor;
                VERIFY(std::abs(position.x) <= 1.0f && std::abs(position.y) <= 1.0f && std::abs(position.z) <= 1.0f, "Failed to scale GLTF mesh.");
            }
        }
    }

    for (size_t i = 0; i < GLTF->nodes_count; i++)
    {
        const auto &node = GLTF->nodes[i];

        if (!node.mesh)
        {
            continue;
        }
        Float4x4 tmp_mtx;
        cgltf_node_transform_world(&node, &tmp_mtx[0][0]);

        Matrix4x4 local_to_world{tmp_mtx};
        for (int primitive : mesh_to_primitives[node.mesh])
        {
            const auto &mesh_data = MESHES[primitive];
            auto &inst = OUT_MODEL.mesh_instances.emplace_back();
            inst.mesh = primitive;
            inst.transform = local_to_world * Math::Scale((double)mesh_data.scale_factor);
        }
    }
}

static usize_t GenerateMeshlets() noexcept
{
    uint64_t buffer_size{0};

    for (auto &mesh_data : MESHES)
    {
        meshopt_optimizeVertexCache(mesh_data.indices.data(), mesh_data.indices.data(), mesh_data.indices.size(), mesh_data.positions.size());
        meshopt_optimizeOverdraw(mesh_data.indices.data(), mesh_data.indices.data(), mesh_data.indices.size(), (float *)mesh_data.positions.data(), mesh_data.positions.size(), sizeof(Float3), 1.05f);

        Array<uint32_t> remap(mesh_data.positions.size());

        meshopt_optimizeVertexFetchRemap(remap.data(), mesh_data.indices.data(), mesh_data.indices.size(), mesh_data.positions.size());
        meshopt_remapIndexBuffer(mesh_data.indices.data(), mesh_data.indices.data(), mesh_data.indices.size(), remap.data());
        meshopt_remapVertexBuffer(mesh_data.positions.data(), mesh_data.positions.data(), mesh_data.positions.size(), sizeof(Float3), remap.data());
        if (!mesh_data.normals.empty())
        {
            meshopt_remapVertexBuffer(mesh_data.normals.data(), mesh_data.normals.data(), mesh_data.normals.size(), sizeof(Float3), remap.data());
        }
        if (!mesh_data.tangents.empty())
        {
            meshopt_remapVertexBuffer(mesh_data.tangents.data(), mesh_data.tangents.data(), mesh_data.tangents.size(), sizeof(Float4), remap.data());
        }
        if (!mesh_data.uvs.empty())
        {
            meshopt_remapVertexBuffer(mesh_data.uvs.data(), mesh_data.uvs.data(), mesh_data.uvs.size(), sizeof(Float2), remap.data());
        }
        if (!mesh_data.colors.empty())
        {
            meshopt_remapVertexBuffer(mesh_data.colors.data(), mesh_data.colors.data(), mesh_data.colors.size(), sizeof(Float4), remap.data());
        }

        // Meshlet generation
        const auto max_meshlets = meshopt_buildMeshletsBound(mesh_data.indices.size(), MESHLET_MAX_VERTICES, MESHLET_MAX_TRIANGLES);

        mesh_data.meshlets.resize(max_meshlets);
        mesh_data.meshlet_vertices.resize(max_meshlets * MESHLET_MAX_VERTICES);

        Array<unsigned char> meshlet_triangles(max_meshlets * MESHLET_MAX_TRIANGLES * 3);
        Array<meshopt_Meshlet> meshlets(max_meshlets);

        auto meshlet_count = meshopt_buildMeshlets(meshlets.data(), mesh_data.meshlet_vertices.data(), meshlet_triangles.data(),
                                                   mesh_data.indices.data(), mesh_data.indices.size(), (float *)mesh_data.positions.data(),
                                                   mesh_data.positions.size(), sizeof(Float3), MESHLET_MAX_VERTICES, MESHLET_MAX_TRIANGLES, 0);

        // Trimming
        const auto &last = meshlets[meshlet_count - 1];
        meshlet_triangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
        meshlets.resize(meshlet_count);

        mesh_data.meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
        mesh_data.meshlets.resize(meshlet_count);
        mesh_data.meshlet_bounds.resize(meshlet_count);
        mesh_data.meshlet_triangles.resize(meshlet_triangles.size() / 3);

        uint32_t triangle_offset{0};
        for (size_t i = 0; i < meshlet_count; ++i)
        {
            const auto &meshlet = meshlets[i];

            Float3 min{FLT_MAX, FLT_MAX, FLT_MAX};
            Float3 max{-FLT_MAX, -FLT_MAX, -FLT_MAX};
            for (uint32_t k = 0; k < meshlet.triangle_count * 3; ++k)
            {
                auto idx = mesh_data.meshlet_vertices[meshlet.vertex_offset + meshlet_triangles[meshlet.triangle_offset + k]];
                const Float3 &p = mesh_data.positions[idx];
                max = Math::Max(max, p);
                min = Math::Min(min, p);
            }

            auto &out_bounds = mesh_data.meshlet_bounds[i];
            out_bounds.center = (max + min) / 2.0f;
            out_bounds.extents = (max - min) / 2.0f;

            // Encode triangles and get rid of 4 byte padding
            auto source_triangles = meshlet_triangles.data() + meshlet.triangle_offset;
            for (uint32_t triIdx = 0; triIdx < meshlet.triangle_count; ++triIdx)
            {
                auto &tri = mesh_data.meshlet_triangles[triIdx + triangle_offset];
                tri.v0 = *source_triangles++;
                tri.v1 = *source_triangles++;
                tri.v2 = *source_triangles++;
            }

            auto &out_meshlet = mesh_data.meshlets[i];
            out_meshlet.triangle_count = meshlet.triangle_count;
            out_meshlet.triangle_offset = triangle_offset;
            out_meshlet.vertex_count = meshlet.vertex_count;
            out_meshlet.vertex_offset = meshlet.vertex_offset;

            triangle_offset += meshlet.triangle_count;
        }
        mesh_data.meshlet_triangles.resize(triangle_offset);

        buffer_size += AlignUp(mesh_data.positions.size() * sizeof(VertexPositionType), BUFFER_ALIGNMENT);
        buffer_size += AlignUp(mesh_data.uvs.size() * sizeof(VertexUVType), BUFFER_ALIGNMENT);
        buffer_size += AlignUp(mesh_data.normals.size() * sizeof(VertexNormalType), BUFFER_ALIGNMENT);
        buffer_size += AlignUp(mesh_data.colors.size() * sizeof(VertexColorType), BUFFER_ALIGNMENT);
        buffer_size += AlignUp(mesh_data.indices.size() * sizeof(uint32_t), BUFFER_ALIGNMENT);

        buffer_size += AlignUp(mesh_data.meshlets.size() * sizeof(ShaderInterop::Meshlet), BUFFER_ALIGNMENT);
        buffer_size += AlignUp(mesh_data.meshlet_vertices.size() * sizeof(uint32_t), BUFFER_ALIGNMENT);
        buffer_size += AlignUp(mesh_data.meshlet_triangles.size() * sizeof(ShaderInterop::MeshletTriangle), BUFFER_ALIGNMENT);
        buffer_size += AlignUp(mesh_data.meshlet_bounds.size() * sizeof(ShaderInterop::MeshletBounds), BUFFER_ALIGNMENT);
    }

    VERIFY(buffer_size < std::numeric_limits<uint32_t>::max(), "Offset stored in 32-bit int");

    return buffer_size;
}

static void GenerateOutputModel(const usize_t buffer_size) noexcept
{
    // Write textures
    OUT_MODEL.textures.resize(TEXTURES.size());
    for (const auto &[n, i] : TEXTURES)
    {
        OUT_MODEL.textures[i] = (OUTPUT_TEX_PATH / n.c_str()).string().c_str();
    }

    // Prepare geometry buffer
    OUT_MODEL.geometry_buffer.resize(buffer_size);

    uint64_t data_offset{0};
    auto &gbuf = OUT_MODEL.geometry_buffer;

    auto CopyData = [&data_offset, &gbuf](const void *source, uint64_t size)
    {
        memcpy(gbuf.data() + data_offset, source, size);
        data_offset = AlignUp(data_offset + size, BUFFER_ALIGNMENT);
    };

    for (const auto &mesh_data : MESHES)
    {
        auto &out_mesh = OUT_MODEL.meshes.emplace_back();

        OUT_MODEL.mesh_materials.push_back(mesh_data.material_index);

        if (!mesh_data.positions.empty())
        {
            out_mesh.flags |= EMeshFlag::ePosition;
        }
        if (!mesh_data.normals.empty())
        {
            out_mesh.flags |= EMeshFlag::eNormal;
        }
        if (!mesh_data.tangents.empty())
        {
            out_mesh.flags |= EMeshFlag::eTangent;
        }
        if (!mesh_data.uvs.empty())
        {
            out_mesh.flags |= EMeshFlag::eUV;
        }
        if (!mesh_data.colors.empty())
        {
            out_mesh.flags |= EMeshFlag::eVertexColor;
        }

        out_mesh.positions_format = vk::Format::eR16G16B16A16Snorm;
        out_mesh.meshlets_count = (uint32_t)mesh_data.meshlets.size();

        // Bounding box
        BBoxf bbox{mesh_data.positions.data(), mesh_data.positions.size()};
        out_mesh.bbox = bbox;

        // Pack and save positions
        {
            out_mesh.positions_view.offset = data_offset;
            out_mesh.positions_view.size = (uint32_t)(mesh_data.positions.size() * sizeof(VertexPositionType));
            out_mesh.positions_view.elements = (uint32_t)mesh_data.positions.size();

            auto ptr = (VertexPositionType *)(gbuf.data() + data_offset);
            for (const auto &position : mesh_data.positions)
            {
                *ptr++ = {Math::PackRgba16Snorm(Float4{position, 0.f})};
            }
            data_offset = AlignUp(data_offset + mesh_data.positions.size() * sizeof(VertexPositionType), BUFFER_ALIGNMENT);
        }

        // Pack and save normals and tangents
        {
            out_mesh.normals_view.offset = data_offset;
            out_mesh.normals_view.size = (uint32_t)(mesh_data.normals.size() * sizeof(VertexNormalType));
            out_mesh.normals_view.elements = (uint32_t)mesh_data.normals.size();

            auto ptr = (VertexNormalType *)(gbuf.data() + data_offset);
            for (size_t i = 0; i < mesh_data.normals.size(); ++i)
            {
                *ptr++ = {
                    Math::PackRgb10A2Snorm(Float4{mesh_data.normals[i], 0.f}),
                    Math::PackRgb10A2Snorm(mesh_data.tangents.empty() ? Float4(1, 0, 0, 1) : mesh_data.tangents[i]),
                };
            }
            data_offset = AlignUp(data_offset + mesh_data.normals.size() * sizeof(VertexNormalType), BUFFER_ALIGNMENT);
        }

        // Pack and save colors
        if (!mesh_data.colors.empty())
        {
            out_mesh.colors_view.offset = data_offset;
            out_mesh.colors_view.size = (uint32_t)(mesh_data.colors.size() * sizeof(VertexColorType));
            out_mesh.colors_view.elements = (uint32_t)mesh_data.colors.size();

            auto ptr = (VertexColorType *)(gbuf.data() + data_offset);
            for (const auto &color : mesh_data.colors)
            {
                *ptr++ = {Math::PackRgba8Unorm(color)};
            }
            data_offset = AlignUp(data_offset + mesh_data.colors.size() * sizeof(VertexColorType), BUFFER_ALIGNMENT);
        }

        // Pack and save UVs
        if (!mesh_data.uvs.empty())
        {
            out_mesh.uvs_view.offset = data_offset;
            out_mesh.uvs_view.size = (uint32_t)(mesh_data.uvs.size() * sizeof(VertexUVType));
            out_mesh.uvs_view.elements = (uint32_t)mesh_data.uvs.size();

            auto ptr = (VertexUVType *)(gbuf.data() + data_offset);
            for (const auto &uv : mesh_data.uvs)
            {
                *ptr++ = {Math::PackRg16Float(uv)};
            }
            data_offset = AlignUp(data_offset + mesh_data.uvs.size() * sizeof(VertexUVType), BUFFER_ALIGNMENT);
        }

        // Pack and save indecies
        {
            out_mesh.small_indices = (mesh_data.positions.size() < std::numeric_limits<uint16_t>::max());
            const auto index_size = (out_mesh.small_indices ? sizeof(uint16_t) : sizeof(uint32_t));

            out_mesh.indieces_view.offset = data_offset;
            out_mesh.indieces_view.size = (uint32_t)(mesh_data.indices.size() * index_size);
            out_mesh.indieces_view.elements = (uint32_t)mesh_data.indices.size();

            auto *ptr = (byte_t *)(gbuf.data() + data_offset);
            for (auto index : mesh_data.indices)
            {
                memcpy(ptr, &index, index_size);
                ptr += index_size;
            }
            data_offset = AlignUp(data_offset + mesh_data.indices.size() * index_size, BUFFER_ALIGNMENT);
        }

        // Copy meshlets data
        out_mesh.meshlets_location = (uint32_t)data_offset;
        CopyData(mesh_data.meshlets.data(), sizeof(ShaderInterop::Meshlet) * mesh_data.meshlets.size());

        // Copy meshlets vertices data
        out_mesh.meshlet_vertices_location = (uint32_t)data_offset;
        CopyData(mesh_data.meshlet_vertices.data(), sizeof(uint32_t) * mesh_data.meshlet_vertices.size());

        // Copy meshlets triangles data
        out_mesh.meshlet_triangles_location = (uint32_t)data_offset;
        CopyData(mesh_data.meshlet_triangles.data(), sizeof(ShaderInterop::MeshletTriangle) * mesh_data.meshlet_triangles.size());

        // Copy meshlets bounds data
        out_mesh.meshlet_bounds_location = (uint32_t)data_offset;
        CopyData(mesh_data.meshlet_bounds.data(), sizeof(ShaderInterop::MeshletBounds) * mesh_data.meshlet_bounds.size());
    }
}

static void WriteMesh() noexcept
{
    auto filename = ASSET_PATH / OUTPUT_MESH_PATH / (MODEL_NAME + ".bemesh");
    LOG_INFO("Mesh output filename: {}", filename.string());

    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::binary);
    FileOutputStream stream{file};

    // Write meshes count
    uint32_t size = (uint32_t)OUT_MODEL.meshes.size();
    stream << size;
    LOG_INFO("Submeshes count: {}", size);

    // Write mesh instances count
    size = (uint32_t)OUT_MODEL.mesh_instances.size();
    stream << size;
    LOG_INFO("Submesh instances count: {}", size);

    // Write geometry buffer size
    size = (uint32_t)OUT_MODEL.geometry_buffer.size();
    stream << size;
    LOG_INFO("Geometry data size: {}", size);

    // Write meshes
    stream.Write(OUT_MODEL.meshes.data(), OUT_MODEL.meshes.size() * sizeof(SubMesh));

    // Write meshes instances
    stream.Write(OUT_MODEL.mesh_instances.data(), OUT_MODEL.mesh_instances.size() * sizeof(SubMeshInstance));

    // Write geometry buffer
    stream.Write(OUT_MODEL.geometry_buffer.data(), OUT_MODEL.geometry_buffer.size());
}

static void WriteModel() noexcept
{
    auto filename = (ASSET_PATH / OUTPUT_MODEL_PATH / MODEL_NAME).replace_extension("bemodel");
    LOG_INFO("Mesh output filename: {}", filename.string());

    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::binary);
    FileOutputStream stream{file};

    // Write mesh name
    AssetName mesh_name{(OUTPUT_MESH_PATH / MODEL_NAME).replace_extension("bemesh").string().c_str()};
    stream << mesh_name;

    // Write textures count
    uint32_t size = (uint32_t)OUT_MODEL.textures.size();
    stream << size;
    LOG_INFO("Textures count: {}", size);

    // Write materials count
    size = (uint32_t)OUT_MODEL.materials.size();
    stream << size;
    LOG_INFO("Materials count: {}", size);

    // Write mesh materials count
    size = (uint32_t)OUT_MODEL.mesh_materials.size();
    stream << size;
    LOG_INFO("Mesh materials count: {}", size);

    // Write textures names
    stream.Write(OUT_MODEL.textures.data(), OUT_MODEL.textures.size() * sizeof(AssetName));

    // Write materials
    for (const auto &m : OUT_MODEL.materials)
    {
        // Write hash of render group name
        stream << m.render_group;
        LOG_INFO("Material group name: {}", m.render_group.data());

        // Write properties count
        stream << (uint32_t)m.properties.size();
        LOG_INFO("Material properties count: {}", size);

        // Write properties
        stream.Write(m.properties.data(), size * sizeof(MaterialProperty));
    }

    // Write mesh materials
    stream.Write(OUT_MODEL.mesh_materials.data(), OUT_MODEL.mesh_materials.size() * sizeof(uint32_t));
}

static void Convert() noexcept
{
    cgltf_options options{};

    auto result = cgltf_parse_file(&options, GLTF_MODEL_PATH.string().c_str(), &GLTF);
    VERIFY(result == cgltf_result_success, "Failed to load GLTF mesh '{}'.", GLTF_MODEL_PATH.string());

    result = cgltf_load_buffers(&options, GLTF, GLTF_MODEL_PATH.string().c_str());
    VERIFY(result == cgltf_result_success, "Failed to load GLTF buffers '{}'", GLTF_MODEL_PATH.string());

    for (uint32_t i = 0; i < GLTF->extensions_used_count; ++i)
    {
        GLTF_EXTENSTIONS.push_back(ConstHashOf(GLTF->extensions_used[i]));
    }

    ProcessMaterials();
    ProcessMeshes();

    cgltf_free(GLTF);

    const auto buf_size = GenerateMeshlets();
    GenerateOutputModel(buf_size);
    WriteMesh();
    WriteModel();
}

int main(int argc, char *argv[])
{
    if (argc < 6)
    {
        LOG_ERROR("GLTF file path required.");
        return 0;
    }

    basisu::basisu_encoder_init();

    Path gltf_path{argv[1]};
    Path asset_path{argv[2]};
    Path tex_output_path{argv[3]};
    Path mesh_output_path{argv[4]};
    Path model_output_path{argv[5]};

    LOG_INFO("GLTF file path: {}", gltf_path.string());
    LOG_INFO("Output asset path: {}", asset_path.string());
    LOG_INFO("Output texture path: {}", tex_output_path.string());
    LOG_INFO("Output mesh path: {}", mesh_output_path.string());
    LOG_INFO("Output model path: {}", model_output_path.string());

    GLTF_MODEL_PATH = gltf_path.string();
    MODEL_NAME = StringUtils::ToLower(gltf_path.filename().stem().string());

    ASSET_PATH = asset_path;
    OUTPUT_TEX_PATH = tex_output_path;
    OUTPUT_MESH_PATH = mesh_output_path;
    OUTPUT_MODEL_PATH = model_output_path;

    Convert();

    return 0;
}

// G:/projects/gamedev/engines/WickedEngine/Content/models/DamagedHelmet.glb
// G:/projects/gamedev/be/assets/models