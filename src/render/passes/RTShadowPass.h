#ifndef __RT_SHADOW_PASS_H__
#define __RT_SHADOW_PASS_H__

#include "render/shader/RtShader.h"
#include "messages/Messages.h"
#include "messages/MessageSubscriber.h"
#include <memory>
#include "../shader/ShaderResourceMapper.h"
#include "data/BufferData.h"
#include "RenderPassBase.h"

namespace CGE
{

	struct RTShadowsData : public Identifiable<RTShadowsData>
	{
		std::vector<TextureDataPtr> visibilityTextures;
	};

	class RTShadowPass : public RenderPassBase
	{
	public:
		RTShadowPass(HashString name);
		~RTShadowPass();
	protected:
		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;
	//	void RecordCommands(CommandBuffer* inCommandBuffer);

	//	Texture2DPtr GetVisibilityTexture() { return m_visibilityTex; }
	//	std::vector<TextureDataPtr>& GetVisibilityTextures() { return m_visibilityTextures; }
	//protected:
	//	void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight);
	//	void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight);
	//	Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass);
	//	RenderPass CreateRenderPass();
	//	void OnCreate();
	//	void OnDestroy();
	private:
		MessageSubscriber m_subscriber;

		RtShaderPtr m_rayGenShader;
		RtShaderPtr m_rayMissShader;

		//VulkanImage m_visibilityBuffer;
		//ImageView m_visibilityView;
		Texture2DPtr m_visibilityTex;
		std::vector<TextureDataPtr> m_visibilityTextures;

		//Texture2DPtr m_normalsTex;
		//Texture2DPtr m_depthTex;
		//BufferDataPtr m_clusterLightsData;
		//BufferDataPtr m_lightsList;
		//BufferDataPtr m_lightsIndices;

		std::vector<ShaderResourceMapper> m_shaderResourceMappers;

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
