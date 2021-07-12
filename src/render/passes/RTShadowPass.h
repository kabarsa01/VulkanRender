#ifndef __RT_SHADOW_PASS_H__
#define __RT_SHADOW_PASS_H__

#include "render/passes/VulkanPassBase.h"
#include "render/shader/RtShader.h"
#include "messages/Messages.h"
#include "messages/MessageSubscriber.h"
#include <memory>
#include "../resources/VulkanBuffer.h"
#include "../shader/ShaderResourceMapper.h"

namespace CGE
{

	namespace vk = VULKAN_HPP_NAMESPACE;

	class RTShadowPass : public VulkanPassBase
	{
	public:
		RTShadowPass(HashString name);
		~RTShadowPass();

		void RecordCommands(CommandBuffer* inCommandBuffer) override;
	protected:
		void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight) override;
		void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight) override;
		Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
		RenderPass CreateRenderPass() override;
		void OnCreate() override;
		void OnDestroy() override;
	private:
		MessageSubscriber m_subscriber;

		RtShaderPtr m_rayGenShader;
		RtShaderPtr m_rayMissShader;

		VulkanImage m_visibilityBuffer1;
		ImageView m_visibilityView1;
		VulkanImage m_visibilityBuffer2;
		ImageView m_visibilityView2;
		Texture2DPtr m_visibilityTex1;
		Texture2DPtr m_visibilityTex2;

		Texture2DPtr m_normalsTex;
		Texture2DPtr m_depthTex;
		VulkanBuffer m_clusterLightsData;
		VulkanBuffer m_lightsList;
		VulkanBuffer m_lightsIndices;

		ShaderResourceMapper m_shaderResourceMapper;
		std::vector<VulkanDescriptorSet> m_descriptorSets;

		vk::Pipeline m_rtPipeline;
		vk::PipelineLayout m_rtPipelineLayout;

		void HandlePreUpdate(std::shared_ptr<GlobalPreUpdateMessage> msg);
		void UpdateShaderResources();
	};

}

#endif
