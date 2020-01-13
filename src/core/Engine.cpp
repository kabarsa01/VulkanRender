#include "core/Engine.h"
#include "core/TimeManager.h"

Engine* Engine::StaticInstance = new Engine();

Engine * Engine::GetInstance()
{
	return StaticInstance;
}

ScenePtr Engine::GetSceneInstance()
{
	return StaticInstance->GetScene();
}

void Engine::Run()
{
	Init();
	MainLoop();
	Cleanup();
}

RendererPtr Engine::GetRendererInstance()
{
	return StaticInstance->GetRenderer();
}

ScenePtr Engine::GetScene()
{
	return SceneInstance;
}

RendererPtr Engine::GetRenderer()
{
	return RendererInstance;
}

void Engine::Init()
{
	// init glfw window
	InitWindow();

	// init modules
	SceneInstance = ObjectBase::NewObject<Scene>();
	RendererInstance = ObjectBase::NewObject<Renderer>();

	RendererInstance->Init();
}

void Engine::MainLoop()
{
	while (!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();

		TimeManager::GetInstance()->UpdateTime();

		SceneInstance->PerFrameUpdate();
		//	RendererInstance->RenderFrame();
	}
}

void Engine::Cleanup()
{
	RendererInstance->Cleanup();
	// glfw cleanup
	glfwDestroyWindow(Window);
	glfwTerminate();
}

void Engine::InitWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	Window = glfwCreateWindow(WindowWidth, WindowHeight, "Vulkan renderer", nullptr, nullptr);
}

Engine::Engine()
{
}

Engine::~Engine()
{
}


