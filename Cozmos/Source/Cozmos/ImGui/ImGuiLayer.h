#pragma once

#include "Cozmos/Core/Layer.h"

#include "Cozmos/Events/ApplicationEvent.h"
#include "Cozmos/Events/KeyEvent.h"
#include "Cozmos/Events/MouseEvent.h"

namespace Cozmos
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;
	};
}