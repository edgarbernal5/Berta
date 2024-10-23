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
		auto enabled = m_control->GetEnabled();
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBackground, true);



		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor, false);
	}

	ListBox::ListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "ListBox";
#endif
	}
}
