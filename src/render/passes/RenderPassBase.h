#ifndef __RENDER_PASS_BASE_H__
#define __RENDER_PASS_BASE_H__

#include <vector>
#include <unordered_map>

#include "vulkan/vulkan.hpp"
#include "common/HashString.h"
#include "RenderPassDataTable.h"
#include "../shader/ShaderResourceMapper.h"
#include "data/Texture2D.h"

namespace CGE
{

	class PassInitContext;
	class PassExecuteContext;

	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------

	class RenderPassBase
	{
	public:
		RenderPassBase() = delete;
		RenderPassBase(HashString name) : m_name(name) {}
		~RenderPassBase() {}

		void Init();
		void Execute(vk::CommandBuffer* commandBuffer);
	protected:
		virtual void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) {}
		virtual void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) {}
	private:
		friend class PassExecuteContext;

		HashString m_name;

		class VulkanDevice* m_device;
		class Renderer* m_renderer;

		vk::RenderPass m_renderPass;
		std::vector<vk::Framebuffer> m_framebuffers;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		vk::RenderPass CreateRenderPass(const PassInitContext& initContext);
		std::vector<vk::Framebuffer> CreateFramebuffers(const PassInitContext& initContext);
	};

	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------

	// Data transfer object or mediator maybe representing render pass init data interface.
	// to make init options and actions clear and obvious for RenderPassBase derived classes
	class PassInitContext
	{
	public:
		// set an array of attachments to a specified index, minimum 2 are needed
		// for round robin usage for double buffering
		void SetAttachments(uint32_t index, std::vector<Texture2DPtr> attachmentArray);
		// set an array of depth attachments, minimum 2 are needed
		// for round robin usage for double buffering
		void SetDepthAttachments(std::vector<Texture2DPtr> depthAttachmentArray);
	private:
		friend class RenderPassBase;

		std::unordered_map<uint32_t, std::vector<Texture2DPtr>> m_attachments;
		std::vector<Texture2DPtr> m_depthAttachments;
	};

	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------

	class PassExecuteContext
	{
	public:
		PassExecuteContext() = delete;
		PassExecuteContext(RenderPassBase* owner) : m_owner(owner) {}

		vk::RenderPass GetRenderPass();
		vk::Framebuffer GetFramebuffer(uint32_t frameIndex = UINT32_MAX);
	private:
		friend class RenderPassBase;

		RenderPassBase* m_owner;
	};

}

#endif
