#pragma once

#include "Core.h"

#include "Window.h"
#include "LayerStack.h"
#include "Cozmos/Events/Event.h"
#include "Cozmos/Events/ApplicationEvent.h"

#include "Cozmos/Core/Timestep.h"

#include "Cozmos/ImGui/ImGuiLayer.h"

namespace Cozmos

{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;

		bool m_IsRunning = true;
		bool m_IsMinimized = false;

		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
	};

	// To be defined in client
	Application* CreateApplication();

}

