#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <core/ObjectBase.h>
#include "fwd.hpp"

class Renderer : public ObjectBase
{
public:
	Renderer();
	virtual ~Renderer();

	virtual void OnInitialize() override;

	void Init();
	void RenderFrame();
	void Cleanup();

	void SetResolution(int InWidth, int InHeight);
	int GetWidth() const;
	int GetHeight() const;
protected:
private:
	//======================= VARS ===============================
	uint32_t Version;
	int Width = 1280;
	int Height = 720;

	VULKAN_HPP_NAMESPACE::Instance VulkanInstance;
	VULKAN_HPP_NAMESPACE::PhysicalDevice VulkanPhysicalDevice;

	//std::vector<RenderPassPtr> RenderPasses;
	//std::map<HashString, unsigned int> RenderPassMap;
	//==================== METHODS ===============================

	void PickPhysicalDevice(std::vector<VULKAN_HPP_NAMESPACE::PhysicalDevice>& InDevices);
	int ScoreDeviceSuitability(const VULKAN_HPP_NAMESPACE::PhysicalDevice& InPhysicalDevice);
//	void RegisterRenderPass(RenderPassPtr InRenderPass);
};

typedef std::shared_ptr<Renderer> RendererPtr;

