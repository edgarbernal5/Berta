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

	class ControlReactor
	{
	public:
		ControlReactor() = default;
		virtual ~ControlReactor() = default;

		virtual void Init(ControlBase& control);
		virtual void Update(Graphics& graphics);
		virtual void MouseEnter(Graphics& graphics, const ArgMouse& args);
		virtual void MouseLeave(Graphics& graphics, const ArgMouse& args);
		virtual void MouseDown(Graphics& graphics, const ArgMouse& args);
		virtual void MouseMove(Graphics& graphics, const ArgMouse& args);
		virtual void MouseUp(Graphics& graphics, const ArgMouse& args);
		virtual void Click(Graphics& graphics, const ArgClick& args);
		virtual void Focus(Graphics& graphics, const ArgFocus& args);
		virtual void KeyPressed(Graphics& graphics, const ArgKeyboard& args);
		virtual void KeyReleased(Graphics& graphics, const ArgKeyboard& args);

	private:
		
	};
}

#endif