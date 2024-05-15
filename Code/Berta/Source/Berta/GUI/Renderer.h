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
	class ControlRenderer;

	class Renderer
	{
	public:
		void Init(ControlBase& control, ControlRenderer& controlRenderer);
		void Map(Window* window, const Rectangle& areaToUpdate);
		void Update();

		void MouseEnter(const ArgMouse& args);
		void MouseLeave(const ArgMouse& args);
		void MouseDown(const ArgMouse& args);
		void MouseMove(const ArgMouse& args);
		void MouseUp(const ArgMouse& args);
		void Click(const ArgClick& args);

		Graphics& GetGraphics() { return m_graphics; }
	private:
		bool m_updating{ false };
		ControlRenderer* m_controlRenderer;
		Graphics m_graphics;
	};
}

#endif