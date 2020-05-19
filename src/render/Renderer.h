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
#include "objects/VulkanCommandBuffers.h"
#include "memory/DeviceMemoryManager.h"
#include "resources/VulkanImage.h"
#include "objects/VulkanDescriptorPools.h"

using namespace VULKAN_HPP_NAMESPACE;

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

class PerFrameData;
class ZPrepass;
class GBufferPass;
class DeferredLightingPass;
class PostProcessPass;

//=======================================================================================================
//=======================================================================================================

class Renderer
{
public:
	Renderer();
	virtual ~Renderer();

	void Init();
	void RenderFrame();
	void WaitForDevice();
	void Cleanup();

	void SetResolution(int InWidth, int InHeight);
	int GetWidth() const;
	int GetHeight() const;

	VulkanDevice& GetVulkanDevice();
	Device& GetDevice();
	VulkanSwapChain& GetSwapChain();
	VulkanCommandBuffers& GetCommandBuffers();
	VulkanDescriptorPools& GetDescriptorPools();
	Queue GetGraphicsQueue();

	PerFrameData* GetPerFrameData() { return perFrameData; }
	GBufferPass* GetGBufferPass() { return gBufferPass; }
	DeferredLightingPass* GetDeferredLightingPass() { return deferredLightingPass; }
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
	VulkanCommandBuffers commandBuffers;
	VulkanDescriptorPools descriptorPools;
	Viewport viewport;

	PerFrameData* perFrameData;

	ZPrepass* zPrepass;
	GBufferPass* gBufferPass;
	DeferredLightingPass* deferredLightingPass;
	PostProcessPass* postProcessPass;

	//==================== METHODS ===============================

	void TransferResources(CommandBuffer& inCmdBuffer, uint32_t inQueueFamilyIndex);
	void OnResolutionChange();
};



