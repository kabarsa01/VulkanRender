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
		assert(rtShader->IsGeneral() && "General raytracing shader expected");
		m_shaders.emplace_back(rtShader);
	}

	void ShaderBindingTable::AddShader(ShaderPtr rtShader)
	{
		RtShaderPtr shaderCast = ObjectBase::Cast<RtShader>(rtShader);
		assert(shaderCast->IsGeneral() && "General raytracing shader expected");
		m_shaders.emplace_back(shaderCast);
	}

	void ShaderBindingTable::AddShaders(const std::vector<RtShaderPtr>& rtShaders)
	{
		for (uint32_t idx = 0; idx < rtShaders.size(); ++idx)
		{
			m_shaders.push_back(rtShaders[idx]);
		}
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

	void ShaderBindingTable::Clear()
	{
		// clear shaders and materials
		m_shaders.clear();
		m_materials.clear();
		// clear produced cached structures
		m_groups.clear();
		m_stages.clear();
		m_shaderIndices.clear();
		m_materialGroupIndices.clear();
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
		m_shaderIndices.clear();
		m_materialGroupIndices.clear();
		for (auto& shaderList : m_shadersByType)
		{
			shaderList.clear();
		}

		for (uint32_t idx = 0; idx < m_shaders.size(); ++idx)
		{
			RtShaderPtr shader = m_shaders[idx];
			m_shadersByType[shader->GetTypeIntegral()].push_back(shader);
			m_shaderIndices[shader->GetResourceId()] = idx;

			vk::PipelineShaderStageCreateInfo stageInfo;
			stageInfo.setModule(shader->GetShaderModule());
			stageInfo.setPName("main");
			stageInfo.setStage(shader->GetStageFlags());
			m_stages.push_back(stageInfo);
		}

		// process ray gen shaders
		FillGeneralShaderGroups(m_shadersByType[ToInt(ERtShaderType::RST_RAY_GEN)], m_groups);
		// remember miss groups offset
		m_missGroupsOffset = static_cast<uint32_t>(m_groups.size());
		// process miss shaders
		FillGeneralShaderGroups(m_shadersByType[ToInt(ERtShaderType::RST_MISS)], m_groups);
		// remember hit groups offset
		m_hitGroupsOffset = static_cast<uint32_t>(m_groups.size());
		// process materials as hit groups
		for (RtMaterialPtr rtMaterial : m_materials)
		{
			if (!rtMaterial->HasHitGroup())
			{
				continue;
			}
			RtShaderPtr intersect = rtMaterial->GetShader(ERtShaderType::RST_INTERSECT);
			RtShaderPtr anyHit = rtMaterial->GetShader(ERtShaderType::RST_ANY_HIT);
			RtShaderPtr closestHit = rtMaterial->GetShader(ERtShaderType::RST_CLOSEST_HIT);

			vk::RayTracingShaderGroupTypeKHR type = intersect ? vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup : vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;

			vk::RayTracingShaderGroupCreateInfoKHR groupInfo;
			groupInfo.setType(type);
			groupInfo.setGeneralShader(VK_SHADER_UNUSED_KHR);
			groupInfo.setIntersectionShader(intersect ? m_shaderIndices[intersect->GetResourceId()] : VK_SHADER_UNUSED_KHR);
			groupInfo.setAnyHitShader(anyHit ? m_shaderIndices[anyHit->GetResourceId()] : VK_SHADER_UNUSED_KHR);
			groupInfo.setClosestHitShader(closestHit ? m_shaderIndices[closestHit->GetResourceId()] : VK_SHADER_UNUSED_KHR);

			m_materialGroupIndices[rtMaterial->GetResourceId()] = static_cast<uint32_t>(m_groups.size());
			m_groups.push_back(groupInfo);
		}
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
		uint32_t groupCount = static_cast<uint32_t>(rtScene->GetGlobalShaderBindingTable().GetShaderGroups().size());
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
			groupInfo.setGeneralShader(m_shaderIndices[shader->GetResourceId()]);
			groupInfo.setIntersectionShader(VK_SHADER_UNUSED_KHR);
			groupInfo.setAnyHitShader(VK_SHADER_UNUSED_KHR);
			groupInfo.setClosestHitShader(VK_SHADER_UNUSED_KHR);

			groups.push_back(groupInfo);
		}
	}

}

