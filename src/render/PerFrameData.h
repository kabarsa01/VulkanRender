#pragma once

#include "render/DataStructures.h"
#include "objects/VulkanDescriptorSet.h"
#include "vulkan/vulkan.hpp"
#include "data/BufferData.h"
#include "core/Engine.h"

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
	
		struct FrameData
		{
			BufferDataPtr shaderDataBuffer;
			BufferDataPtr transformDataBuffer;
			DescriptorSetLayoutBinding shaderDataBinding;
			DescriptorSetLayoutBinding transformDataBinding;
			VulkanDescriptorSet m_set;
			std::vector<WriteDescriptorSet> descriptorWrites;
		};
		std::vector<FrameData> m_data;
	
		GlobalShaderData* globalShaderData;
		GlobalTransformData* globalTransformData;
	
		FrameData& GetData() { return m_data[Engine::GetFrameIndex(m_data.size())]; }
		std::vector<DescriptorSetLayoutBinding> ProduceBindings(FrameData& frameData);
		std::vector<WriteDescriptorSet> ProduceWrites(FrameData& frameData);
		void GatherData();
	};
}
