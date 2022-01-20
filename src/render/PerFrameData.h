#pragma once

#include "render/DataStructures.h"
#include "objects/VulkanDescriptorSet.h"
#include "vulkan/vulkan.hpp"
#include "data/BufferData.h"
#include "core/Engine.h"
#include "shader/ShaderResourceMapper.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::WriteDescriptorSet;

	class PerFrameData
	{
	public:
		PerFrameData();
		virtual ~PerFrameData();
	
		void Create(VulkanDevice* inDevice);
		void Destroy();
		void UpdateBufferData();
		VulkanDescriptorSet& GetVulkanDescriptorSet() { return GetData().m_set; }
		DescriptorSet& GetSet() { return GetData().m_set.GetSet(); }
		DescriptorSetLayout& GetLayout() { return GetData().m_set.GetLayout(); }
	private:
		VulkanDevice* device;

		BufferDataPtr m_globalDataBuffer;
		BufferDataPtr m_globalPreviousDataBuffer;
		BufferDataPtr m_transformDataBuffer;
		BufferDataPtr m_transformPreviousDataBuffer;
	
		struct FrameData
		{
			VulkanDescriptorSet m_set;
			ShaderResourceMapper resourceMapper;
		};
		std::vector<FrameData> m_data;
		
		GlobalShaderData* m_globalShaderData;
		GlobalTransformData* m_globalTransformData;
		GlobalTransformData* m_globalPreviousTransformData;
		uint64_t m_relevantTransformsSize = 0;
	
		FrameData& GetData() { return m_data[Engine::GetFrameIndex(m_data.size())]; }
		void GatherData();
	};
}
