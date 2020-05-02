#include "Material.h"
#include "DataManager.h"

Material::Material(HashString inId)
	: Resource(inId)
	, shaderHash(HashString::NONE)
{

}

Material::Material(HashString inId, const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath)
	: Resource(inId)
	, vertexShaderPath(inVertexShaderPath)
	, fragmentShaderPath(inFragmentShaderPath)
	, shaderHash(inVertexShaderPath + inFragmentShaderPath)
{

}

Material::~Material()
{

}

void Material::LoadResources()
{
	vertexShader = DataManager::RequestResourceType<Shader>(vertexShaderPath);
	fragmentShader = DataManager::RequestResourceType<Shader>(fragmentShaderPath);
}

HashString Material::GetShaderHash()
{
	return shaderHash;
}

void Material::SetShaderPath(const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath)
{
	vertexShaderPath = inVertexShaderPath;
	fragmentShaderPath = inFragmentShaderPath;
	shaderHash = HashString(vertexShaderPath + fragmentShaderPath);
}

void Material::SetTexture(const std::string& inName, Texture2DPtr inTexture2D)
{
	textures2D[inName] = inTexture2D;
}

bool Material::Load()
{
	return true;
}

bool Material::Cleanup()
{
	return true;
}

DescriptorSetLayout Material::ComposeDescriptorSetLayout()
{
	std::vector<DescriptorSetLayoutBinding> bindings;

	// TODO: BINDING_INFO and NAME should be paired somewhere here !!!!

	std::vector<BindingInfo> uniformBuffers = vertexShader->GetBindings(DescriptorType::eUniformBuffer);
	for (BindingInfo& info : uniformBuffers)
	{
		DescriptorSetLayoutBinding binding;
		binding.setBinding(info.binding);
		binding.setDescriptorType(DescriptorType::eUniformBuffer);
		binding.setDescriptorCount(info.IsArray() ? info.arrayDimensions[0] : 1);
		binding.setStageFlags(ShaderStageFlagBits::eAllGraphics);

		bindings.push_back(binding);
	}

	DescriptorSetLayoutBinding globalsLayoutBinding;
	globalsLayoutBinding.setBinding(0);
	globalsLayoutBinding.setDescriptorType(DescriptorType::eUniformBuffer);
	globalsLayoutBinding.setDescriptorCount(1);
	globalsLayoutBinding.setStageFlags(ShaderStageFlagBits::eAllGraphics);
	DescriptorSetLayoutBinding mvpLayoutBinding;
	mvpLayoutBinding.setBinding(2);
	mvpLayoutBinding.setDescriptorType(DescriptorType::eUniformBuffer);
	mvpLayoutBinding.setDescriptorCount(1);
	mvpLayoutBinding.setStageFlags(ShaderStageFlagBits::eAllGraphics);
	DescriptorSetLayoutBinding samplerLayoutBinding;
	samplerLayoutBinding.setBinding(1);
	samplerLayoutBinding.setDescriptorType(DescriptorType::eSampler);
	samplerLayoutBinding.setDescriptorCount(1);
	samplerLayoutBinding.setStageFlags(ShaderStageFlagBits::eFragment);
	DescriptorSetLayoutBinding diffuseLayoutBinding;
	diffuseLayoutBinding.setBinding(3);
	diffuseLayoutBinding.setDescriptorType(DescriptorType::eSampledImage);
	diffuseLayoutBinding.setDescriptorCount(1);
	diffuseLayoutBinding.setStageFlags(ShaderStageFlagBits::eFragment);

	DescriptorSetLayoutBinding setLayoutBindings[] = { globalsLayoutBinding, mvpLayoutBinding, samplerLayoutBinding, diffuseLayoutBinding };
	DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
	descriptorSetLayoutInfo.setBindingCount(4);
	descriptorSetLayoutInfo.setPBindings(setLayoutBindings);

	return DescriptorSetLayout();
	//descriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutInfo);
}

