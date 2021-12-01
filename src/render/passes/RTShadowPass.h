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
		Texture2DPtr visibilityTex;
	};

	class RTShadowPass : public RenderPassBase
	{
	public:
		RTShadowPass(HashString name);
		~RTShadowPass();

		void Update()
		{
			HandlePreUpdate(nullptr);
		}
	protected:
		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;
	private:
		MessageSubscriber m_subscriber;

		RtShaderPtr m_rayGenShader;
		RtShaderPtr m_rayMissShader;

		Texture2DPtr m_visibilityTex;
		std::vector<TextureDataPtr> m_visibilityTextures;
		std::vector<ShaderResourceMapper> m_shaderResourceMappers;

		struct RtShadowPassFrameData
		{
			vk::Pipeline m_rtPipeline;
			vk::PipelineLayout m_rtPipelineLayout;
			std::vector<VulkanDescriptorSet> m_sets;
			std::vector<vk::DescriptorSet> m_nativeSets;
			vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProps;
			BufferDataPtr m_sbtBuffer;
			uint32_t m_handleSizeAligned;
		};
		std::vector<RtShadowPassFrameData> m_frameDataArray;

		void HandlePreUpdate(std::shared_ptr<GlobalPreFrameMessage> msg);
		void UpdateShaderResources();
		void UpdatePipeline();
	};

}

#endif
