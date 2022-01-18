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
#include "data/TextureData.h"


// pre-build batch to compile all our shaders
//
//call:treeProcess
//goto : eof
//
//	: treeProcess
//	rem Do whatever you want here over the files of this subdir, for example :
//	for%% f in(*.vert * .tesc * .tese * .geom * .frag * .comp * .rgen * .rmiss * .rchit) do glslangValidator --target - env vulkan1.2 - V % %f - o % %~nf.spv
//		for / D % %d in(*) do (
//			cd % %d
//			call : treeProcess
//			cd ..
//			)
//			exit / b

//-------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------
namespace CGE
{

	using VULKAN_HPP_NAMESPACE::Viewport;
	
	class PerFrameData;
	class ClusterComputePass;
	class ZPrepass;
	class GBufferPass;
	class RTShadowPass;
	class RTGIPass;
	class DeferredLightingPass;
	class LightCompositingPass;
	class PostProcessPass;

	class DepthPrepass;
	
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
		RTShadowPass* GetRTShadowPass() { return rtShadowPass; }
		DeferredLightingPass* GetDeferredLightingPass() { return deferredLightingPass; }
	protected:
	private:
		//======================= VARS ===============================
		uint32_t version;
		int width = 1920;
		int height = 1080;
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
	
		//////////////////////////////////////////////////////////////////////

		DepthPrepass* m_depthPrepass;
		ClusterComputePass* m_clusterComputePass;
		GBufferPass* gBufferPass;
		RTShadowPass* rtShadowPass;
		DeferredLightingPass* deferredLightingPass;
		RTGIPass* rtGIPass;
		LightCompositingPass* compositingPass;
		PostProcessPass* postProcessPass;
	
		//==================== METHODS ===============================
	
		void TransferResources(CommandBuffer& inCmdBuffer, uint32_t inQueueFamilyIndex);
		void GenerateMips(CommandBuffer& inCmdBuffer, std::vector<TextureDataPtr>& inImages);
		void OnResolutionChange();
	};
	
}
