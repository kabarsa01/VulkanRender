#ifndef __RT_GI_PASS_H__
#define __RT_GI_PASS_H__

#include <vulkan/vulkan.hpp>

#include "messages/MessageSubscriber.h"
#include "RenderPassBase.h"
#include <vector>
#include "utils/Identifiable.h"
#include "../shader/ShaderBindingTable.h"
#include "messages/Messages.h"
#include "data/MeshData.h"

namespace CGE
{

	struct RTGIPassData : public Identifiable<RTGIPassData>
	{
		std::vector<Texture2DPtr> lightingData;
		std::vector<Texture2DPtr> irradianceData;
		std::vector<Texture2DPtr> giDepthData;
		BufferDataPtr probeGridBuffer;
		Texture2DPtr probeGridTexture;
		Texture2DPtr probeGridDepthTexture;
	};

	class RTGIPass : public RenderPassBase
	{
	public:
		RTGIPass(HashString);
		~RTGIPass();

		void Update();
	protected:
		struct RTGIPassFrameData
		{
			ShaderBindingTable sbt;
			ShaderResourceMapper resourceMapper;
			vk::PipelineLayout pipelineLayout;
			vk::Pipeline pipeline;
		};

		MessageSubscriber m_subscriber;

		std::vector<RTGIPassFrameData> m_frameData;
		std::vector<Texture2DPtr> m_lightingData;
		std::vector<Texture2DPtr> m_irradianceData;
		std::vector<Texture2DPtr> m_giDepthData;
		BufferDataPtr m_probeGridBuffer;
		Texture2DPtr m_probeGridTexture;
		Texture2DPtr m_probeGridDepthTexture;
		Texture2DPtr m_temporalCounter;
		RtShaderPtr m_rayGen;
		RtShaderPtr m_rayGenDDGI;
		RtShaderPtr m_rayMiss;
		RtShaderPtr m_closestHit;
		RtMaterialPtr m_globalRTGIMaterial;

		vk::MemoryBarrier m_memBar;

		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;

		void HandleUpdate(std::shared_ptr<GlobalPostSceneMessage> msg);

		void CreateProbeGridData();
	};

}

#endif
