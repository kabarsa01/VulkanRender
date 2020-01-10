#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <core/ObjectBase.h>

class Renderer : public ObjectBase
{
public:
	Renderer();
	virtual ~Renderer();

	virtual void OnInitialize() override;

	void Init();
	void RenderFrame();

	void SetResolution(int InWidth, int InHeight);
	int GetWidth() const;
	int GetHeight() const;
protected:
private:
	//======================= VARS ===============================
	int Width = 1280;
	int Height = 720;

	VkInstance Instance;

	//std::vector<RenderPassPtr> RenderPasses;
	//std::map<HashString, unsigned int> RenderPassMap;
	//==================== METHODS ===============================

//	void RegisterRenderPass(RenderPassPtr InRenderPass);
};

typedef std::shared_ptr<Renderer> RendererPtr;

