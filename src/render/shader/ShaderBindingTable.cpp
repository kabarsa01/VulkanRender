#include "ShaderBindingTable.h"
#include "core/ObjectBase.h"
#include "data/RtMaterial.h"
#include "data/DataManager.h"
#include "../objects/VulkanDevice.h"
#include "core/Engine.h"
#include "../Renderer.h"
#include "../RtScene.h"
#include "utils/ResourceUtils.h"
#include "utils/Singleton.h"

namespace CGE
{

	ShaderBindingTable::ShaderBindingTable()
	{

	}

	ShaderBindingTable::~ShaderBindingTable()
	{

	}

	void ShaderBindingTable::AddShader(RtShaderPtr rtShader)
	{
		m_shaders.emplace_back(rtShader);
	}

	void ShaderBindingTable::AddShader(ShaderPtr rtShader)
	{
		m_shaders.emplace_back(ObjectBase::Cast<RtShader>(rtShader));
	}

	void ShaderBindingTable::AddShaders(const std::vector<RtShaderPtr>& rtShaders)
	{
		for (uint32_t idx = 0; idx < rtShaders.size(); ++idx)
		{
			m_shaders.push_back(rtShaders[idx]);
		}
	}

	void ShaderBindingTable::SetShaders(const std::vector<RtShaderPtr>& rtShaders)
	{
		m_shaders = rtShaders;
	}

	void ShaderBindingTable::AddRtMaterial(RtMaterialPtr rtMaterial)
	{
		m_materials.emplace_back(rtMaterial);
	}

	void ShaderBindingTable::AddRtMaterials(const std::vector<RtMaterialPtr>& rtMaterials)
	{
		for (uint32_t idx = 0; idx < rtMaterials.size(); ++idx)
		{
			m_materials.push_back(rtMaterials[idx]);
		}
	}

	void ShaderBindingTable::SetRtMaterials(const std::vector<RtMaterialPtr>& rtMaterials)
	{
		m_materials = rtMaterials;
	}

	void ShaderBindingTable::AddGlobalRtMaterial(RtMaterialPtr rtMaterial)
	{
		m_globalMaterials.emplace_back(rtMaterial);
	}

	void ShaderBindingTable::SetGlobalRtMaterials(const std::vector<RtMaterialPtr>& rtMaterials)
	{
		for (uint32_t idx = 0; idx < rtMaterials.size(); ++idx)
		{
			m_globalMaterials.push_back(rtMaterials[idx]);
		}
	}

	uint32_t ShaderBindingTable::GetGroupOffset(HashString id)
	{
		auto it = m_groupIndices.find(id);
		if (it != m_groupIndices.end())
		{
			return it->second;
		}
		return UINT32_MAX;
	}

	vk::StridedDeviceAddressRegionKHR ShaderBindingTable::GetRegion(ERtShaderType type, HashString id)
	{
		uint32_t typeOffset = m_groupTypeOffsets[ToInt(type)];
		uint32_t nextTypeIndex = type > ERtShaderType::RST_MISS ? ToInt(ERtShaderType::RST_MAX) : ToInt(type) + 1;
		uint32_t nextTypeOffset = m_groupTypeOffsets[nextTypeIndex];
		uint32_t groupOffsetIndex = GetGroupOffset(id);
		if (groupOffsetIndex == UINT32_MAX)
		{
			groupOffsetIndex = typeOffset;
		}

		uint32_t groupSizeBytes = (nextTypeOffset - groupOffsetIndex) * m_handleSizeAlignedBytes;

		vk::StridedDeviceAddressRegionKHR region;
		region.setDeviceAddress(m_sbtBuffer->GetDeviceAddress() + groupOffsetIndex * m_handleSizeAlignedBytes);
		region.setSize(type == ERtShaderType::RST_RAY_GEN ? GetHandleSizeAlignedBytes() : groupSizeBytes);
		region.setStride(GetHandleSizeAlignedBytes());

		return region;
	}

	void ShaderBindingTable::Clear()
	{
		// clear shaders and materials
		m_shaders.clear();
		m_materials.clear();
		// clear produced cached structures
		m_groups.clear();
		m_stages.clear();
		m_shaderStagesIndices.clear();
		m_groupIndices.clear();
		for (auto& shaderList : m_shadersByType)
		{
			shaderList.clear();
		}
	}

	void ShaderBindingTable::Update()
	{
		// clear only produces values
		m_groups.clear();
		m_stages.clear();
		m_shaderStagesIndices.clear();
		m_groupIndices.clear();
		for (auto& shaderList : m_shadersByType)
		{
			shaderList.clear();
		}

		for (uint32_t idx = 0; idx < m_shaders.size(); ++idx)
		{
			RtShaderPtr shader = m_shaders[idx];
			m_shadersByType[shader->GetTypeIntegral()].push_back(shader);
			m_shaderStagesIndices[shader->GetResourceId()] = idx;

			vk::PipelineShaderStageCreateInfo stageInfo;
			stageInfo.setModule(shader->GetShaderModule());
			stageInfo.setPName("main");
			stageInfo.setStage(shader->GetStageFlags());
			m_stages.push_back(stageInfo);
		}

		// process ray gen shaders
		m_groupTypeOffsets[ToInt(ERtShaderType::RST_RAY_GEN)] = 0;
		FillGeneralShaderGroups(m_shadersByType[ToInt(ERtShaderType::RST_RAY_GEN)], m_groups);
		// remember miss groups offset
		m_groupTypeOffsets[ToInt(ERtShaderType::RST_MISS)] = static_cast<uint32_t>(m_groups.size());
		// process miss shaders
		FillGeneralShaderGroups(m_shadersByType[ToInt(ERtShaderType::RST_MISS)], m_groups);
		// remember hit groups offset
		m_groupTypeOffsets[ToInt(ERtShaderType::RST_INTERSECT)] = static_cast<uint32_t>(m_groups.size());
		m_groupTypeOffsets[ToInt(ERtShaderType::RST_ANY_HIT)] = static_cast<uint32_t>(m_groups.size());
		m_groupTypeOffsets[ToInt(ERtShaderType::RST_CLOSEST_HIT)] = static_cast<uint32_t>(m_groups.size());
		// process materials as hit groups
		for (RtMaterialPtr rtMaterial : m_materials)
		{
			if (!rtMaterial->HasHitGroup())
			{
				continue;
			}

			vk::RayTracingShaderGroupCreateInfoKHR groupInfo = CreateGroupForMaterial(rtMaterial);
			m_groupIndices[rtMaterial->GetResourceId()] = static_cast<uint32_t>(m_groups.size());
			m_groups.push_back(groupInfo);

			// for each hit group create a set of trailing global hit group records
			// so that each instance could use other set of hit shaders by using
			// sbt offset for different render passes and so on. on my machine group
			// handle size is 64 bytes, so even for 1000 raytracing materials (which is veeeee-eeeee-eeeery unlikely)
			// having several global materials for GI, Shadows and so on easily affordable
			for (RtMaterialPtr globalMaterial : m_globalMaterials)
			{
				m_groups.push_back(CreateGroupForMaterial(globalMaterial));
			}
		}
		m_groupTypeOffsets[ToInt(ERtShaderType::RST_MAX)] = static_cast<uint32_t>(m_groups.size());
	}

	void ShaderBindingTable::ConstructBuffer(vk::Pipeline raytracingPipeline, HashString name)
	{
		VulkanDevice* device = &Engine::GetRendererInstance()->GetVulkanDevice();
		vk::Device& nativeDevice = device->GetDevice();
		RtScene* rtScene = Singleton<RtScene>::GetInstance();

		vk::StructureChain<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR> structChain =
			device->GetPhysicalDevice().GetDevice().getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
		auto rtProps = structChain.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
		uint32_t handleSize = rtProps.shaderGroupHandleSize;
		uint32_t alignment = rtProps.shaderGroupBaseAlignment;
		// nvidia recommended to use base alignment
		// we avoid using power of two version formula just in case. who knows
		m_handleSizeAlignedBytes = alignment * ((handleSize + alignment - 1) / alignment);
		uint32_t groupCount = static_cast<uint32_t>(m_groups.size());
		uint64_t sbtSize = groupCount * m_handleSizeAlignedBytes;
		std::vector<char> shadersHandles(sbtSize);

		auto handlesResult = nativeDevice.getRayTracingShaderGroupHandlesKHR(raytracingPipeline, 0, groupCount, sbtSize, shadersHandles.data());
		if (handlesResult != vk::Result::eSuccess)
		{
			// TODO
		}
		if (m_sbtBuffer)
		{
			m_sbtBuffer->DestroyHint();
		}
		m_sbtBuffer = ResourceUtils::CreateBufferData(
			name + std::to_string(Engine::GetInstance()->GetFrameCount()),
			sbtSize,
			vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eTransferDst,
			true
		);
		if (sbtSize > 0)
		{
			std::vector<char> alignedShadersHandles(sbtSize);
			char* addr = alignedShadersHandles.data();
			for (uint32_t idx = 0; idx < groupCount; idx++)
			{
				memcpy(addr, shadersHandles.data() + handleSize * idx, handleSize);
				addr += m_handleSizeAlignedBytes;
			}
			m_sbtBuffer->CopyTo(sbtSize, alignedShadersHandles.data());
		}
	}

	void ShaderBindingTable::FillGeneralShaderGroups(const std::vector<RtShaderPtr>& shaders, std::vector<vk::RayTracingShaderGroupCreateInfoKHR>& groups)
	{
		for (RtShaderPtr shader : shaders)
		{
			vk::RayTracingShaderGroupCreateInfoKHR groupInfo;

			groupInfo.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral);
			groupInfo.setGeneralShader(m_shaderStagesIndices[shader->GetResourceId()]);
			groupInfo.setIntersectionShader(VK_SHADER_UNUSED_KHR);
			groupInfo.setAnyHitShader(VK_SHADER_UNUSED_KHR);
			groupInfo.setClosestHitShader(VK_SHADER_UNUSED_KHR);

			m_groupIndices[shader->GetResourceId()] = static_cast<uint32_t>(m_groups.size());
			groups.push_back(groupInfo);
		}
	}

	vk::RayTracingShaderGroupCreateInfoKHR ShaderBindingTable::CreateGroupForMaterial(RtMaterialPtr rtMaterial)
	{
		if (!rtMaterial || !rtMaterial->HasHitGroup())
		{
			return {};
		}
		RtShaderPtr intersect = rtMaterial->GetShader(ERtShaderType::RST_INTERSECT);
		RtShaderPtr anyHit = rtMaterial->GetShader(ERtShaderType::RST_ANY_HIT);
		RtShaderPtr closestHit = rtMaterial->GetShader(ERtShaderType::RST_CLOSEST_HIT);

		vk::RayTracingShaderGroupTypeKHR type = intersect ? vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup : vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;

		vk::RayTracingShaderGroupCreateInfoKHR groupInfo;
		groupInfo.setType(type);
		groupInfo.setGeneralShader(VK_SHADER_UNUSED_KHR);
		groupInfo.setIntersectionShader(intersect ? m_shaderStagesIndices[intersect->GetResourceId()] : VK_SHADER_UNUSED_KHR);
		groupInfo.setAnyHitShader(anyHit ? m_shaderStagesIndices[anyHit->GetResourceId()] : VK_SHADER_UNUSED_KHR);
		groupInfo.setClosestHitShader(closestHit ? m_shaderStagesIndices[closestHit->GetResourceId()] : VK_SHADER_UNUSED_KHR);

		return groupInfo;
	}

}

