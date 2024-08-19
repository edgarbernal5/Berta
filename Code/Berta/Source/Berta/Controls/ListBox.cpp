/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ListBox.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void ListBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
	}

	void ListBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->Background, true);
	}

	ListBox::ListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}
}
