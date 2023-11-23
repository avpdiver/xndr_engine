#include "frameworks/rhi/rhi.h"

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Be::Framework::RHI
{

    vk::ShaderStageFlagBits ConvertExecutionModel(spv::ExecutionModel execution_model)
    {
        PROFILER_SCOPE;

        switch (execution_model)
        {
        case spv::ExecutionModel::ExecutionModelVertex:
            return vk::ShaderStageFlagBits::eVertex;
        case spv::ExecutionModel::ExecutionModelFragment:
            return vk::ShaderStageFlagBits::eFragment;
        case spv::ExecutionModel::ExecutionModelGeometry:
            return vk::ShaderStageFlagBits::eGeometry;
        case spv::ExecutionModel::ExecutionModelGLCompute:
            return vk::ShaderStageFlagBits::eCompute;
        case spv::ExecutionModel::ExecutionModelRayGenerationKHR:
            return vk::ShaderStageFlagBits::eRaygenKHR;
        case spv::ExecutionModel::ExecutionModelIntersectionKHR:
            return vk::ShaderStageFlagBits::eIntersectionKHR;
        case spv::ExecutionModel::ExecutionModelAnyHitKHR:
            return vk::ShaderStageFlagBits::eAnyHitKHR;
        case spv::ExecutionModel::ExecutionModelClosestHitKHR:
            return vk::ShaderStageFlagBits::eClosestHitKHR;
        case spv::ExecutionModel::ExecutionModelMissKHR:
            return vk::ShaderStageFlagBits::eMissKHR;
        case spv::ExecutionModel::ExecutionModelCallableKHR:
            return vk::ShaderStageFlagBits::eCallableKHR;
        case spv::ExecutionModel::ExecutionModelTaskEXT:
            return vk::ShaderStageFlagBits::eTaskEXT;
        case spv::ExecutionModel::ExecutionModelMeshEXT:
            return vk::ShaderStageFlagBits::eMeshEXT;
        default:
            FATAL("Invalid SPIRV execution model.");
            return {};
        }
    }

    Array<RhiShaderInputParameterDesc> ParseInputParameters(const spirv_cross::CompilerGLSL &compiler)
    {
        PROFILER_SCOPE;

        auto resources = compiler.get_shader_resources();
        Array<RhiShaderInputParameterDesc> input_parameters;

        for (const auto &resource : resources.stage_inputs)
        {
            auto &input = input_parameters.emplace_back();

            input.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            input.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
            input.semantic_name = resource.name;

            if (!input.semantic_name.empty() && input.semantic_name.back() == '0')
            {
                input.semantic_name.pop_back();
            }

            auto type = compiler.get_type(resource.base_type_id);
            if (type.basetype == spirv_cross::SPIRType::Float)
            {
                if (type.vecsize == 1)
                {
                    input.format = vk::Format::eR32Sfloat;
                }
                else if (type.vecsize == 2)
                {
                    input.format = vk::Format::eR32G32Sfloat;
                }
                else if (type.vecsize == 3)
                {
                    input.format = vk::Format::eR32G32B32Sfloat;
                }
                else if (type.vecsize == 4)
                {
                    input.format = vk::Format::eR32G32B32A32Sfloat;
                }
            }
            else if (type.basetype == spirv_cross::SPIRType::UInt)
            {
                if (type.vecsize == 1)
                {
                    input.format = vk::Format::eR32Uint;
                }
                else if (type.vecsize == 2)
                {
                    input.format = vk::Format::eR32G32Uint;
                }
                else if (type.vecsize == 3)
                {
                    input.format = vk::Format::eR32G32B32Uint;
                }
                else if (type.vecsize == 4)
                {
                    input.format = vk::Format::eR32G32B32A32Uint;
                }
            }
            else if (type.basetype == spirv_cross::SPIRType::Int)
            {
                if (type.vecsize == 1)
                {
                    input.format = vk::Format::eR32Sint;
                }
                else if (type.vecsize == 2)
                {
                    input.format = vk::Format::eR32G32Sint;
                }
                else if (type.vecsize == 3)
                {
                    input.format = vk::Format::eR32G32B32Sint;
                }
                else if (type.vecsize == 4)
                {
                    input.format = vk::Format::eR32G32B32A32Sint;
                }
            }
        }

        return input_parameters;
    }

    Array<RhiShaderOutputParameterDesc> ParseOutputParameters(const spirv_cross::CompilerGLSL &compiler)
    {
        PROFILER_SCOPE;

        auto resources = compiler.get_shader_resources();
        Array<RhiShaderOutputParameterDesc> output_parameters;
        for (const auto &resource : resources.stage_outputs)
        {
            auto &output = output_parameters.emplace_back();
            output.binding = compiler.get_decoration(resource.id, spv::DecorationLocation);
        }
        return output_parameters;
    }

    bool IsBufferDimension(spv::Dim dimension)
    {
        PROFILER_SCOPE;

        switch (dimension)
        {
        case spv::Dim::DimBuffer:
            return true;
        case spv::Dim::Dim1D:
        case spv::Dim::Dim2D:
        case spv::Dim::Dim3D:
        case spv::Dim::DimCube:
            return false;
        default:
            assert(false);
            return false;
        }
    }

    vk::DescriptorType GetViewType(const spirv_cross::CompilerGLSL &compiler, const spirv_cross::SPIRType &type, uint32_t resource_id)
    {
        PROFILER_SCOPE;

        switch (type.basetype)
        {
        case spirv_cross::SPIRType::AccelerationStructure:
        {
            return vk::DescriptorType::eAccelerationStructureKHR;
        }
        case spirv_cross::SPIRType::SampledImage:
        case spirv_cross::SPIRType::Image:
        {
            bool is_readonly = (type.image.sampled != 2);
            if (IsBufferDimension(type.image.dim))
            {
                if (is_readonly)
                {
                    return vk::DescriptorType::eUniformTexelBuffer;
                }
                else
                {
                    return vk::DescriptorType::eStorageTexelBuffer;
                }
            }
            else
            {
                if (is_readonly)
                {
                    return vk::DescriptorType::eCombinedImageSampler;
                }
                else
                {
                    return vk::DescriptorType::eStorageImage;
                }
            }
        }
        case spirv_cross::SPIRType::Sampler:
        {
            return vk::DescriptorType::eSampler;
        }
        case spirv_cross::SPIRType::Struct:
        {
            if (type.storage == spv::StorageClassStorageBuffer)
            {
                auto flags = compiler.get_buffer_block_flags(resource_id);                
                // TODO check is_readonly ???
                // bool is_readonly = flags.get(spv::DecorationNonWritable);
                return vk::DescriptorType::eStorageBuffer;                
            }
            else if (type.storage == spv::StorageClassPushConstant || type.storage == spv::StorageClassUniform)
            {
                return vk::DescriptorType::eUniformBuffer;
            }
            assert(false);
            return {};
        }
        default:
            assert(false);
            return {};
        }
    }

    vk::ImageViewType GetDimension(spv::Dim dim, const spirv_cross::SPIRType &resource_type)
    {
        PROFILER_SCOPE;

        switch (dim)
        {
        case spv::Dim::Dim1D:
        {
            if (resource_type.image.arrayed)
            {
                return vk::ImageViewType::e1DArray;
            }
            else
            {
                return vk::ImageViewType::e1D;
            }
        }
        case spv::Dim::Dim2D:
        {
            // TODO check multisampling ???
            // resource_type.image.ms

            if (resource_type.image.arrayed)
            {
                return vk::ImageViewType::e2DArray;
            }
            else
            {
                return vk::ImageViewType::e2D;
            }
        }
        case spv::Dim::Dim3D:
        {
            return vk::ImageViewType::e3D;
        }
        case spv::Dim::DimCube:
        {
            if (resource_type.image.arrayed)
            {
                return vk::ImageViewType::eCubeArray;
            }
            else
            {
                return vk::ImageViewType::eCube;
            }
        }
        case spv::Dim::DimBuffer:
        {
            return {};
        }
        default:
            FATAL("Invalid SPIRV dim.");
            return {};
        }
    }

    vk::ImageViewType GetViewDimension(const spirv_cross::SPIRType &resource_type)
    {
        PROFILER_SCOPE;

        if (resource_type.basetype == spirv_cross::SPIRType::BaseType::Image ||
            resource_type.basetype == spirv_cross::SPIRType::BaseType::SampledImage)
        {
            return GetDimension(resource_type.image.dim, resource_type);
        }
        else if (resource_type.basetype == spirv_cross::SPIRType::BaseType::Struct)
        {
            return {};
        }
        else
        {
            return {};
        }
    }

    RhiShaderBindingDesc GetBindingDesc(const spirv_cross::CompilerGLSL &compiler, const spirv_cross::Resource &resource)
    {
        PROFILER_SCOPE;

        RhiShaderBindingDesc desc{};

        auto type = compiler.get_type(resource.type_id);
        desc.name = compiler.get_name(resource.id);
        desc.type = GetViewType(compiler, type, resource.id);
        desc.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        desc.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        desc.count = 1;
        desc.dimension = GetViewDimension(type);

        if (!type.array.empty() && type.array.front() == 0)
        {
            desc.count = MaxValue;
        }

        switch (desc.type)
        {
        case vk::DescriptorType::eStorageBuffer:
        {
            bool is_block = compiler.get_decoration_bitset(type.self).get(spv::DecorationBlock) ||
                            compiler.get_decoration_bitset(type.self).get(spv::DecorationBufferBlock);
            bool is_sized_block = is_block && (compiler.get_storage_class(resource.id) == spv::StorageClassUniform ||
                                               compiler.get_storage_class(resource.id) == spv::StorageClassUniformConstant ||
                                               compiler.get_storage_class(resource.id) == spv::StorageClassStorageBuffer);
            assert(is_sized_block);

            auto base_type = compiler.get_type(resource.base_type_id);
            desc.structure_stride = uint32_t(compiler.get_declared_struct_size_runtime_array(base_type, 1) - compiler.get_declared_struct_size_runtime_array(base_type, 0));
            assert(desc.structure_stride);

            break;
        }
        default:
            break;
        }

        return desc;
    }

    void ParseBindings(const spirv_cross::CompilerGLSL &compiler, Array<RhiShaderBindingDesc> &bindings)
    {
        PROFILER_SCOPE;

        auto enumerate_resources = [&](const spirv_cross::SmallVector<spirv_cross::Resource> &res)
        {
            for (const auto &r : res)
            {
                bindings.emplace_back(GetBindingDesc(compiler, r));
            }
        };

        auto resources = compiler.get_shader_resources();

        enumerate_resources(resources.uniform_buffers);
        enumerate_resources(resources.storage_buffers);
        enumerate_resources(resources.sampled_images);
        enumerate_resources(resources.storage_images);
        enumerate_resources(resources.separate_images);
        enumerate_resources(resources.separate_samplers);
        enumerate_resources(resources.atomic_counters);
        enumerate_resources(resources.acceleration_structures);
    }

    RhiShaderReflection RhiShaderReflection::Create(const Array<uint32_t> &spirv) noexcept
    {
        PROFILER_SCOPE;

        return {spirv};
    }

    RhiShaderReflection::RhiShaderReflection(const Array<uint32_t> &spirv) noexcept
        : m_blob{spirv}
    {
        PROFILER_SCOPE;

        spirv_cross::CompilerGLSL compiler(m_blob);

        auto entry_points = compiler.get_entry_points_and_stages();
        for (const auto &entry_point : entry_points)
        {
            m_entry_points.push_back({entry_point.name.c_str(), ConvertExecutionModel(entry_point.execution_model)});
        }

        ParseBindings(compiler, m_bindings);

        m_input_parameters = ParseInputParameters(compiler);
        m_output_parameters = ParseOutputParameters(compiler);
    }

    const Array<RhiShaderEntryPoint> &RhiShaderReflection::GetEntryPoints() const noexcept
    {
        return m_entry_points;
    }

    const Array<RhiShaderBindingDesc> &RhiShaderReflection::GetBindings() const noexcept
    {
        return m_bindings;
    }

    const Array<RhiShaderInputParameterDesc> &RhiShaderReflection::GetInputParameters() const noexcept
    {
        return m_input_parameters;
    }

    const Array<RhiShaderOutputParameterDesc> &RhiShaderReflection::GetOutputParameters() const noexcept
    {
        return m_output_parameters;
    }

    const RhiShaderFeatureInfo &RhiShaderReflection::GetShaderFeatureInfo() const noexcept
    {
        return m_shader_feature_info;
    }

}
