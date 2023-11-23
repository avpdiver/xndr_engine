#include "systems/renderer/renderer.h"

namespace Be::System::Renderer
{

    ModelManager::ModelManager(RhiDriver &driver, TextureManager &texture_manager, MeshManager &mesh_manager) noexcept
        : m_driver{driver},
          m_texture_manager{texture_manager},
          m_mesh_manager{mesh_manager}
    {
    }

    ModelHandle ModelManager::Load(const String &key, InputStream &stream, EModelManagerFlag flags) noexcept
    {
        PROFILER_SCOPE;

        AssetName mesh_name{};
        uint32_t textures_count{0};
        uint32_t materials_count{0};
        uint32_t mesh_materials_count{0};

        stream >> textures_count >> materials_count >> mesh_materials_count;

        auto model = MakeRefCounter<Model>();

        Array<AssetName> textures_names;
        textures_names.resize(textures_count);
        model->m_textures.reserve(textures_count);
        stream.Read(textures_names.data(), textures_count * sizeof(AssetName));

        for (uint32_t i = 0; i < textures_count; i++)
        {
            auto t = textures_names.at(i).c_str();
            model->m_textures.push_back(m_texture_manager.Load(t, t));
        }

        Array<MaterialBlueprint> material_blueprints;
        material_blueprints.reserve(materials_count);
        for (uint32_t i = 0; i < materials_count; i++)
        {
            auto &m = material_blueprints.emplace_back();
            uint32_t prop_count{0};
            stream >> m.render_group >> prop_count;

            m.properties.resize(prop_count);
            stream.Read(m.properties.data(), prop_count * sizeof(MaterialProperty));
        }

        model->m_submesh_materials.resize(mesh_materials_count);
        stream.Read(model->m_submesh_materials.data(), mesh_materials_count);

        return model;
    }

    ModelHandle ModelManager::Load(const String &key, const Path &path, EModelManagerFlag flags) noexcept
    {
        PROFILER_SCOPE;

        std::ifstream file{path, std::ios::in | std::ios::binary};
        VERIFY(file, "Failed to open model file: {}", path.string());

        FileInputStream stream{file};
        return Load(key, stream, flags);
    }

}