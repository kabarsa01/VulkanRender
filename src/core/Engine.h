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
protected:
	ScenePtr SceneInstance;
	RendererPtr RendererInstance;

	void Init();
	void MainLoop();
	void Cleanup();

	void InitWindow();
private:
	static Engine* StaticInstance;

	GLFWwindow* Window;
	int WindowWidth = 1280;
	int WindowHeight = 720;

	Engine();
	virtual ~Engine();
};
