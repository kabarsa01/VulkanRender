#ifndef __RT_SHADOW_PASS_H__
#define __RT_SHADOW_PASS_H__

#include "render/passes/VulkanPassBase.h"
#include "render/shader/RtShader.h"
#include "messages/Messages.h"
#include "messages/MessageSubscriber.h"
#include <memory>
#include "../shader/ShaderResourceMapper.h"
#include "data/BufferData.h"

namespace CGE
{

	namespace vk = VULKAN_HPP_NAMESPACE;

	class RTShadowPass : public VulkanPassBase
	{
	public:
		RTShadowPass(HashString name);
		~RTShadowPass();

		void RecordCommands(CommandBuffer* inCommandBuffer) override;

		Texture2DPtr GetVisibilityTexture() { return m_visibilityTex; }
		std::vector<TextureDataPtr>& GetVisibilityTextures() { return m_visibilityTextures; }
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

		VulkanImage m_visibilityBuffer;
		ImageView m_visibilityView;
		Texture2DPtr m_visibilityTex;
		std::vector<TextureDataPtr> m_visibilityTextures;

		Texture2DPtr m_normalsTex;
		Texture2DPtr m_depthTex;
		BufferDataPtr m_clusterLightsData;
		BufferDataPtr m_lightsList;
		BufferDataPtr m_lightsIndices;

		ShaderResourceMapper m_shaderResourceMapper;

		vk::Pipeline m_rtPipeline;
		vk::PipelineLayout m_rtPipelineLayout;
		std::vector<VulkanDescriptorSet> m_sets;
		std::vector<vk::DescriptorSet> m_nativeSets;
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProps;
		BufferDataPtr m_sbtBuffer;
		uint32_t m_handleSizeAligned;

		void HandlePreUpdate(std::shared_ptr<GlobalPreFrameMessage> msg);
		void UpdateShaderResources();
		void UpdatePipeline();
	};

}

#endif
