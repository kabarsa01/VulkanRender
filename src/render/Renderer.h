#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <optional>
#include <core/ObjectBase.h>
#include "fwd.hpp"

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> computeFamily;
};

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
	uint32_t version;
	int width = 1280;
	int height = 720;

	VULKAN_HPP_NAMESPACE::Instance vulkanInstance;
	VULKAN_HPP_NAMESPACE::SurfaceKHR vulkanSurface;
	VULKAN_HPP_NAMESPACE::PhysicalDevice vulkanPhysicalDevice;
	VULKAN_HPP_NAMESPACE::Device vulkanDevice;
	VULKAN_HPP_NAMESPACE::Queue graphicsQueue;
	VULKAN_HPP_NAMESPACE::Queue computeQueue;
//	QueueFamilyIndices queueFamilies;

	//std::vector<RenderPassPtr> RenderPasses;
	//std::map<HashString, unsigned int> RenderPassMap;
	//==================== METHODS ===============================

	void PickPhysicalDevice(std::vector<VULKAN_HPP_NAMESPACE::PhysicalDevice>& InDevices);
	int ScoreDeviceSuitability(const VULKAN_HPP_NAMESPACE::PhysicalDevice& InPhysicalDevice);
	QueueFamilyIndices FindQueueFamilies(const VULKAN_HPP_NAMESPACE::PhysicalDevice& InPhysicalDevice);
	void CreateLogicalDevice();
//	void RegisterRenderPass(RenderPassPtr InRenderPass);
};

typedef std::shared_ptr<Renderer> RendererPtr;

