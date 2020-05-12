#include "core/Engine.h"
#include "core/TimeManager.h"
#include "data/DataManager.h"
#include "render/Renderer.h"

Engine* Engine::staticInstance = new Engine();

Engine* Engine::GetInstance()
{
	return staticInstance;
}

ScenePtr Engine::GetSceneInstance()
{
	return staticInstance->GetScene();
}

void Engine::Run()
{
	Init();
	MainLoop();
	Cleanup();
}

Renderer* Engine::GetRendererInstance()
{
	return staticInstance->GetRenderer();
}

ScenePtr Engine::GetScene()
{
	return sceneInstance;
}

Renderer* Engine::GetRenderer()
{
	return rendererInstance;
}

GLFWwindow* Engine::GetGlfwWindow()
{
	return window;
}

void Engine::Init()
{
	// init glfw window
	InitWindow();

	// init modules
	rendererInstance = new Renderer();
	sceneInstance = ObjectBase::NewObject<Scene>();

	rendererInstance->Init();
	sceneInstance->Init();
}

void Engine::MainLoop()
{
	// in case of a long resources initialization make the first delta time really small
	TimeManager::GetInstance()->UpdateTime();

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		TimeManager::GetInstance()->UpdateTime();

		sceneInstance->PerFrameUpdate();
		rendererInstance->RenderFrame();
	}

	rendererInstance->WaitForDevice();
}

void Engine::Cleanup()
{
	DataManager::ShutdownInstance();

	rendererInstance->Cleanup();
	delete rendererInstance;

	// glfw cleanup
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Engine::InitWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(windowWidth, windowHeight, "Vulkan renderer", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, Engine::FramebufferResizeCallback);
}

void Engine::FramebufferResizeCallback(GLFWwindow* inWindow, int inWidth, int inHeight)
{
	GetRendererInstance()->SetResolution(inWidth, inHeight);
}

Engine::Engine()
{
}

Engine::~Engine()
{
}


