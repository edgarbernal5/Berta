/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_CONTROL_RENDERER_HEADER
#define BT_CONTROL_RENDERER_HEADER

#include "Berta/Paint/Graphics.h"
#include "Berta/GUI/CommonEvents.h"

namespace Berta
{
	class Graphics;
	class ControlBase;

	class ControlRenderer
	{
	public:
		ControlRenderer() = default;
		virtual ~ControlRenderer() = default;

		virtual void Init(ControlBase& control);
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