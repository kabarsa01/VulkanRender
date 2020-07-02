#include "LightClusteringPass.h"
#include "data/DataManager.h"
#include "../DataStructures.h"
#include "VulkanPassBase.h"
#include "render/Renderer.h"
#include "ZPrepass.h"
#include "scene/light/LightComponent.h"
#include "scene/SceneObjectComponent.h"
#include "scene/SceneObjectBase.h"
#include "scene/Transform.h"

LightClusteringPass::LightClusteringPass(HashString inName)
	: VulkanPassBase(inName)
{

}

void LightClusteringPass::RecordCommands(CommandBuffer* inCommandBuffer)
{
	ScenePtr scene = Engine::GetSceneInstance();
	std::vector<LightComponentPtr> lights = scene->GetSceneComponentsCast<LightComponent>();
	std::map<LightType, std::vector<LightInfo>> sortedLights;
	for (LightComponentPtr lightComp : lights)
	{
		LightInfo info;
		info.direction = vec4(lightComp->GetParent()->transform.GetForwardVector(), 0.0);
		info.position = vec4(lightComp->GetParent()->transform.GetLocation(), 1.0);
		info.color = vec4(lightComp->color, 0.0);
		info.rai.x = lightComp->radius;
		info.rai.y = lightComp->spotHalfAngle;
		info.rai.z = lightComp->intensity;

		sortedLights[lightComp->type].push_back(info);
	}
	lightsIndices->directionalPosition.x = 0;
	lightsIndices->directionalPosition.y = static_cast<uint32_t>(sortedLights[LT_Directional].size());
	lightsIndices->pointPosition.x = lightsIndices->directionalPosition.y;
	lightsIndices->pointPosition.y = static_cast<uint32_t>(sortedLights[LT_Point].size());
	lightsIndices->spotPosition.x = lightsIndices->pointPosition.x + lightsIndices->pointPosition.y;
	lightsIndices->spotPosition.y = static_cast<uint32_t>(sortedLights[LT_Spot].size());

	std::memcpy(&lightsList->lights[lightsIndices->directionalPosition.x], sortedLights[LT_Directional].data(), sizeof(LightInfo) * lightsIndices->directionalPosition.y);
	std::memcpy(&lightsList->lights[lightsIndices->spotPosition.x], sortedLights[LT_Spot].data(), sizeof(LightInfo) * lightsIndices->spotPosition.y);
	std::memcpy(&lightsList->lights[lightsIndices->pointPosition.x], sortedLights[LT_Point].data(), sizeof(LightInfo) * lightsIndices->pointPosition.y);

	computeMaterial->UpdateUniformBuffer<LightsList>("lightsList", *lightsList);
	computeMaterial->UpdateUniformBuffer<LightsIndices>("lightsIndices", *lightsIndices);

	// barriers ----------------------------------------------
	ImageMemoryBarrier depthTextureBarrier = depthTexture->GetImage().CreateLayoutBarrier(
		ImageLayout::eUndefined,
		ImageLayout::eShaderReadOnlyOptimal,
		AccessFlagBits::eShaderWrite,
		AccessFlagBits::eShaderRead,
		ImageAspectFlagBits::eDepth | ImageAspectFlagBits::eStencil,
		0, 1, 0, 1);
	ImageMemoryBarrier clustersTextureBarrier = image.CreateLayoutBarrier(
		ImageLayout::eUndefined,
		ImageLayout::eGeneral,
		AccessFlagBits::eShaderRead,
		AccessFlagBits::eShaderWrite,
		ImageAspectFlagBits::eColor,
		0, 1, 0, 1);
	std::array<ImageMemoryBarrier, 2> barriers{ depthTextureBarrier, clustersTextureBarrier };
	inCommandBuffer->pipelineBarrier(
		PipelineStageFlagBits::eAllGraphics,
		PipelineStageFlagBits::eComputeShader,
		DependencyFlags(),
		0, nullptr, 0, nullptr,
		static_cast<uint32_t>( barriers.size() ), barriers.data());

	PipelineData& pipelineData = FindPipeline(computeMaterial);

	DeviceSize offset = 0;
	inCommandBuffer->bindPipeline(PipelineBindPoint::eCompute, pipelineData.pipeline);
	inCommandBuffer->bindDescriptorSets(PipelineBindPoint::eCompute, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});
	inCommandBuffer->dispatch(32, 32, 1);
}

void LightClusteringPass::OnCreate()
{
	image.createInfo.setArrayLayers(1);
	image.createInfo.setExtent(Extent3D(64, 64, 1));
	image.createInfo.setFormat(Format::eR8G8B8A8Unorm);
	image.createInfo.setImageType(ImageType::e2D);
	image.createInfo.setInitialLayout(ImageLayout::eUndefined);
	image.createInfo.setMipLevels(1);
	image.createInfo.setSamples(SampleCountFlagBits::e1);
	image.createInfo.setSharingMode(SharingMode::eExclusive);
	image.createInfo.setTiling(ImageTiling::eOptimal);
	image.createInfo.setUsage(ImageUsageFlagBits::eStorage | ImageUsageFlagBits::eSampled);
	image.Create(GetVulkanDevice());
	image.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
	imageView = image.CreateView(ImageSubresourceRange(ImageAspectFlagBits::eColor, 0, 1, 0, 1), ImageViewType::e2D);

	texture = ObjectBase::NewObject<Texture2D, const HashString&>("ComputeTexture");
	texture->CreateFromExternal(image, imageView, true);
	ZPrepass* zPrepass = GetRenderer()->GetZPrepass();
	depthTexture = ObjectBase::NewObject<Texture2D, const HashString&>("ComputeDepthTexture");
	depthTexture->CreateFromExternal(zPrepass->GetDepthAttachment(), zPrepass->GetDepthAttachmentView(), false);

	clusterLightData = new ClusterLightsData();
	lightsList = new LightsList();
	lightsIndices = new LightsIndices();

	computeMaterial = DataManager::RequestResourceType<Material>("LightClusteringMaterial");
	computeMaterial->SetComputeShaderPath("content/shaders/LightClustering.spv");
	computeMaterial->SetStorageBuffer<ClusterLightsData>("clusterLightsData", *clusterLightData);
	computeMaterial->SetUniformBuffer<LightsList>("lightsList", *lightsList);
	computeMaterial->SetUniformBuffer<LightsIndices>("lightsIndices", *lightsIndices);
	computeMaterial->SetStorageTexture("storageTex", texture);
	computeMaterial->SetTexture("depthTexture", depthTexture);
	computeMaterial->LoadResources();
}

void LightClusteringPass::OnDestroy()
{
	delete clusterLightData;
	delete lightsList;
	delete lightsIndices;
}

RenderPass LightClusteringPass::CreateRenderPass()
{
	return RenderPass();
}

void LightClusteringPass::CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight)
{
}

void LightClusteringPass::CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight)
{
}

Pipeline LightClusteringPass::CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass)
{
	ComputePipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.setLayout(inLayout);
	pipelineCreateInfo.setStage(inMaterial->GetComputeStageInfo());

	return GetVulkanDevice()->GetDevice().createComputePipeline(GetVulkanDevice()->GetPipelineCache(), pipelineCreateInfo);
}

