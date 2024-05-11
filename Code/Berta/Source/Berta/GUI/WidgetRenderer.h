/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WIDGET_RENDERER_HEADER
#define BT_WIDGET_RENDERER_HEADER

#include "Berta/Paint/Graphics.h"
#include "Berta/GUI/CommonEvents.h"

namespace Berta
{
	class Graphics;
	class WidgetBase;

	class WidgetRenderer
	{
	public:
		WidgetRenderer() = default;
		virtual ~WidgetRenderer() = default;

		virtual void Init(WidgetBase& widget);
		virtual void Update(Graphics& graphics);
		virtual void MouseMove(Graphics& graphics, const ArgMouseMove& args);

	private:
		
	};
}

#endif