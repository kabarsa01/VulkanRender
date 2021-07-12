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
		m_filePath = inPath.GetString();
		m_shaderModule = nullptr;
	}
	
	Shader::~Shader()
	{
		DestroyShaderModule();
	}
	
	bool Shader::Create()
	{
		if (m_shaderModule)
		{
			return true;
		}
	
		std::ifstream file(m_filePath, std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		m_binary.clear();
	
		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		m_binary.resize(size);
		file.seekg(0, std::ios::beg);
	
		file.read(m_binary.data(), size);
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
		return m_shaderModule;
	}
	
	void Shader::DestroyShaderModule()
	{
		if (m_shaderModule)
		{
			Engine::GetRendererInstance()->GetVulkanDevice().GetDevice().destroyShaderModule(m_shaderModule);
			m_shaderModule = nullptr;
		}
	}
	
	const std::vector<char>& Shader::GetCode() const
	{
		return m_binary;
	}
	
	std::vector<BindingInfo>& Shader::GetAllBindings()
	{
		return m_bindings;
	}

	std::vector<BindingInfo>& Shader::GetBindingsTypes(DescriptorType inDescriptorType)
	{
		return m_bindingsTypes[inDescriptorType];
	}
	
	bool Shader::HasBinding(HashString name)
	{
		return m_bindingsNames.find(name) != m_bindingsNames.end();
	}

	BindingInfo Shader::GetBindingSafe(HashString name)
	{
		if (m_bindingsNames.find(name) != m_bindingsNames.end())
		{
			return m_bindingsNames[name];
		}
		return BindingInfo();
	}

	BindingInfo& Shader::GetBinding(HashString name)
	{
		return m_bindingsNames[name];
	}

	void Shader::CreateShaderModule()
	{
		if (m_shaderModule)
		{
			return;
		}
	
		vk::ShaderModuleCreateInfo createInfo;
		// size in bytes, even though code is uint32_t*
		createInfo.setCodeSize(m_binary.size());
		createInfo.setPCode(reinterpret_cast<const uint32_t*>(m_binary.data()));
	
		m_shaderModule = Engine::GetRendererInstance()->GetVulkanDevice().GetDevice().createShaderModule(createInfo);
	}
	
	void Shader::ExtractBindingInfo(
		SPIRV_CROSS_NAMESPACE::SmallVector<SPIRV_CROSS_NAMESPACE::Resource>& inResources, 
		SPIRV_CROSS_NAMESPACE::Compiler& inCompiler, 
		DescriptorType inDescriptorType)
	{	
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
	
			// populate internal structures
			m_bindings.push_back(info);
			m_bindingsTypes[inDescriptorType].push_back(info);
			m_bindingsNames[info.name] = info;
		}
	}
	
	void Shader::ExtractBindingsInfo()
	{
		SPIRV_CROSS_NAMESPACE::Compiler spirv(reinterpret_cast<const uint32_t*>(m_binary.data()), m_binary.size() / sizeof(uint32_t));
		SPIRV_CROSS_NAMESPACE::ShaderResources resources = spirv.get_shader_resources();
	
		ExtractBindingInfo(resources.uniform_buffers, spirv, DescriptorType::eUniformBuffer);
		ExtractBindingInfo(resources.storage_buffers, spirv, DescriptorType::eStorageBuffer);
		ExtractBindingInfo(resources.separate_samplers, spirv, DescriptorType::eSampler);
		ExtractBindingInfo(resources.separate_images, spirv, DescriptorType::eSampledImage);
		ExtractBindingInfo(resources.storage_images, spirv, DescriptorType::eStorageImage);
		ExtractBindingInfo(resources.acceleration_structures, spirv, DescriptorType::eAccelerationStructureKHR);
		// rejected sibling
		ExtractBindingInfo(resources.sampled_images, spirv, DescriptorType::eCombinedImageSampler);
	}
	
}
