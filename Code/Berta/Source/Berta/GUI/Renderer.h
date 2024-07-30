/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_RENDERER_HEADER
#define BT_RENDERER_HEADER

#include "Berta/Paint/Graphics.h"
#include "Berta/GUI/CommonEvents.h"

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
		void Update();

		void MouseEnter(const ArgMouse& args);
		void MouseLeave(const ArgMouse& args);
		void MouseDown(const ArgMouse& args);
		void MouseMove(const ArgMouse& args);
		void MouseUp(const ArgMouse& args);
		void MouseWheel(const ArgWheel& args);
		void Click(const ArgClick& args);
		void DblClick(const ArgClick& args);
		void Focus(const ArgFocus& args);
		void KeyChar(const ArgKeyboard& args);
		void KeyPressed(const ArgKeyboard& args);
		void KeyReleased(const ArgKeyboard& args);
		void Resize(const ArgResize& args);

		Graphics& GetGraphics() { return m_graphics; }
	private:
		bool m_updating{ false };
		ControlReactor* m_controlReactor{ nullptr };
		Graphics m_graphics;
	};
}

#endif