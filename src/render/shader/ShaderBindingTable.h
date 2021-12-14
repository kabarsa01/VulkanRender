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
		void AddRtMaterial(RtMaterialPtr rtMaterial);
		void AddRtMaterials(const std::vector<RtMaterialPtr>& rtMaterials);

		std::vector<RtShaderPtr>& GetShaders() { return m_shaders; }
		std::vector<vk::PipelineShaderStageCreateInfo>& GetShaderStages() { return m_stages; }
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR>& GetShaderGroups() { return m_groups; }
		// offsets getters
		uint32_t GetMissGroupsOffset() { return m_missGroupsOffset; }
		uint32_t GetMissGroupsOffsetBytes() { return m_handleSizeAlignedBytes * GetMissGroupsOffset(); }
		uint32_t GetHitGroupsOffset() { return m_hitGroupsOffset; }
		uint32_t GetHitGroupsOffsetBytes() { return m_handleSizeAlignedBytes * GetHitGroupsOffset(); }
		uint32_t GetHitGroupOffset(RtMaterialPtr material) { return m_materialGroupIndices[material->GetResourceId()] - m_hitGroupsOffset; }
		uint32_t GetHitGroupOffsetBytes(RtMaterialPtr material) { return m_handleSizeAlignedBytes * GetHitGroupOffset(material); }
		// sizes and strides getters
		uint32_t GetRayGenGroupsSize() { return m_missGroupsOffset; }
		uint32_t GetRayGenGroupsSizeBytes() { return m_handleSizeAlignedBytes * GetRayGenGroupsSize(); }
		uint32_t GetMissGroupsSize() { return m_hitGroupsOffset - m_missGroupsOffset; }
		uint32_t GetMissGroupsSizeBytes() { return m_handleSizeAlignedBytes * GetMissGroupsSize(); }
		uint32_t GetHitGroupsSize() { return static_cast<uint32_t>(m_groups.size()) - m_hitGroupsOffset; }
		uint32_t GetHitGroupsSizeBytes() { return m_handleSizeAlignedBytes * GetHitGroupsSize(); }
		uint32_t GetHandleSizeAlignedBytes() { return m_handleSizeAlignedBytes; }
		// buffer getter
		BufferDataPtr GetBuffer() { return m_sbtBuffer; }

		void Clear();
		void Update();
		void ConstructBuffer(vk::Pipeline raytracingPipeline, HashString name);
	private:
		std::vector<RtShaderPtr> m_shaders;
		std::vector<RtMaterialPtr> m_materials;
		std::vector<vk::PipelineShaderStageCreateInfo> m_stages;
		std::unordered_map<HashString, uint32_t> m_shaderIndices;
		std::array<std::vector<RtShaderPtr>, static_cast<uint32_t>(ERtShaderType::RST_MAX)> m_shadersByType;
		// groups data
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_groups;
		std::unordered_map<HashString, uint32_t> m_materialGroupIndices;
		uint32_t m_missGroupsOffset;
		uint32_t m_hitGroupsOffset;
		uint32_t m_handleSizeAlignedBytes;
		// sbt buffer
		BufferDataPtr m_sbtBuffer;

		void FillGeneralShaderGroups(const std::vector<RtShaderPtr>& shaders, std::vector<vk::RayTracingShaderGroupCreateInfoKHR>& groups);
	};

}

#endif

