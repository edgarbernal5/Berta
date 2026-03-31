/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_RENDERER_HEADER
#define BT_RENDERER_HEADER

#include "Berta/Paint/Graphics.h"
#include "Berta/GUI/ControlEvents.h"

namespace Berta
{
	struct Window;
	class ControlBase;
	class ControlReactor;

	class Renderer
	{
	public:
		void Init(ControlBase& control, ControlReactor& controlReactor);
		void Shutdown();
		void Map(Window* window, const Rectangle& areaToUpdate);
		void Update(const Rectangle& clipRect);
		
		void MouseEnter(const ArgMouse& args);
		void MouseLeave(const ArgMouse& args);
		void MouseDown(const ArgMouse& args);
		void MouseMove(const ArgMouse& args);
		void MouseUp(const ArgMouse& args);
		void MouseWheel(const ArgWheel& args);
		void Click(const ArgClick& args);
		void DblClick(const ArgMouse& args);
		void Focus(const ArgFocus& args);
		void KeyChar(const ArgKeyboard& args);
		void KeyPressed(const ArgKeyboard& args);
		void KeyReleased(const ArgKeyboard& args);
		void Resize(const ArgResize& args);
		void Move(const ArgMove& args);
		void DpiChanged();

		Graphics& GetGraphics() const { return *m_graphics; }
		void SetGraphics(Graphics* newGraphics);

	private:
		template <typename TArgument>
		void ProcessEvent(void(ControlReactor::* reactorEventPtr)(Graphics&, const TArgument&), const TArgument& args);
		void ProcessEvent(void(ControlReactor::* reactorEventPtr)(Graphics&));

		bool m_updating{ false };
		ControlBase* m_control{ nullptr };
		ControlReactor* m_controlReactor{ nullptr };
		Graphics* m_graphics{ nullptr };
	};

	template<typename TArgument>
	inline void Renderer::ProcessEvent(void(ControlReactor::* reactorEventPtr)(Graphics&, const TArgument&), const TArgument& args)
	{
		if (m_controlReactor == nullptr) //Added this check due to panels that don't have either reactor or graphics.
		{
			return;
		}

		(m_controlReactor->*reactorEventPtr)(*m_graphics, args);
	}
}

#endif