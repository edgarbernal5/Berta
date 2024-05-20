/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Label.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void LabelReactor::Init(ControlBase& control)
	{
		m_control = &control;
	}

	void LabelReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->Size.ToRectangle(), GUI::GetBackgroundColor(window), true);
		graphics.DrawString({ 0,0 }, m_control->Caption(), GUI::GetForegroundColor(window));
	}

	Label::Label(Window* parent, const Rectangle& rectangle, std::wstring text)
	{
		Create(parent, rectangle);
		Caption(text);
	}
}