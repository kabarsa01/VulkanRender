#ifndef __RENDER_PASS_BASE_H__
#define __RENDER_PASS_BASE_H__

#include <vector>

#include "vulkan/vulkan.hpp"
#include "common/HashString.h"
#include "RenderPassDataTable.h"

namespace CGE
{

	class RenderPassBase
	{
	public:
		RenderPassBase() = delete;
		RenderPassBase(HashString name) : m_name(name) {}
		~RenderPassBase() {}

		void Init(RenderPassDataTable& dataTable) {}
		void Execute() {}
	private:
		HashString m_name;

		class VulkanDevice* m_device;
		class Renderer* m_renderer;

		vk::RenderPass m_renderPass;
		std::vector<vk::Framebuffer> m_framebuffers;
		std::vector<HashString> m_attachments;
		HashString m_depthAttachment;

		uint32_t width = 1280;
		uint32_t height = 720;
	};

}

#endif
