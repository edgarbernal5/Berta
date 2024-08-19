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
		bool enabled = m_control->GetEnabled();
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBackground, true);
		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor, false);
	}

	void ThumbListBoxReactor::Module::AddItem(const std::wstring& text, const Image& thumbnail)
	{
	}

	void ThumbListBoxReactor::Module::Clear()
	{
	}

	ThumbListBox::ThumbListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}

	void ThumbListBox::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		m_reactor.GetModule().AddItem(text, thumbnail);
	}

	void ThumbListBox::Clear()
	{
		m_reactor.GetModule().Clear();
	}
}
