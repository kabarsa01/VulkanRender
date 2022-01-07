#ifndef __SHADER_BINDING_TABLE_H__
#define __SHADER_BINDING_TABLE_H__

#include <vector>
#include <unordered_map>
#include <array>

#include "RtShader.h"
#include "vulkan/vulkan.hpp"
#include "common/HashString.h"
#include "Shader.h"
#include "data/RtMaterial.h"
#include "data/BufferData.h"

namespace CGE
{

	class ShaderBindingTable
	{
	public:
		ShaderBindingTable();
		~ShaderBindingTable();

		void AddShader(RtShaderPtr rtShader);
		void AddShader(ShaderPtr rtShader);
		void AddShaders(const std::vector<RtShaderPtr>& rtShaders);
		void SetShaders(const std::vector<RtShaderPtr>& rtShaders);
		void AddRtMaterial(RtMaterialPtr rtMaterial);
		void AddRtMaterials(const std::vector<RtMaterialPtr>& rtMaterials);
		void SetRtMaterials(const std::vector<RtMaterialPtr>& rtMaterials);
		void AddGlobalRtMaterial(RtMaterialPtr rtMaterial);
		void SetGlobalRtMaterials(const std::vector<RtMaterialPtr>& rtMaterials);

		std::vector<RtShaderPtr>& GetShaders() { return m_shaders; }
		std::vector<vk::PipelineShaderStageCreateInfo>& GetShaderStages() { return m_stages; }
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR>& GetShaderGroups() { return m_groups; }
		// offsets getters
		uint32_t GetGroupOffset(HashString id);
		uint32_t GetGroupTypeOffset(ERtShaderType type) { return m_groupTypeOffsets[ToInt(type)]; }
		uint32_t GetGroupTypeOffset(ERtShaderType type, HashString id) { return GetGroupOffset(id) - m_groupTypeOffsets[ToInt(type)]; }
		uint32_t GetGroupTypeOffsetBytes(ERtShaderType type) { return GetGroupTypeOffset(type) * m_handleSizeAlignedBytes; }
		// handle size
		uint32_t GetHandleSizeAlignedBytes() { return m_handleSizeAlignedBytes; }
		// regions
		vk::StridedDeviceAddressRegionKHR GetRegion(ERtShaderType type, HashString id);
		// buffer getter
		BufferDataPtr GetBuffer() { return m_sbtBuffer; }

		void Clear();
		void Update();
		void ConstructBuffer(vk::Pipeline raytracingPipeline, HashString name);
	private:
		std::vector<RtShaderPtr> m_shaders;
		std::vector<RtMaterialPtr> m_materials;
		std::vector<RtMaterialPtr> m_globalMaterials;
		std::vector<vk::PipelineShaderStageCreateInfo> m_stages;
		std::unordered_map<HashString, uint32_t> m_shaderStagesIndices;
		std::array<std::vector<RtShaderPtr>, static_cast<uint32_t>(ERtShaderType::RST_MAX)> m_shadersByType;
		// groups data
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_groups;
		std::unordered_map<HashString, uint32_t> m_groupIndices;
		std::array<uint32_t, static_cast<uint32_t>(ERtShaderType::RST_MAX) + 1> m_groupTypeOffsets;
		// handle size in bytes aligned - very important stuff
		uint32_t m_handleSizeAlignedBytes;
		// sbt buffer
		BufferDataPtr m_sbtBuffer;

		void FillGeneralShaderGroups(const std::vector<RtShaderPtr>& shaders, std::vector<vk::RayTracingShaderGroupCreateInfoKHR>& groups);
		vk::RayTracingShaderGroupCreateInfoKHR CreateGroupForMaterial(RtMaterialPtr rtMaterial);
	};

}

#endif

