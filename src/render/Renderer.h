#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <optional>
#include <core/ObjectBase.h>
#include "glm/fwd.hpp"
#include <set>
#include "data/MeshData.h"
#include "objects/VulkanPhysicalDevice.h"
#include "objects/VulkanDevice.h"
#include "objects/VulkanSwapChain.h"

using namespace VULKAN_HPP_NAMESPACE;

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

// pre-build batch to compile all our shaders
//
//call :treeProcess
//goto : eof
//
//: treeProcess
//rem Do whatever you want here over the files of this subdir, for example :
//for %%f in(*.vert *.tesc *.tese *.geom *.frag *.comp) do glslangValidator - V % %f - o % %~nf.spv
//for / D % %d in(*) do (
//	cd %%d
//	call : treeProcess
//	cd ..
//)
//exit / b

//-------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------



//=======================================================================================================
//=======================================================================================================

class Renderer : public ObjectBase
{
public:
	Renderer();
	virtual ~Renderer();

	virtual void OnInitialize() override;

	void Init();
	void RenderFrame();
	void WaitForDevice();
	void Cleanup();

	void SetResolution(int InWidth, int InHeight);
	int GetWidth() const;
	int GetHeight() const;

	VulkanDevice& GetVulkanDevice();
	VulkanSwapChain& GetSwapChain();
	CommandPool GetCommandPool();
	Queue GetGraphicsQueue();
protected:
private:
	// TEMP
	VulkanBuffer uniformBuffer;
	//======================= VARS ===============================
	uint32_t version;
	int width = 1280;
	int height = 720;
	bool framebufferResized = false;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	VulkanDevice device;
	VulkanSwapChain swapChain;

	Viewport viewport;

	DescriptorSetLayout descriptorSetLayout;
	PipelineLayout pipelineLayout;
	Pipeline pipeline;

	CommandPool commandPool;
	std::vector<CommandBuffer> commandBuffers;
	DescriptorPool descriptorPool;
	std::vector<DescriptorSet> descriptorSets;

	//==================== METHODS ===============================

	void RecreateSwapChain();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateCommandPool();
	void CreateCommandBuffers();
	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();
	void UpdateUniformBuffer();
};

typedef std::shared_ptr<Renderer> RendererPtr;

