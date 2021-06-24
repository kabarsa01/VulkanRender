#include "render/shader/Shader.h"
#include <fstream>
#include <streambuf>
#include "core/Engine.h"
#include "render/ShaderRegistry.h"
#include "render/Renderer.h"

namespace CGE
{
	namespace vk = VULKAN_HPP_NAMESPACE;

	Shader::Shader(const HashString& inPath)
		: Resource(inPath)
	{
		filePath = inPath.GetString();
		shaderModule = nullptr;
	}
	
	Shader::~Shader()
	{
		DestroyShaderModule();
	}
	
	bool Shader::Create()
	{
		if (shaderModule)
		{
			return true;
		}
	
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		binary.clear();
	
		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		binary.resize(size);
		file.seekg(0, std::ios::beg);
	
		file.read(binary.data(), size);
		file.close();
	
		ExtractBindingsInfo();
		CreateShaderModule();

		Engine::Get()->GetShaderRegistry()->AddShader(get_shared_from_this<Shader>());
		return true;
	}
	
	bool Shader::Destroy()
	{
		DestroyShaderModule();
		Engine::Get()->GetShaderRegistry()->RemoveShader(get_shared_from_this<Shader>());
		return true;
	}
	
	ShaderModule Shader::GetShaderModule()
	{
		return shaderModule;
	}
	
	void Shader::DestroyShaderModule()
	{
		if (shaderModule)
		{
			Engine::GetRendererInstance()->GetVulkanDevice().GetDevice().destroyShaderModule(shaderModule);
			shaderModule = nullptr;
		}
	}
	
	const std::vector<char>& Shader::GetCode() const
	{
		return binary;
	}
	
	std::vector<BindingInfo>& Shader::GetBindings(DescriptorType inDescriptorType)
	{
		return bindings[inDescriptorType];
	}
	
	void Shader::CreateShaderModule()
	{
		if (shaderModule)
		{
			return;
		}
	
		vk::ShaderModuleCreateInfo createInfo;
		// size in bytes, even though code is uint32_t*
		createInfo.setCodeSize(binary.size());
		createInfo.setPCode(reinterpret_cast<const uint32_t*>(binary.data()));
	
		shaderModule = Engine::GetRendererInstance()->GetVulkanDevice().GetDevice().createShaderModule(createInfo);
	}
	
	std::vector<BindingInfo> Shader::ExtractBindingInfo(
		SPIRV_CROSS_NAMESPACE::SmallVector<SPIRV_CROSS_NAMESPACE::Resource>& inResources, 
		SPIRV_CROSS_NAMESPACE::Compiler& inCompiler, 
		DescriptorType inDescriptorType)
	{
		std::vector<BindingInfo> bindingsVector;
	
		for (SPIRV_CROSS_NAMESPACE::Resource& resource : inResources)
		{
			BindingInfo info;
	
			info.set = inCompiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			info.binding = inCompiler.get_decoration(resource.id, spv::DecorationBinding);
			info.name = inCompiler.get_name(resource.id);
			info.blockName = resource.name;
			info.descriptorType = inDescriptorType;
	
			SPIRV_CROSS_NAMESPACE::SPIRType type = inCompiler.get_type(resource.type_id);
	
			info.vectorSize = type.vecsize;
			info.numColumns = type.columns;
	
			info.arrayDimensions.resize(type.array.size());
			for (uint64_t index = 0; index < type.array.size(); index++)
			{
				info.arrayDimensions[index] = type.array[index];
			}
			printf("Resource %s with block name %s at set = %u, binding = %u\n", info.name.GetString().c_str(), info.blockName.GetString().c_str(), info.set, info.binding);
	
			bindingsVector.push_back(info);
		}
		
		return bindingsVector;
	}
	
	void Shader::ExtractBindingsInfo()
	{
		SPIRV_CROSS_NAMESPACE::Compiler spirv(reinterpret_cast<const uint32_t*>(binary.data()), binary.size() / sizeof(uint32_t));
		SPIRV_CROSS_NAMESPACE::ShaderResources resources = spirv.get_shader_resources();
	
		bindings[DescriptorType::eUniformBuffer] = ExtractBindingInfo(resources.uniform_buffers, spirv, DescriptorType::eUniformBuffer);
		bindings[DescriptorType::eStorageBuffer] = ExtractBindingInfo(resources.storage_buffers, spirv, DescriptorType::eStorageBuffer);
		bindings[DescriptorType::eSampler] = ExtractBindingInfo(resources.separate_samplers, spirv, DescriptorType::eSampler);
		bindings[DescriptorType::eSampledImage] = ExtractBindingInfo(resources.separate_images, spirv, DescriptorType::eSampledImage);
		bindings[DescriptorType::eStorageImage] = ExtractBindingInfo(resources.storage_images, spirv, DescriptorType::eStorageImage);
		bindings[DescriptorType::eAccelerationStructureKHR] = ExtractBindingInfo(resources.acceleration_structures, spirv, DescriptorType::eAccelerationStructureKHR);
		// rejected sibling
		bindings[DescriptorType::eCombinedImageSampler] = ExtractBindingInfo(resources.sampled_images, spirv, DescriptorType::eCombinedImageSampler);
	}
	
}
