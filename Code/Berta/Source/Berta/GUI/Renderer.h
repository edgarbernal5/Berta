/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_RENDERER_HEADER
#define BT_RENDERER_HEADER

#include "Berta/Paint/Graphics.h"

namespace Berta
{
	struct BasicWindow;
	class WidgetBase;
	class WidgetRenderer;

	class Renderer
	{
	public:
		void Init(WidgetBase& widget, WidgetRenderer& widgetRenderer);
		void Map(BasicWindow* window, const Rectangle& areaToUpdate);
		void Update();

		Graphics& GetGraphics() { return m_graphics; }
	private:
		bool m_updating{ false };
		WidgetRenderer* m_widgetRenderer;
		Graphics m_graphics;
	};
}

#endif