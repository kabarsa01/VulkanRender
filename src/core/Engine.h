#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/ObjectBase.h"
#include "scene/Scene.h"
#include "render/Renderer.h"

class Engine
{
public:
	static Engine* GetInstance();
	static ScenePtr GetSceneInstance();
	static RendererPtr GetRendererInstance();

	void Run();

	ScenePtr GetScene();
	RendererPtr GetRenderer();
	GLFWwindow* GetGlfwWindow();
protected:
	ScenePtr sceneInstance;
	RendererPtr rendererInstance;

	void Init();
	void MainLoop();
	void Cleanup();

	void InitWindow();
	static void FramebufferResizeCallback(GLFWwindow* inWindow, int inWidth, int inHeight);
private:
	static Engine* staticInstance;

	GLFWwindow* window;
	int windowWidth = 1280;
	int windowHeight = 720;

	Engine();
	Engine(const Engine& inOther) {}
	void operator=(const Engine& inOther) {}
	virtual ~Engine();
};
