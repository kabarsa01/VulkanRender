#include "core/Engine.h"
#include "core/TimeManager.h"
#include "data/DataManager.h"
#include "render/Renderer.h"
#include "async/ThreadPool.h"
#include "messages/MessageBus.h"
#include "render/ShaderRegistry.h"

namespace CGE
{
	static const constexpr uint32_t THREAD_COUNT = 16;
	static const constexpr uint32_t MESSAGE_THREAD_COUNT = 4;

	Engine* Engine::m_staticInstance = new Engine();

	Engine* Engine::Get()
	{
		return m_staticInstance;
	}

	Engine* Engine::GetInstance()
	{
		return m_staticInstance;
	}
	
	Scene* Engine::GetSceneInstance()
	{
		return m_staticInstance->GetScene();
	}
	
	void Engine::Run()
	{
		Init();
		MainLoop();
		Cleanup();
	}
	
	uint64_t Engine::GetFrameCount()
	{
		return m_frameCount;
	}

	Renderer* Engine::GetRendererInstance()
	{
		return m_staticInstance->GetRenderer();
	}
	
	Scene* Engine::GetScene()
	{
		return m_sceneInstance;
	}
	
	Renderer* Engine::GetRenderer()
	{
		return m_rendererInstance;
	}
	
	ShaderRegistry* Engine::GetShaderRegistry()
	{
		return m_shaderRegistry;
	}

	GLFWwindow* Engine::GetGlfwWindow()
	{
		return m_window;
	}
	
	void Engine::Init()
	{
		ThreadPool::InitInstance(THREAD_COUNT);
		MessageBus::InitInstance(MESSAGE_THREAD_COUNT);
		// init glfw window
		InitWindow();
	
		// init modules
		m_rendererInstance = new Renderer();
		m_shaderRegistry = new ShaderRegistry();
		m_sceneInstance = new Scene();
	
		m_rendererInstance->Init();
		m_sceneInstance->Init();
	}
	
	void Engine::MainLoop()
	{
		// in case of a long resources initialization make the first delta time really small
		TimeManager::GetInstance()->UpdateTime();
	
		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();
	
			TimeManager::GetInstance()->UpdateTime();

			MessageBus::GetInstance()->PublishSync(std::make_shared<GlobalPreFrameMessage>(TimeManager::GetInstance()->GetDeltaTime()));
			MessageBus::GetInstance()->PublishSync(std::make_shared<GlobalPreSceneMessage>(TimeManager::GetInstance()->GetDeltaTime()));
	
			auto sceneStartTime = std::chrono::high_resolution_clock::now();
			m_sceneInstance->PerFrameUpdate();
			auto sceneCurrentTime = std::chrono::high_resolution_clock::now();
			double sceneDeltaTime = std::chrono::duration<double, std::chrono::microseconds::period>(sceneCurrentTime - sceneStartTime).count();
			std::printf("scene update time is %f microseconds\n", sceneDeltaTime);
			auto renderStartTime = std::chrono::high_resolution_clock::now();

			MessageBus::GetInstance()->PublishSync(std::make_shared<GlobalPostSceneMessage>(TimeManager::GetInstance()->GetDeltaTime()));
			MessageBus::GetInstance()->PublishSync(std::make_shared<GlobalUpdateMessage>(TimeManager::GetInstance()->GetDeltaTime()));
			MessageBus::GetInstance()->PublishSync(std::make_shared<GlobalPreRenderMessage>(TimeManager::GetInstance()->GetDeltaTime()));

			m_rendererInstance->RenderFrame();
			auto renderCurrentTime = std::chrono::high_resolution_clock::now();
			double renderDeltaTime = std::chrono::duration<double, std::chrono::microseconds::period>(renderCurrentTime - renderStartTime).count();
			std::printf("render update time is %f microseconds\n", renderDeltaTime);

			MessageBus::GetInstance()->PublishSync(std::make_shared<GlobalPostRenderMessage>(TimeManager::GetInstance()->GetDeltaTime()));
			MessageBus::GetInstance()->PublishSync(std::make_shared<GlobalPostFrameMessage>(m_frameCount));

			// just not to forget let it increment in a separate statement
			++m_frameCount;
		}
	
		m_rendererInstance->WaitForDevice();
	}
	
	void Engine::Cleanup()
	{
		ThreadPool::DestroyInstance();
		DataManager::ShutdownInstance();
	
		m_rendererInstance->Cleanup();
		delete m_rendererInstance;
		delete m_shaderRegistry;
	
		// glfw cleanup
		glfwDestroyWindow(m_window);
		glfwTerminate();

		MessageBus::DestroyInstance();
	}
	
	void Engine::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	
		m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Vulkan renderer", nullptr, nullptr);
		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, Engine::FramebufferResizeCallback);
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
	
	
}
