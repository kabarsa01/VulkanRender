#include "render/passes/RTShadowPass.h"
#include "vulkan/vulkan.hpp"
#include "../RtScene.h"
#include "utils/Singleton.h"
#include "utils/ResourceUtils.h"
#include "ZPrepass.h"
#include "GBufferPass.h"
#include "LightClusteringPass.h"
#include "../shader/ShaderResourceMapper.h"


namespace CGE
{

	namespace vk = VULKAN_HPP_NAMESPACE;

	RTShadowPass::RTShadowPass(HashString name)
		: VulkanPassBase(name)
	{
		m_subscriber.AddHandler<GlobalPreUpdateMessage>(this, &RTShadowPass::HandlePreUpdate);
	}

	RTShadowPass::~RTShadowPass()
	{

	}

	void RTShadowPass::RecordCommands(CommandBuffer* inCommandBuffer)
	{
	}

	void RTShadowPass::CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight)
	{
	}

	void RTShadowPass::CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight)
	{
	}

	Pipeline RTShadowPass::CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass)
	{
		VulkanDevice* device = GetVulkanDevice();
		vk::Device& nativeDevice = device->GetDevice();

		RtScene* rtScene = Singleton<RtScene>::GetInstance();

		vk::PipelineCacheCreateInfo pipelineCacheInfo;
		vk::PipelineCache pipelineCache = nativeDevice.createPipelineCache(pipelineCacheInfo);

		auto func = PFN_vkCreateDeferredOperationKHR(vkGetDeviceProcAddr(nativeDevice, "createDeferredOperationKHR"));
		vk::DeferredOperationKHR deferredOperation;

		//------------------------------------------------------------------
		// shader stages and groups

		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

		// raygen group
		vk::RayTracingShaderGroupCreateInfoKHR groupInfo;
		groupInfo.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral);
		groupInfo.setAnyHitShader(VK_SHADER_UNUSED_KHR);
		groupInfo.setClosestHitShader(VK_SHADER_UNUSED_KHR);
		groupInfo.setIntersectionShader(VK_SHADER_UNUSED_KHR);
		groupInfo.setGeneralShader(static_cast<uint32_t>(shaderStages.size()));
		shaderGroups.push_back(groupInfo);

		vk::PipelineShaderStageCreateInfo shaderStageInfo;
		shaderStageInfo.setStage(vk::ShaderStageFlagBits::eRaygenKHR);
		shaderStageInfo.setFlags(vk::PipelineShaderStageCreateFlags{});
		shaderStageInfo.setPName("main"); // temp name
		//shaderStageInfo.setModule()

		//------------------------------------------------------------------
		// get descriptor sets from shaders
		std::vector<VulkanDescriptorSet>& sets = m_shaderResourceMapper.GetDescriptorSets();
		std::vector<vk::DescriptorSetLayout> descLayouts;
		descLayouts.resize(sets.size());
		for (uint32_t idx = 0; idx < descLayouts.size(); idx++)
		{
			descLayouts[idx] = sets[idx].GetLayout();
		}
		//------------------------------------------------------------------
		// pipeline layout info
		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo.setSetLayoutCount(descLayouts.size());
		layoutInfo.setPSetLayouts(descLayouts.data());
		vk::PipelineLayout pipelineLayout = nativeDevice.createPipelineLayout(layoutInfo);
		//------------------------------------------------------------------
		// pipeline create info
		vk::RayTracingPipelineCreateInfoKHR pipelineInfo;
		pipelineInfo.setGroupCount(static_cast<uint32_t>(rtScene->GetShaderGroups().size()));
		pipelineInfo.setPGroups(rtScene->GetShaderGroups().data());
		pipelineInfo.setStageCount(static_cast<uint32_t>(rtScene->GetShaderStages().size()));
		pipelineInfo.setPStages(rtScene->GetShaderStages().data());
		pipelineInfo.setFlags({});
		pipelineInfo.setLayout(pipelineLayout);

		auto result = nativeDevice.createRayTracingPipelineKHR(deferredOperation, pipelineCache, pipelineInfo);
		if (result.result != vk::Result::eSuccess)
		{
			return Pipeline();
		}

		return result.value;
	}

	RenderPass RTShadowPass::CreateRenderPass()
	{
		return RenderPass();
	}

	void RTShadowPass::OnCreate()
	{
		RtScene* rtScene = Singleton<RtScene>::GetInstance();
		Renderer* renderer = Engine::GetRendererInstance();
		VulkanDevice& device = renderer->GetVulkanDevice();

		int width = renderer->GetWidth();
		int height = renderer->GetHeight();

		m_visibilityBuffer1 = ResourceUtils::CreateImage2D(
			&device,
			width,
			height,
			vk::Format::eR8G8B8A8Unorm, 
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled);
		m_visibilityView1 = m_visibilityBuffer1.CreateView(ResourceUtils::CreateColorSubresRange(), ImageViewType::e2D);

		m_visibilityTex1 = ObjectBase::NewObject<Texture2D, const HashString&>("RtShadowsVisibilityTexture");
		m_visibilityTex1->CreateFromExternal(m_visibilityBuffer1, m_visibilityView1, true);

		ZPrepass* zPrepass = GetRenderer()->GetZPrepass();
		GBufferPass* gBufferPass = GetRenderer()->GetGBufferPass();
		LightClusteringPass* clusteringPass = GetRenderer()->GetLightClusteringPass();

		// obtain resources that were created by other passes
		m_normalsTex = ObjectBase::NewObject<Texture2D, const HashString&>("RtShadowsNormalTexture");
		m_normalsTex->CreateFromExternal(gBufferPass->GetAttachments()[1], gBufferPass->GetAttachmentViews()[1]);
		m_depthTex = ObjectBase::NewObject<Texture2D, const HashString&>("RtShadowsDepthTexture");
		m_depthTex->CreateFromExternal(zPrepass->GetDepthAttachment(), zPrepass->GetDepthAttachmentView(), false);
		m_clusterLightsData = clusteringPass->computeMaterial->GetStorageBuffer("clusterLightsData");
		m_clusterLightsData.SetCleanup(false);
		m_lightsList = clusteringPass->computeMaterial->GetUniformBuffer("lightsList");
		m_lightsList.SetCleanup(false);
		m_lightsIndices = clusteringPass->computeMaterial->GetUniformBuffer("lightsIndices");
		m_lightsIndices.SetCleanup(false);

		m_shaderResourceMapper.AddSampledImage("normalTex", m_normalsTex);
		m_shaderResourceMapper.AddSampledImage("depthTex", m_depthTex);
		m_shaderResourceMapper.AddStorageBuffer("clusterLightsData", m_clusterLightsData);
		m_shaderResourceMapper.AddUniformBuffer("lightsList", m_lightsList);
		m_shaderResourceMapper.AddUniformBuffer("lightsIndices", m_lightsIndices);
		// visibility image
		m_shaderResourceMapper.AddStorageImage("visibilityTex1", m_visibilityTex1);
		m_shaderResourceMapper.AddAccelerationStructure("tlas", rtScene->GetTlas().accelerationStructure);
	}

	void RTShadowPass::OnDestroy()
	{
		// TODO cleanup
	}

	void RTShadowPass::HandlePreUpdate(std::shared_ptr<GlobalPreUpdateMessage> msg)
	{

	}

	void RTShadowPass::UpdateShaderResources()
	{
		Renderer* renderer = Engine::GetRendererInstance();
		VulkanDevice& device = renderer->GetVulkanDevice();
		RtScene* rtScene = Singleton<RtScene>::GetInstance();

		std::vector<RtShaderPtr>& shaders = rtScene->GetShaders();
		m_shaderResourceMapper.SetShaders(shaders);

		m_shaderResourceMapper.Update();
	}

}

