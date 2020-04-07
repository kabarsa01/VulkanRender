#include "VulkanShaderModule.h"
#include "Shader.h"
#include "core\Engine.h"
#include <stdexcept>

using namespace VULKAN_HPP_NAMESPACE;

VulkanShaderModule::VulkanShaderModule(const Shader& inShader)
	: ObjectBase()
{
	const std::vector<char>& code = inShader.GetCode();

	ShaderModuleCreateInfo createInfo;
	createInfo.setCodeSize(code.size());
	createInfo.setPCode( reinterpret_cast<const uint32_t*>( code.data() ) );

	shaderModule = Engine::GetRendererInstance()->GetDevice().createShaderModule(createInfo);
}

VulkanShaderModule::~VulkanShaderModule()
{
	Engine::GetRendererInstance()->GetDevice().destroyShaderModule(shaderModule);
}
