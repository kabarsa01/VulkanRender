#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/ObjectBase.h"
#include "scene/Scene.h"

namespace CGE
{
	class Scene;
	class Renderer;
	class ShaderRegistry;
	
	class Engine
	{
	public:
		static Engine* Get();
		static Engine* GetInstance();
		static Scene* GetSceneInstance();
		static Renderer* GetRendererInstance();
		static uint32_t GetFrameIndex(uint32_t div);
	
		void Run();
	
		uint64_t GetFrameCount();
		Scene* GetScene();
		Renderer* GetRenderer();
		ShaderRegistry* GetShaderRegistry();
		GLFWwindow* GetGlfwWindow();
	protected:
		Scene* m_sceneInstance;
		Renderer* m_rendererInstance;
		ShaderRegistry* m_shaderRegistry;
	
		void Init();
		void MainLoop();
		void Cleanup();
	
		void InitWindow();
		static void FramebufferResizeCallback(GLFWwindow* inWindow, int inWidth, int inHeight);
	private:
		static Engine* m_staticInstance;

		uint64_t m_frameCount = 0;
	
		GLFWwindow* m_window;
		int m_windowWidth = 1920;
		int m_windowHeight = 1080;
	
		Engine();
		Engine(const Engine& inOther) = delete;
		void operator=(const Engine& inOther) = delete;
		~Engine();
	};

}
