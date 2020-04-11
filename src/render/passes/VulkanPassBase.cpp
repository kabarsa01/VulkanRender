#include "VulkanPassBase.h"

VulkanPassBase::VulkanPassBase()
{

}

VulkanPassBase::~VulkanPassBase()
{

}

void VulkanPassBase::Create(VulkanDevice* inDevice)
{
	vulkanDevice = inDevice;

	Device& device = inDevice->GetDevice();

	AttachmentDescription colorAttachDesc;
	colorAttachDesc.setFormat(Format::eR16G16B16A16Sfloat);
	colorAttachDesc.setSamples(SampleCountFlagBits::e1);
	colorAttachDesc.setLoadOp(AttachmentLoadOp::eClear);
	colorAttachDesc.setStoreOp(AttachmentStoreOp::eStore);
	colorAttachDesc.setStencilLoadOp(AttachmentLoadOp::eDontCare);
	colorAttachDesc.setStencilStoreOp(AttachmentStoreOp::eDontCare);
	colorAttachDesc.setInitialLayout(ImageLayout::eUndefined);
	colorAttachDesc.setFinalLayout(ImageLayout::eColorAttachmentOptimal);

	AttachmentReference colorAttachRef;
	colorAttachRef.setAttachment(0);
	colorAttachRef.setLayout(ImageLayout::eColorAttachmentOptimal);

	SubpassDescription subpassDesc;
	subpassDesc.setColorAttachmentCount(1);
	subpassDesc.setPColorAttachments(&colorAttachRef);
	subpassDesc.setPipelineBindPoint(PipelineBindPoint::eGraphics);

	RenderPassCreateInfo renderPassInfo;
	renderPassInfo.setAttachmentCount(1);
	renderPassInfo.setPAttachments(&colorAttachDesc);
	renderPassInfo.setSubpassCount(1);
	renderPassInfo.setPSubpasses(&subpassDesc);

	renderPass = device.createRenderPass(renderPassInfo);
}

void VulkanPassBase::Destroy()
{

}

void VulkanPassBase::Draw(CommandBuffer* inCmdBuffer)
{

}

