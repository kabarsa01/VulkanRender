#ifndef __UPDATE_GI_PROBES_PASS_H__
#define __UPDATE_GI_PROBES_PASS_H__


#include "RenderPassBase.h"
#include "common/HashString.h"
#include "vulkan/vulkan.hpp"
#include "RenderPassDataTable.h"
#include <vector>
#include "data/Material.h"


namespace CGE
{

	struct UpdateGIProbesData : public Identifiable<UpdateGIProbesData>
	{
		std::vector<Texture2DPtr> screenProbesData;
		std::vector<Texture2DPtr> irradianceData;
	};

	class UpdateGIProbesPass : public RenderPassBase
	{
	public:
		UpdateGIProbesPass(HashString name);
		~UpdateGIProbesPass();
	protected:
		std::vector<MaterialPtr> m_giProbesComputeMaterials;
		std::vector<MaterialPtr> m_irradianceComputeMaterials;

		struct FrameData
		{
			Texture2DPtr depth;
			Texture2DPtr normal;
			Texture2DPtr velocity;
			Texture2DPtr prevDepth;
			Texture2DPtr prevNormal;
			Texture2DPtr screenProbes;
			Texture2DPtr prevScreenProbes;
			Texture2DPtr irradiance;
			Texture2DPtr prevIrradiance;
		};
		std::vector<FrameData> m_frameData;

		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;
	};

}

#endif
