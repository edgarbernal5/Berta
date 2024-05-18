/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "InputText.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void InputTextRenderer::Init(ControlBase& control)
	{
		m_control = &control;
	}

	void InputTextRenderer::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->Size.ToRectangle(), GUI::GetBoxBackgroundColor(window), true);
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBorderColor, false);
		graphics.DrawString({ 0,0 }, m_control->Caption(), GUI::GetForegroundColor(window));
	}

	InputText::InputText(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle);
	}
}