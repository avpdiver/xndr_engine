#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{

    static const Map<vk::ShaderStageFlagBits, uint8_t> SHADER_STAGE_INDECIES{
        std::make_pair(vk::ShaderStageFlagBits::eVertex, 0),
        std::make_pair(vk::ShaderStageFlagBits::eFragment, 1),
        std::make_pair(vk::ShaderStageFlagBits::eCompute, 2),
        std::make_pair(vk::ShaderStageFlagBits::eTaskEXT, 3),
        std::make_pair(vk::ShaderStageFlagBits::eMeshEXT, 4),
    };

    RhiProgram::RhiProgram(RhiDriver &driver, const Array<RhiShaderHandle> &shaders) noexcept
        : RhiResource{driver},
          m_pipeline_bind_point{vk::PipelineBindPoint::eGraphics}
    {
        PROFILER_SCOPE;

        CreateDescriptorSets(shaders);
        CreateShaders(shaders);
    }

    RhiProgram::~RhiProgram() noexcept
    {
        PROFILER_SCOPE;

        if (m_pipeline_layout)
        {
            m_driver.Delete(m_pipeline_layout);
        }

        for (const auto &v : m_descriptors)
        {
            m_driver.Delete(v.pool);
        }

        for (const auto &v : m_descriptor_set_layouts)
        {
            m_driver.Delete(v);
        }

        for (const auto &v : m_shaders)
        {
            if (!v)
            {
                continue;
            }
            m_driver.Delete(v);
        }
    }

    void RhiProgram::CreateDescriptorSets(const Array<RhiShaderHandle> &shaders) noexcept
    {
        PROFILER_SCOPE;

        ZeroMem(m_bindings);
        ZeroMem(m_binding_masks);

        for (const auto &shader : shaders)
        {
            const auto &blob = shader->GetSpirv();
            m_hash += HashOf(blob.data(), sizeof(uint32_t) * blob.size());

            for (const auto &sb : shader->GetBindings())
            {
                auto &b = m_bindings[sb.set][sb.binding];
                m_binding_names[RhiProgramBindingName{sb.name}] = {sb.set, sb.binding};

                b.shader_stage |= shader->GetStage();
                b.descriptor_type = sb.type;
                b.set = sb.set;
                b.binding = sb.binding;
                b.count = sb.count;

                m_binding_masks[sb.set] |= (1u << sb.binding);
            }

            if (shader->GetStage() == vk::ShaderStageFlagBits::eCompute)
            {
                m_pipeline_bind_point = vk::PipelineBindPoint::eCompute;
            }
        }

        for (uint8_t set = 0; set < MAX_DESCRIPTOR_SETS; set++)
        {
            const auto binding_mask = m_binding_masks[set];
            if (binding_mask == 0)
            {
                continue;
            }

            bool is_bindless{false};
            vk::DescriptorType bindless_type;
            Map<vk::DescriptorType, uint32_t> descriptor_count;

            Array<vk::DescriptorSetLayoutBinding> vk_bindings;
            Array<vk::DescriptorBindingFlags> vk_bindings_flags;

            ForEachBit(
                binding_mask,
                [&](uint32_t binding)
                {
                    const auto &b = m_bindings[set][binding];
                    auto &v = vk_bindings.emplace_back();

                    v.binding = b.binding;
                    v.descriptorType = b.descriptor_type;
                    v.descriptorCount = b.count;
                    v.stageFlags = b.shader_stage;

                    auto &f = vk_bindings_flags.emplace_back();
                    if (b.count == MaxValue)
                    {
                        v.descriptorCount = MAX_BINDLESS_HEAP_SIZE;
                        v.stageFlags = vk::ShaderStageFlagBits::eAll;

                        is_bindless = true;
                        bindless_type = b.descriptor_type;

                        f = vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
                    }
                });

            vk::DescriptorSetLayoutBindingFlagsCreateInfo layout_flags_info{
                .bindingCount = uint32_t(vk_bindings_flags.size()),
                .pBindingFlags = vk_bindings_flags.data(),
            };

            vk::DescriptorSetLayoutCreateInfo layout_info{
                .pNext = &layout_flags_info,
                .bindingCount = uint32_t(vk_bindings.size()),
                .pBindings = vk_bindings.data(),
            };

            auto ds_layout = m_device.VkHandle().createDescriptorSetLayout(layout_info);
            m_descriptor_set_layouts.emplace_back(ds_layout);

            for (const auto &binding : vk_bindings)
            {
                descriptor_count[binding.descriptorType] += binding.descriptorCount;
            }

            if (is_bindless)
            {
                m_descriptor_sets.emplace_back(m_driver.GetBindlessDescriptorPool(bindless_type).GetDescriptorSet());
            }
            else
            {
                auto &set_pool = m_descriptors.emplace_back(m_driver.AllocateDescriptorSet(ds_layout, descriptor_count));
                m_descriptor_sets.emplace_back(set_pool.set);
            }
        }

        vk::PipelineLayoutCreateInfo pipeline_layout_info{
            .setLayoutCount = uint32_t(m_descriptor_set_layouts.size()),
            .pSetLayouts = m_descriptor_set_layouts.data(),
        };
        m_pipeline_layout = m_device.VkHandle().createPipelineLayout(pipeline_layout_info);
    }

    void RhiProgram::CreateShaders(const Array<RhiShaderHandle> &shaders) noexcept
    {
        PROFILER_SCOPE;

        Array<vk::ShaderCreateInfoEXT> cis;
        cis.reserve(shaders.size());

        for (uint32_t i = 0; i < shaders.size(); i++)
        {
            const auto &shader = shaders[i];
            const auto &blob = shader->GetSpirv();

            auto &ci = cis.emplace_back();

            ci.flags = vk::ShaderCreateFlagBitsEXT::eLinkStage;
            ci.stage = shader->GetStage();
            ci.codeType = vk::ShaderCodeTypeEXT::eSpirv;
            ci.codeSize = sizeof(uint32_t) * blob.size();
            ci.pCode = blob.data();
            ci.pName = shader->GetEntryPoints()[0].name.c_str();
            ci.setLayoutCount = static_cast<uint32_t>(m_descriptor_set_layouts.size());
            ci.pSetLayouts = m_descriptor_set_layouts.data();
            ci.pushConstantRangeCount = 0;
            ci.pPushConstantRanges = nullptr;
            ci.pSpecializationInfo = nullptr;

            if (i < shaders.size() - 1)
            {
                ci.nextStage = shaders[i + 1]->GetStage();
            }
        }

        Array<vk::ShaderEXT> shaders_ext;
        shaders_ext.resize(shaders.size());

        VK_VERIFY(m_device.VkHandle().createShadersEXT(static_cast<uint32_t>(cis.size()), cis.data(), nullptr, shaders_ext.data()),
                  "Failed to create shaders");

        m_shaders.resize(SHADER_STAGES.size());
        for (uint32_t i = 0; i < cis.size(); i++)
        {
            const auto index = SHADER_STAGE_INDECIES.at(cis.at(i).stage);
            m_shaders[index] = shaders_ext[i];
        }
    }

    void RhiProgram::WriteBinding(const RhiWriteBindingDesc &binding) const noexcept
    {
        PROFILER_SCOPE;

        WriteBindings({binding});
    }

    void RhiProgram::WriteBindings(const Array<RhiWriteBindingDesc> &bindings) const noexcept
    {
        PROFILER_SCOPE;

        Array<vk::WriteDescriptorSet> writes;
        for (const auto &b : bindings)
        {
            const auto &s = b.view->GetWriteDescriptorSet();
            if (!(s.pImageInfo || s.pBufferInfo || s.pTexelBufferView || s.pNext))
            {
                continue;
            }

            auto &w = writes.emplace_back();
            w = s;

            w.descriptorType = b.binding.descriptor_type;
            w.dstSet = m_descriptor_sets[b.binding.set];
            w.dstBinding = b.binding.binding;
            w.dstArrayElement = 0;
            w.descriptorCount = 1;
        }

        if (!writes.empty())
        {
            m_device.VkHandle().updateDescriptorSets(uint32_t(writes.size()), writes.data(), 0, nullptr);
        }
    }

    void RhiProgram::GetBindingNames(OUT Set<RhiProgramBindingName>& names) const noexcept
    {
        PROFILER_SCOPE;
        
        for (const auto &[n, p] : m_binding_names)
        {
            names.emplace(n);
        }
    }

}
