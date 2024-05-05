/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WIDGET_RENDERER_HEADER
#define BT_WIDGET_RENDERER_HEADER

#include "Berta/Paint/Graphics.h"

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

	private:
		
	};
}

#endif