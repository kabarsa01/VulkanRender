#include "VulkanShaderModule.h"
#include "Shader.h"
#include "core\Engine.h"
#include <stdexcept>
#include "..\Renderer.h"

using namespace VULKAN_HPP_NAMESPACE;

VulkanShaderModule::VulkanShaderModule(const Shader& inShader)
	: ObjectBase()
{
	const std::vector<char>& code = inShader.GetCode();

	ShaderModuleCreateInfo createInfo;
	createInfo.setCodeSize(code.size());
	createInfo.setPCode( reinterpret_cast<const uint32_t*>( code.data() ) );

	shaderModule = Engine::GetRendererInstance()->GetVulkanDevice().GetDevice().createShaderModule(createInfo);
}

VulkanShaderModule::~VulkanShaderModule()
{
	Engine::GetRendererInstance()->GetVulkanDevice().GetDevice().destroyShaderModule(shaderModule);
}
