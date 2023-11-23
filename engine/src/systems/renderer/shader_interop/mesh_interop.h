#ifndef MESH_INTEROP_H
#define MESH_INTEROP_H

#define MESHLET_MAX_TRIANGLES 124
#define MESHLET_MAX_VERTICES  64

#define MESH_HAS_POSITION     1
#define MESH_HAS_NORMAL       (1 << 1)
#define MESH_HAS_TANGENT      (1 << 2)
#define MESH_HAS_UV           (1 << 3)
#define MESH_HAS_VERTEX_COLOR (1 << 4)
#define MESH_HAS_BONE_INDEX   (1 << 5)
#define MESH_HAS_BONE_WEIGHTS (1 << 6)
 
#ifdef __cplusplus
namespace Be::System::Renderer::ShaderInterop
{
#endif 

struct MeshData
{
	uint32_t buffer_index;
	
	uint32_t positions_offset;
	uint32_t normals_offset;
	uint32_t uvs_offset;
	uint32_t colors_offset;
	
	uint32_t indices_offset;
	uint32_t index_byte_size;

	uint32_t meshlet_offset;
	uint32_t meshlet_vertex_offset;
	uint32_t meshlet_triangle_offset;
	uint32_t meshlet_bounds_offset;
	uint32_t meshlet_count;

	uint32_t flags;
};

struct Meshlet
{
	uint32_t vertex_offset;
	uint32_t triangle_offset;
	uint32_t vertex_count;
	uint32_t triangle_count;
};

#ifdef __cplusplus
struct MeshletTriangle
{
    uint32_t v0 : 10;
 	uint32_t v1 : 10;
 	uint32_t v2 : 10;
 	uint32_t pad : 2;
};
#else
#define MeshletTriangle uint32_t
#endif

struct MeshletBounds
{
	vec3 center;
	vec3 extents;
};

#ifdef __cplusplus
}
#endif

#endif // MESH_INTEROP_H