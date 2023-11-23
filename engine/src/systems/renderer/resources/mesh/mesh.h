#pragma once

namespace Be::System::Renderer
{

    BIT_ENUM(EMeshFlag, uint32_t,
             eNone = 0,
             ePosition = MESH_HAS_POSITION,
             eNormal = MESH_HAS_NORMAL,
             eTangent = MESH_HAS_TANGENT,
             eUV = MESH_HAS_UV,
             eVertexColor = MESH_HAS_VERTEX_COLOR,
             eBoneIndex = MESH_HAS_BONE_INDEX,
             eBoneWeights = MESH_HAS_BONE_WEIGHTS);

    struct SubMeshBufferView
    {
        uint32_t offset{0};
        uint32_t size{0};
        uint32_t elements{0};
    };

    struct SubMesh
    {
        EMeshFlag flags{EMeshFlag::eNone};
        vk::Format positions_format{};
        bool small_indices{false};

        SubMeshBufferView positions_view{};
        SubMeshBufferView normals_view{};
        SubMeshBufferView colors_view{};
        SubMeshBufferView uvs_view{};
        SubMeshBufferView indieces_view{};

        uint32_t meshlets_count{0};
        uint32_t meshlets_location{0};
        uint32_t meshlet_vertices_location{0};
        uint32_t meshlet_triangles_location{0};
        uint32_t meshlet_bounds_location{0};

        BBox bbox{};
    };

    struct SubMeshInstance
    {
        uint32_t mesh{MaxValue};
        Matrix4x4 transform{};
    };

    /*
    Geometry buffer structure (continuously repeated for each submesh):
       - Positions: UInt2(Rgba16Snorm)
       - Normals and Tangents: UInt2(Rgb10A2Snorm, Rgb10A2Snorm)
       - Colors: uint32_t(Rgba8Unorm)
       - UVs: uint32_t(Rg16Float)
       - Indecies: uint16_t|uint32_t
       - Meshlets: ShaderInterop::Meshlet
       - Meshlets vertices: uint32_t
       - Meshlets triangles: ShaderInterop::MeshletTriangle
       - Meshlets bounds: ShaderInterop::MeshletBounds
    */
    class Mesh final : public RefCounter
    {
    private:
        RhiBufferViewHandle m_geometry;

    private:
        Array<SubMesh> m_submeshes{};
        Array<SubMeshInstance> m_instances{};

        friend class MeshManager;
    };

    DEFINE_RENDERER_HANDLE(Mesh);

}

#include "systems/renderer/resources/mesh/mesh_manager.h"