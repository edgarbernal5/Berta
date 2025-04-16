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
		graphics.DrawRectangle(window->ClientSize.ToRectangle(), window->Appearance->Background, true);
		graphics.DrawString({ 0,0 }, m_control->GetCaption(), window->Appearance->Foreground);
	}

	Label::Label(Window* parent, const Rectangle& rectangle, const std::wstring& text)
	{
		Create(parent, true, rectangle);
		SetCaption(text);

#if BT_DEBUG
		m_handle->Name = "Label";
#endif
	}

	Label::Label(Window* parent, const Rectangle& rectangle, const std::string& text)
	{
		Create(parent, true, rectangle);
		SetCaption(text);

#if BT_DEBUG
		m_handle->Name = "Label";
#endif
	}
}