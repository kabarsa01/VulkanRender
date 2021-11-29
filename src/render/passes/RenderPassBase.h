#ifndef __RENDER_PASS_BASE_H__
#define __RENDER_PASS_BASE_H__

#include <vector>
#include <unordered_map>

#include "vulkan/vulkan.hpp"
#include "common/HashString.h"
#include "RenderPassDataTable.h"
#include "../shader/ShaderResourceMapper.h"
#include "data/Texture2D.h"
#include "data/Material.h"
#include "../PipelineRegistry.h"

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
		RenderPassBase(HashString name);
		~RenderPassBase();

		void Init();
		void Execute(vk::CommandBuffer* commandBuffer);
	protected:
		virtual void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) = 0;
		virtual void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) = 0;
	private:
		friend class PassInitContext;
		friend class PassExecuteContext;

		HashString m_name;

		class VulkanDevice* m_device;
		class Renderer* m_renderer;

		vk::RenderPass m_renderPass;
		std::vector<vk::Framebuffer> m_framebuffers;

		PassInitContext* m_initContext;
		PassExecuteContext* m_executeContext;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		vk::RenderPass CreateRenderPass(const PassInitContext& initContext);
		std::vector<vk::Framebuffer> CreateFramebuffers(const PassInitContext& initContext);
		vk::Pipeline CreateGraphicsPipeline(const PassInitContext& initContext, MaterialPtr material, vk::PipelineLayout layout);
		vk::Pipeline CreateComputePipeline(const PassInitContext& initContext, MaterialPtr material, vk::PipelineLayout layout);
		vk::PipelineLayout CreatePipelineLayout(std::vector<DescriptorSetLayout>& descriptorSetLayouts);
		PipelineData& CreateOrFindPipeline(const PassInitContext& initContext, MaterialPtr material);
	};

	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------

	// Data transfer object or mediator maybe representing render pass init data interface.
	// to make init options and actions clear and obvious for RenderPassBase derived classes
	class PassInitContext
	{
	public:
		bool compute = false;
		vk::PipelineDepthStencilStateCreateInfo depthInfo;
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo;

		PassInitContext() = delete;
		PassInitContext(RenderPassBase* owner);
		// set an array of attachments to a specified index, minimum 2 are needed
		// for round robin usage for double buffering
		void SetAttachments(uint32_t attachmentIndex, const std::vector<Texture2DPtr>& attachmentArray, bool clearAttachment);
		// set an array of depth attachments, minimum 2 are needed
		// for round robin usage for double buffering
		void SetDepthAttachments(const std::vector<Texture2DPtr>& depthAttachmentArray, bool clearDepth);

		HashString GetPassName() { return m_owner->m_name; }
		uint32_t GetWidth() { return m_owner->m_width; }
		uint32_t GetHeight() { return m_owner->m_height; }
	private:
		friend class RenderPassBase;

		RenderPassBase* m_owner;

		std::unordered_map<uint32_t, std::vector<Texture2DPtr>> m_attachments;
		std::vector<Texture2DPtr> m_depthAttachments;
		bool m_clearAttachment;
		bool m_clearDepth;
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
		const std::vector<Texture2DPtr>& GetFrameAttachments(uint32_t frameIndex = UINT32_MAX);
		const std::unordered_map<uint32_t, std::vector<Texture2DPtr>>& GetAllAttachments();
		Texture2DPtr GetDepthAttachment(uint32_t frameIndex = UINT32_MAX);
		const std::vector<Texture2DPtr>& GetDepthAttachments();

		HashString GetPassName() { return m_owner->m_name; }
		uint32_t GetWidth() { return m_owner->m_width; }
		uint32_t GetHeight() { return m_owner->m_height; }

		PipelineData& FindPipeline(MaterialPtr material);
	private:
		friend class RenderPassBase;

		RenderPassBase* m_owner;

		std::unordered_map<uint32_t, std::vector<Texture2DPtr>> m_attachments;
		std::vector<Texture2DPtr> m_depthAttachments;
	};

}

#endif
