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
		virtual void MouseEnter(Graphics& graphics, const ArgMouse& args);
		virtual void MouseLeave(Graphics& graphics, const ArgMouse& args);
		virtual void MouseDown(Graphics& graphics, const ArgMouse& args);
		virtual void MouseMove(Graphics& graphics, const ArgMouse& args);
		virtual void MouseUp(Graphics& graphics, const ArgMouse& args);
		virtual void Click(Graphics& graphics, const ArgClick& args);

	private:
		
	};
}

#endif