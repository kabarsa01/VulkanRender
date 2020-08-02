#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/ObjectBase.h"
#include "scene/Scene.h"

class Renderer;

class Engine
{
public:
	static Engine* GetInstance();
	static ScenePtr GetSceneInstance();
	static Renderer* GetRendererInstance();

	void Run();

	ScenePtr GetScene();
	Renderer* GetRenderer();
	GLFWwindow* GetGlfwWindow();
protected:
	ScenePtr sceneInstance;
	Renderer* rendererInstance;

	void Init();
	void MainLoop();
	void Cleanup();

	void InitWindow();
	static void FramebufferResizeCallback(GLFWwindow* inWindow, int inWidth, int inHeight);
private:
	static Engine* staticInstance;

	GLFWwindow* window;
	int windowWidth = 1600;
	int windowHeight = 900;

	Engine();
	Engine(const Engine& inOther) {}
	void operator=(const Engine& inOther) {}
	virtual ~Engine();
};
