#include "DeferredLightingPass.h"
#include "utils/ResourceUtils.h"
#include "data/MeshData.h"
#include "GBufferPass.h"
#include "data/DataManager.h"
#include "../DataStructures.h"
#include "LightClusteringPass.h"
#include "ZPrepass.h"
#include "../RtScene.h"
#include "utils/Singleton.h"
#include "RTShadowPass.h"
#include "core/Engine.h"
#include "ClusterComputePass.h"
#include "DepthPrepass.h"

namespace CGE
{
	using namespace VULKAN_HPP_NAMESPACE;

	DeferredLightingPass::DeferredLightingPass(HashString inName)
		: RenderPassBase(inName)
	{
	
	}
	
	//void DeferredLightingPass::RecordCommands(CommandBuffer* inCommandBuffer)
	//{
	//	LightClusteringPass* clusteringPass = GetRenderer()->GetLightClusteringPass();
	//	BufferDataPtr buffer = clusteringPass->computeMaterial->GetStorageBuffer("clusterLightsData");

	//	RTShadowPass* rtPass = Engine::GetRendererInstance()->GetRTShadowPass();
	//
	//	// barriers ----------------------------------------------
	//	BufferMemoryBarrier clusterDataBarrier = buffer->GetBuffer().CreateMemoryBarrier(
	//		0, 0, 
	//		AccessFlagBits::eShaderWrite, 
	//		AccessFlagBits::eShaderRead);
	//	ImageMemoryBarrier attachmentBarrier = GetAttachments()[0].CreateLayoutBarrier(
	//		ImageLayout::eUndefined,
	//		ImageLayout::eColorAttachmentOptimal,
	//		AccessFlagBits::eShaderRead,
	//		AccessFlagBits::eShaderWrite,
	//		ImageAspectFlagBits::eColor,
	//		0, 1, 0, 1);
	//	ImageMemoryBarrier depthTextureBarrier = depthTexture->GetImage().CreateLayoutBarrier(
	//		ImageLayout::eUndefined,
	//		ImageLayout::eShaderReadOnlyOptimal,
	//		AccessFlagBits::eShaderWrite,
	//		AccessFlagBits::eShaderRead,
	//		ImageAspectFlagBits::eDepth | ImageAspectFlagBits::eStencil,
	//		0, 1, 0, 1);
	//	ImageMemoryBarrier visibilityTextureBarrier = visibilityTexture->GetImage().CreateLayoutBarrier(
	//		ImageLayout::eUndefined,
	//		ImageLayout::eShaderReadOnlyOptimal,
	//		AccessFlagBits::eShaderWrite,
	//		AccessFlagBits::eShaderRead,
	//		ImageAspectFlagBits::eColor,
	//		0, 1, 0, 1);
	//	std::vector<ImageMemoryBarrier> barriers{ attachmentBarrier, depthTextureBarrier, visibilityTextureBarrier };
	//	for (auto tex : rtPass->GetVisibilityTextures())
	//	{
	//		ImageMemoryBarrier visibilityBarrier = tex->GetImage().CreateLayoutBarrier(
	//			ImageLayout::eUndefined,
	//			ImageLayout::eShaderReadOnlyOptimal,
	//			AccessFlagBits::eShaderWrite,
	//			AccessFlagBits::eShaderRead,
	//			ImageAspectFlagBits::eColor,
	//			0, 1, 0, 1);

	//		barriers.push_back(visibilityBarrier);
	//	}

	//	inCommandBuffer->pipelineBarrier(
	//		PipelineStageFlagBits::eVertexShader | PipelineStageFlagBits::eComputeShader | PipelineStageFlagBits::eRayTracingShaderKHR,
	//		PipelineStageFlagBits::eFragmentShader,
	//		DependencyFlags(),
	//		0, nullptr,
	//		1, &clusterDataBarrier,
	//		static_cast<uint32_t>(barriers.size()), barriers.data());
	//
	//	MeshDataPtr meshData = MeshData::FullscreenQuad();
	//	PipelineData& pipelineData = FindPipeline(lightingMaterial);
	//
	//	ClearValue clearValue;
	//	clearValue.setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));
	//
	//	RenderPassBeginInfo passBeginInfo;
	//	passBeginInfo.setRenderPass(GetRenderPass());
	//	passBeginInfo.setFramebuffer(GetFramebuffer());
	//	passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), Extent2D(GetWidth(), GetHeight()) ));
	//	passBeginInfo.setClearValueCount(1);
	//	passBeginInfo.setPClearValues(&clearValue);
	//
	//	DeviceSize offset = 0;
	//	inCommandBuffer->beginRenderPass(passBeginInfo, SubpassContents::eInline);
	//	inCommandBuffer->bindPipeline(PipelineBindPoint::eGraphics, pipelineData.pipeline);
	//	inCommandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});
	//
	//	inCommandBuffer->bindVertexBuffers(0, 1, &meshData->GetVertexBuffer()->GetNativeBuffer(), &offset);
	//	inCommandBuffer->bindIndexBuffer(meshData->GetIndexBuffer()->GetNativeBuffer(), 0, IndexType::eUint32);
	//	inCommandBuffer->drawIndexed(meshData->GetIndexCount(), 1, 0, 0, 0);
	//	inCommandBuffer->endRenderPass();
	//}
	//
	//void DeferredLightingPass::OnCreate()
	//{
	//	ZPrepass* zPrepass = GetRenderer()->GetZPrepass();
	//	GBufferPass* gBufferPass = GetRenderer()->GetGBufferPass();
	//	LightClusteringPass* clusteringPass = GetRenderer()->GetLightClusteringPass();
	//	RTShadowPass* rtShadowPass = GetRenderer()->GetRTShadowPass();
	//
	//	albedoTexture = ObjectBase::NewObject<Texture2D, const HashString&>("DeferredLightingAlbedoTexture");
	//	albedoTexture->CreateFromExternal(gBufferPass->GetAttachments()[0], gBufferPass->GetAttachmentViews()[0]);
	//	normalTexture = ObjectBase::NewObject<Texture2D, const HashString&>("DeferredLightingNormalTexture");
	//	normalTexture->CreateFromExternal(gBufferPass->GetAttachments()[1], gBufferPass->GetAttachmentViews()[1]);
	//	depthTexture = ObjectBase::NewObject<Texture2D, const HashString&>("DeferredLightingDepthTexture");
	//	depthTexture->CreateFromExternal(zPrepass->GetDepthAttachment(), zPrepass->GetDepthAttachmentView(), false);
	//	visibilityTexture = ObjectBase::NewObject<Texture2D, const HashString&>("DeferredLightingVisibilityTexture");
	//	visibilityTexture->CreateFromExternal(rtShadowPass->GetVisibilityTexture(), false);
	//
	//	lightingMaterial = DataManager::RequestResourceType<Material, const std::string&, const std::string&>(
	//		"DeferredLightingMaterial",
	//		"content/shaders/ScreenSpaceVert.spv",
	//		"content/shaders/DeferredLighting.spv"
	//		);
	//	lightingMaterial->SetStorageBufferExternal("clusterLightsData", clusteringPass->computeMaterial->GetStorageBuffer("clusterLightsData"));
	//	lightingMaterial->SetUniformBufferExternal("lightsList", clusteringPass->computeMaterial->GetUniformBuffer("lightsList"));
	//	lightingMaterial->SetUniformBufferExternal("lightsIndices", clusteringPass->computeMaterial->GetUniformBuffer("lightsIndices"));
	//	lightingMaterial->SetTexture("albedoTex", albedoTexture);
	//	lightingMaterial->SetTexture("normalsTex", normalTexture);
	//	lightingMaterial->SetTexture("depthTex", depthTexture);
	//	lightingMaterial->SetTexture("visibilityTex", visibilityTexture);
	//	lightingMaterial->SetTextureArray("visibilityTextures", rtShadowPass->GetVisibilityTextures());
	//	lightingMaterial->SetAccelerationStructure("topLevelAS", Singleton<RtScene>::GetInstance()->GetTlas().accelerationStructure);

	//	lightingMaterial->LoadResources();
	//}
	//
	//RenderPass DeferredLightingPass::CreateRenderPass()
	//{
	//	AttachmentDescription colorAttachmentDesc;
	//	colorAttachmentDesc.setFormat(Format::eR16G16B16A16Sfloat); // 16bit float to accumulate lighting
	//	colorAttachmentDesc.setSamples(SampleCountFlagBits::e1);
	//	colorAttachmentDesc.setLoadOp(AttachmentLoadOp::eClear);
	//	colorAttachmentDesc.setStoreOp(AttachmentStoreOp::eStore);
	//	colorAttachmentDesc.setStencilLoadOp(AttachmentLoadOp::eDontCare);
	//	colorAttachmentDesc.setStencilStoreOp(AttachmentStoreOp::eDontCare);
	//	colorAttachmentDesc.setInitialLayout(ImageLayout::eUndefined);
	//	colorAttachmentDesc.setFinalLayout(ImageLayout::eColorAttachmentOptimal);
	//	AttachmentReference colorAttachmentRef;
	//	colorAttachmentRef.setAttachment(0);
	//	colorAttachmentRef.setLayout(ImageLayout::eColorAttachmentOptimal);
	//
	//	SubpassDescription subpassDesc;
	//	subpassDesc.setColorAttachmentCount(1);
	//	subpassDesc.setPColorAttachments(&colorAttachmentRef);
	//	subpassDesc.setPDepthStencilAttachment(nullptr);
	//	subpassDesc.setPipelineBindPoint(PipelineBindPoint::eGraphics);
	//
	//	SubpassDependency subpassDependency;
	//	subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
	//	subpassDependency.setDstSubpass(0);
	//	subpassDependency.setSrcStageMask(PipelineStageFlagBits::eColorAttachmentOutput);
	//	subpassDependency.setSrcAccessMask(AccessFlags());
	//	subpassDependency.setDstStageMask(PipelineStageFlagBits::eColorAttachmentOutput);
	//	subpassDependency.setDstAccessMask(AccessFlagBits::eColorAttachmentRead | AccessFlagBits::eColorAttachmentWrite);
	//
	//	RenderPassCreateInfo renderPassInfo;
	//	renderPassInfo.setAttachmentCount(1);
	//	renderPassInfo.setPAttachments(&colorAttachmentDesc);
	//	renderPassInfo.setSubpassCount(1);
	//	renderPassInfo.setPSubpasses(&subpassDesc);
	//	renderPassInfo.setDependencyCount(1);
	//	renderPassInfo.setPDependencies(&subpassDependency);
	//
	//	return GetVulkanDevice()->GetDevice().createRenderPass(renderPassInfo);
	//}
	//
	//void DeferredLightingPass::CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight)
	//{
	//	VulkanImage colorAttachmentImage = ResourceUtils::CreateColorAttachment(GetVulkanDevice(), inWidth, inHeight, vk::Format::eR16G16B16A16Sfloat); // do not forget 16 bit float
	//	outAttachments.push_back(colorAttachmentImage);
	//	outAttachmentViews.push_back(colorAttachmentImage.CreateView({ ImageAspectFlagBits::eColor, 0, 1, 0, 1 }, ImageViewType::e2D));
	//}
	//
	//void DeferredLightingPass::CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight)
	//{
	//}
	//
	//Pipeline DeferredLightingPass::CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass)
	//{
	//	std::vector<PipelineShaderStageCreateInfo> shaderStageInfoArray = { inMaterial->GetVertexStageInfo(), inMaterial->GetFragmentStageInfo() };
	//
	//	VertexInputBindingDescription bindingDesc = MeshData::GetBindingDescription(0);
	//	std::array<VertexInputAttributeDescription, 5> attributeDesc = Vertex::GetAttributeDescriptions(0);
	//	PipelineVertexInputStateCreateInfo vertexInputInfo;
	//	vertexInputInfo.setVertexBindingDescriptionCount(1);
	//	vertexInputInfo.setPVertexBindingDescriptions(&bindingDesc);
	//	vertexInputInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDesc.size()));
	//	vertexInputInfo.setPVertexAttributeDescriptions(attributeDesc.data());
	//
	//	PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	//	inputAssemblyInfo.setTopology(PrimitiveTopology::eTriangleList);
	//	inputAssemblyInfo.setPrimitiveRestartEnable(VK_FALSE);
	//
	//	Viewport viewport;
	//	viewport.setX(0.0f);
	//	viewport.setY(0.0f);
	//	viewport.setWidth(static_cast<float>(GetWidth()));
	//	viewport.setHeight(static_cast<float>(GetHeight()));
	//	viewport.setMinDepth(0.0f);
	//	viewport.setMaxDepth(1.0f);
	//
	//	Rect2D scissor;
	//	scissor.setOffset(Offset2D(0, 0));
	//	scissor.setExtent(Extent2D{ GetWidth(), GetHeight() });
	//
	//	PipelineViewportStateCreateInfo viewportInfo;
	//	viewportInfo.setViewportCount(1);
	//	viewportInfo.setPViewports(&viewport);
	//	viewportInfo.setScissorCount(1);
	//	viewportInfo.setPScissors(&scissor);
	//
	//	PipelineRasterizationStateCreateInfo rasterizationInfo;
	//	rasterizationInfo.setDepthClampEnable(VK_FALSE);
	//	rasterizationInfo.setRasterizerDiscardEnable(VK_FALSE);
	//	rasterizationInfo.setPolygonMode(PolygonMode::eFill);
	//	rasterizationInfo.setLineWidth(1.0f);
	//	rasterizationInfo.setCullMode(CullModeFlagBits::eNone);
	//	rasterizationInfo.setFrontFace(FrontFace::eClockwise);
	//	rasterizationInfo.setDepthBiasEnable(VK_FALSE);
	//
	//	PipelineMultisampleStateCreateInfo multisampleInfo;
	//
	//	PipelineColorBlendAttachmentState colorBlendAttachment;
	//	colorBlendAttachment.setColorWriteMask(ColorComponentFlagBits::eR | ColorComponentFlagBits::eG | ColorComponentFlagBits::eB | ColorComponentFlagBits::eA);
	//	colorBlendAttachment.setBlendEnable(VK_FALSE);
	//	colorBlendAttachment.setSrcColorBlendFactor(BlendFactor::eOne);
	//	colorBlendAttachment.setDstColorBlendFactor(BlendFactor::eZero);
	//	colorBlendAttachment.setColorBlendOp(BlendOp::eAdd);
	//	colorBlendAttachment.setSrcAlphaBlendFactor(BlendFactor::eOne);
	//	colorBlendAttachment.setDstAlphaBlendFactor(BlendFactor::eZero);
	//	colorBlendAttachment.setAlphaBlendOp(BlendOp::eAdd);
	//
	//	PipelineColorBlendStateCreateInfo colorBlendInfo;
	//	colorBlendInfo.setLogicOpEnable(VK_FALSE);
	//	colorBlendInfo.setLogicOp(LogicOp::eCopy);
	//	colorBlendInfo.setAttachmentCount(1);
	//	colorBlendInfo.setPAttachments(&colorBlendAttachment);
	//	colorBlendInfo.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });
	//
	//	GraphicsPipelineCreateInfo pipelineInfo;
	//	pipelineInfo.setStageCount(2);
	//	pipelineInfo.setPStages(shaderStageInfoArray.data());
	//	pipelineInfo.setPVertexInputState(&vertexInputInfo);
	//	pipelineInfo.setPInputAssemblyState(&inputAssemblyInfo);
	//	pipelineInfo.setPViewportState(&viewportInfo);
	//	pipelineInfo.setPRasterizationState(&rasterizationInfo);
	//	pipelineInfo.setPMultisampleState(&multisampleInfo);
	//	pipelineInfo.setPDepthStencilState(nullptr);
	//	pipelineInfo.setPColorBlendState(&colorBlendInfo);
	//	pipelineInfo.setPDynamicState(nullptr);
	//	pipelineInfo.setLayout(inLayout);
	//	pipelineInfo.setRenderPass(inRenderPass);
	//	pipelineInfo.setSubpass(0);
	//	pipelineInfo.setBasePipelineHandle(Pipeline());
	//	pipelineInfo.setBasePipelineIndex(-1);
	//
	//	return GetVulkanDevice()->GetDevice().createGraphicsPipeline(GetVulkanDevice()->GetPipelineCache(), pipelineInfo).value;
	//}
	
	void DeferredLightingPass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto clusterData = dataTable.GetPassData<ClusterComputeData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();
		auto rtShadowsData = dataTable.GetPassData<RTShadowsData>();

		uint32_t frameIndex = Engine::GetFrameIndex(m_lightingMaterials.size());
		MaterialPtr lightingMat = m_lightingMaterials[frameIndex];
		BufferDataPtr buffer = lightingMat->GetStorageBuffer("clusterLightsData");
		TextureDataPtr depthTex = lightingMat->GetSampledTexture("depthTex");

		auto textures = lightingMat->GetSampledTextures("albedoTex", "normalsTex");

		RTShadowPass* rtPass = Engine::GetRendererInstance()->GetRTShadowPass();
	
		// barriers ----------------------------------------------
		BufferMemoryBarrier clusterDataBarrier = buffer->GetBuffer().CreateMemoryBarrier(
			0, 0, 
			AccessFlagBits::eShaderWrite, 
			AccessFlagBits::eShaderRead);
		//ImageMemoryBarrier attachmentBarrier = GetAttachments()[0].CreateLayoutBarrier(
		//	ImageLayout::eUndefined,
		//	ImageLayout::eColorAttachmentOptimal,
		//	AccessFlagBits::eShaderRead,
		//	AccessFlagBits::eShaderWrite,
		//	ImageAspectFlagBits::eColor,
		//	0, 1, 0, 1);
		ImageMemoryBarrier depthTextureBarrier = depthTex->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			AccessFlagBits::eShaderWrite,
			AccessFlagBits::eShaderRead,
			ImageAspectFlagBits::eDepth | ImageAspectFlagBits::eStencil,
			0, 1, 0, 1);
		ImageMemoryBarrier visibilityTextureBarrier = rtShadowsData->visibilityTex->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			AccessFlagBits::eShaderWrite,
			AccessFlagBits::eShaderRead,
			ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		std::vector<ImageMemoryBarrier> barriers{ depthTextureBarrier, visibilityTextureBarrier };
		for (auto tex : rtShadowsData->visibilityTextures)
		{
			ImageMemoryBarrier visibilityBarrier = tex->GetImage().CreateLayoutBarrier(
				ImageLayout::eUndefined,
				ImageLayout::eShaderReadOnlyOptimal,
				AccessFlagBits::eShaderWrite,
				AccessFlagBits::eShaderRead,
				ImageAspectFlagBits::eColor,
				0, 1, 0, 1);
			barriers.push_back(visibilityBarrier);
		}

		commandBuffer->pipelineBarrier(
			PipelineStageFlagBits::eAllCommands,
			PipelineStageFlagBits::eAllCommands,
			DependencyFlags(),
			0, nullptr,
			1, &clusterDataBarrier,
			static_cast<uint32_t>(barriers.size()), barriers.data());
	
		MeshDataPtr meshData = MeshData::FullscreenQuad();
		PipelineData& pipelineData = executeContext.FindPipeline(lightingMat);
	
		ClearValue clearValue;
		clearValue.setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));
	
		RenderPassBeginInfo passBeginInfo;
		passBeginInfo.setRenderPass(executeContext.GetRenderPass());
		passBeginInfo.setFramebuffer(executeContext.GetFramebuffer());
		passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), Extent2D(executeContext.GetWidth(), executeContext.GetHeight()) ));
		passBeginInfo.setClearValueCount(1);
		passBeginInfo.setPClearValues(&clearValue);
	
		DeviceSize offset = 0;
		commandBuffer->beginRenderPass(passBeginInfo, SubpassContents::eInline);
		commandBuffer->bindPipeline(PipelineBindPoint::eGraphics, pipelineData.pipeline);
		commandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});
	
		commandBuffer->bindVertexBuffers(0, 1, &meshData->GetVertexBuffer()->GetNativeBuffer(), &offset);
		commandBuffer->bindIndexBuffer(meshData->GetIndexBuffer()->GetNativeBuffer(), 0, IndexType::eUint32);
		commandBuffer->drawIndexed(meshData->GetIndexCount(), 1, 0, 0, 0);
		commandBuffer->endRenderPass();
	}

	void DeferredLightingPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto clusteringData = dataTable.GetPassData<ClusterComputeData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();
		auto rtShadowsData = dataTable.GetPassData<RTShadowsData>();

		assert((depthData->depthTextures.size() == clusteringData->computeMaterials.size()) && (clusteringData->computeMaterials.size() == gbufferData->albedos.size()), "Wrong number of resources");

		for (uint32_t idx = 0; idx < depthData->depthTextures.size(); ++idx)
		{
			MaterialPtr lightingMaterial = DataManager::RequestResourceType<Material, const std::string&, const std::string&>(
				"DeferredLightingMaterial",
				"content/shaders/ScreenSpaceVert.spv",
				"content/shaders/DeferredLighting.spv"
				);
			lightingMaterial->SetStorageBufferExternal("clusterLightsData", clusteringData->computeMaterials[idx]->GetStorageBuffer("clusterLightsData"));
			lightingMaterial->SetUniformBufferExternal("lightsList", clusteringData->computeMaterials[idx]->GetUniformBuffer("lightsList"));
			lightingMaterial->SetUniformBufferExternal("lightsIndices", clusteringData->computeMaterials[idx]->GetUniformBuffer("lightsIndices"));
			lightingMaterial->SetTexture("albedoTex", gbufferData->albedos[idx]);
			lightingMaterial->SetTexture("normalsTex", gbufferData->normals[idx]);
			lightingMaterial->SetTexture("depthTex", depthData->depthTextures[idx]);
			lightingMaterial->SetTextureArray("visibilityTextures", rtShadowsData->visibilityTextures);
			lightingMaterial->SetAccelerationStructure("topLevelAS", Singleton<RtScene>::GetInstance()->GetTlas().accelerationStructure);
			lightingMaterial->LoadResources();

			m_lightingMaterials.push_back(lightingMaterial);
		}

		auto deferredLightingData = dataTable.CreatePassData<DeferredLightingData>();
		deferredLightingData->hdrRenderTargets = ResourceUtils::CreateColorTextureArray("DeferredLightingRT", 2, initContext.GetWidth(), initContext.GetHeight(), vk::Format::eR16G16B16A16Sfloat, false);
		initContext.SetAttachments(0, deferredLightingData->hdrRenderTargets, true);
	}

}
