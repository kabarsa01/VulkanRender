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
			Texture2DPtr lightingTexture;
			vk::PipelineLayout pipelineLayout;
			vk::Pipeline pipeline;
		};

		MessageSubscriber m_subscriber;

		std::vector<RTGIPassFrameData> m_frameData;
		std::vector<Texture2DPtr> m_lightingData;
		RtShaderPtr m_rayGen;
		RtShaderPtr m_rayMiss;
		RtShaderPtr m_closestHit;
		RtMaterialPtr m_globalRTGIMaterial;

		MeshDataPtr m_hemisphereSamplingPoints;
		BufferDataPtr m_samplingPointsBuffer;

		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;

		void HandleUpdate(std::shared_ptr<GlobalPostSceneMessage> msg);
	};

}

#endif
