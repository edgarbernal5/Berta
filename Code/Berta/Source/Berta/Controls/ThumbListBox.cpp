/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ThumbListBox.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void ThumbListBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
	}

	void ThumbListBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBackground, true);
	}

	ThumbListBox::ThumbListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}

	void ThumbListBox::Clear()
	{
	}
	void ThumbListBox::PushItem(const std::wstring& text)
	{
	}
}
