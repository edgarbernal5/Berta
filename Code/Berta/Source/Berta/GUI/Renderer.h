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
	class WidgetBase;
	class WidgetRenderer;

	class Renderer
	{
	public:
		void Init(WidgetBase& widget, WidgetRenderer& widgetRenderer);
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
		WidgetRenderer* m_widgetRenderer;
		Graphics m_graphics;
	};
}

#endif