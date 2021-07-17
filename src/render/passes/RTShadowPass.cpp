#include "render/passes/RTShadowPass.h"
#include "vulkan/vulkan.hpp"
#include "../RtScene.h"
#include "utils/Singleton.h"
#include "utils/ResourceUtils.h"
#include "ZPrepass.h"
#include "GBufferPass.h"
#include "LightClusteringPass.h"
#include "../shader/ShaderResourceMapper.h"
#include "../PerFrameData.h"
#include "../objects/VulkanDevice.h"
#include "../objects/VulkanPhysicalDevice.h"
#include "data/DataManager.h"


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
		RtScene* rtScene = Singleton<RtScene>::GetInstance();

		inCommandBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_rtPipeline);
		inCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_rtPipelineLayout, 0, m_sets.size(), m_sets.data(), 0, nullptr);

		vk::StridedDeviceAddressRegionKHR rayGenRegion;
		rayGenRegion.setDeviceAddress(m_sbtBuffer.GetDeviceAddress());
		rayGenRegion.setSize(m_handleSizeAligned * rtScene->GetRayGenGroupsSize());
		rayGenRegion.setStride(m_handleSizeAligned);

		vk::StridedDeviceAddressRegionKHR rayMissRegion;
		rayMissRegion.setDeviceAddress(m_sbtBuffer.GetDeviceAddress() + m_handleSizeAligned * rtScene->GetMissGroupsOffset());
		rayMissRegion.setSize(m_handleSizeAligned * rtScene->GetMissGroupsSize());
		rayMissRegion.setStride(m_handleSizeAligned);

		vk::StridedDeviceAddressRegionKHR rayHitRegion;
		rayHitRegion.setDeviceAddress(m_sbtBuffer.GetDeviceAddress() + m_handleSizeAligned * rtScene->GetHitGroupsOffset());
		rayHitRegion.setSize(m_handleSizeAligned * rtScene->GetHitGroupsSize());
		rayHitRegion.setStride(m_handleSizeAligned);

		inCommandBuffer->traceRaysKHR(rayGenRegion, rayMissRegion, rayHitRegion, {0,0,0}, GetWidth(), GetHeight(), 1);
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
		PerFrameData* frameData = Engine::GetRendererInstance()->GetPerFrameData();

		vk::PipelineCacheCreateInfo pipelineCacheInfo;
		vk::PipelineCache pipelineCache = nativeDevice.createPipelineCache(pipelineCacheInfo);

		vk::DeferredOperationKHR deferredOperation;

		nativeDevice.destroyPipeline(m_rtPipeline);
		nativeDevice.destroyPipelineLayout(m_rtPipelineLayout);

		//------------------------------------------------------------------
		// get descriptor sets from shaders
		m_sets.clear();
		std::vector<VulkanDescriptorSet>& sets = m_shaderResourceMapper.GetDescriptorSets();
		std::vector<vk::DescriptorSetLayout> descLayouts;
		descLayouts.resize(sets.size());
		m_sets.resize(sets.size());
		for (uint32_t idx = 0; idx < descLayouts.size(); idx++)
		{
			descLayouts[idx] = sets[idx].GetLayout();
			m_sets[idx] = idx == 0 ? frameData->GetSet() : sets[idx].GetSet();
		}
		//------------------------------------------------------------------
		// pipeline layout info
		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo.setSetLayoutCount(descLayouts.size());
		layoutInfo.setPSetLayouts(descLayouts.data());
		m_rtPipelineLayout = nativeDevice.createPipelineLayout(layoutInfo);
		//------------------------------------------------------------------
		// pipeline create info
		vk::RayTracingPipelineCreateInfoKHR pipelineInfo;
		pipelineInfo.setGroupCount(static_cast<uint32_t>(rtScene->GetShaderGroups().size()));
		pipelineInfo.setPGroups(rtScene->GetShaderGroups().data());
		pipelineInfo.setStageCount(static_cast<uint32_t>(rtScene->GetShaderStages().size()));
		pipelineInfo.setPStages(rtScene->GetShaderStages().data());
		pipelineInfo.setFlags({});
		pipelineInfo.setLayout(m_rtPipelineLayout);

		auto pipelineResult = nativeDevice.createRayTracingPipelineKHR(deferredOperation, pipelineCache, pipelineInfo);
		if (pipelineResult.result != vk::Result::eSuccess)
		{
			return Pipeline();
		}
		m_rtPipeline = pipelineResult.value;

		vk::StructureChain<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR> structChain = 
			device->GetPhysicalDevice().GetDevice().getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
		m_rtProps = structChain.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
		uint32_t handleSize = m_rtProps.shaderGroupHandleSize;
		uint32_t baseAlignment = m_rtProps.shaderGroupBaseAlignment;
		// nvidia recommended to use base alignment
		// we avoid using power of two version formula just in case. who knows
		m_handleSizeAligned = baseAlignment * ((handleSize + baseAlignment - 1) / baseAlignment);
		uint32_t groupCount = static_cast<uint32_t>( rtScene->GetShaderGroups().size() );
		uint64_t sbtSize = groupCount * m_handleSizeAligned;
		std::vector<char> shadersHandles(sbtSize);

		auto handlesResult = nativeDevice.getRayTracingShaderGroupHandlesKHR(m_rtPipeline, 0, groupCount, sbtSize, shadersHandles.data());
		if (handlesResult != vk::Result::eSuccess)
		{
			// TODO
		}
		m_sbtBuffer.Destroy();
		m_sbtBuffer = ResourceUtils::CreateBuffer(
			device, 
			sbtSize,
			vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			true
		);
		if (sbtSize > 0)
		{
			m_sbtBuffer.CopyTo(sbtSize, shadersHandles.data(), true);
		}

		return m_rtPipeline;
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
			vk::Format::eR32Uint,//R8G8B8A8Unorm, 
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

		m_rayGenShader = DataManager::RequestResourceType<RtShader>("content/shaders/RayGenShadows.spv", ERtShaderType::RST_RAY_GEN);
		m_rayMissShader = DataManager::RequestResourceType<RtShader>("content/shaders/RayMissShadows.spv", ERtShaderType::RST_MISS);
	}

	void RTShadowPass::OnDestroy()
	{
		// TODO cleanup
	}

	void RTShadowPass::HandlePreUpdate(std::shared_ptr<GlobalPreUpdateMessage> msg)
	{
		UpdateShaderResources();
		CreatePipeline(nullptr, {}, {});
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

