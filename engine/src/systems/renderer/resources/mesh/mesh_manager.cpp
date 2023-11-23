#include "systems/renderer/renderer.h"

namespace Be::System::Renderer
{

    MeshManager::MeshManager(RhiDriver &driver) noexcept
        : m_driver{driver}
    {
    }

    MeshHandle MeshManager::Load(const String &key, InputStream &stream, EMeshManagerFlag flags) noexcept
    {
        PROFILER_SCOPE;

        uint32_t submeshes_count{0};
        uint32_t instances_count{0};
        uint32_t geometry_size{0};

        stream >> submeshes_count >> instances_count >> geometry_size;

        MeshHandle mesh{new Mesh()};

        mesh->m_submeshes.resize(submeshes_count);
        stream.Read(mesh->m_submeshes.data(), submeshes_count * sizeof(SubMesh));

        mesh->m_instances.resize(instances_count);
        stream.Read(mesh->m_instances.data(), instances_count * sizeof(SubMeshInstance));

        ByteArray geometry{};
        geometry.resize(geometry_size);
        stream.Read(geometry.data(), geometry_size);

        RhiBufferDesc buffer_desc{
            .bind_flag = ERhiBindFlag::eUnorderedAccess | ERhiBindFlag::eCopyDest,
            .size = geometry_size,
        };
        auto geometry_buffer = m_driver.CreateBuffer(buffer_desc);

        RhiBufferViewDesc view_desc{
            .buffer = geometry_buffer,
            .view_type = vk::DescriptorType::eStorageBuffer,
            .offset = 0,
            .size = geometry_size,
        };
        mesh->m_geometry = m_driver.CreateBufferView(view_desc);

        return mesh;
    }

    MeshHandle MeshManager::Load(const String &key, const Path &path, EMeshManagerFlag flags) noexcept
    {
        PROFILER_SCOPE;

        std::ifstream file{path, std::ios::in | std::ios::binary};
        VERIFY(file, "Failed to open mesh file: {}", path.string());

        FileInputStream stream{file};
        return Load(key, stream, flags);
    }

}