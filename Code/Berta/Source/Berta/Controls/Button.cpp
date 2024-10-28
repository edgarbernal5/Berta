/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Button.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void ButtonReactor::Init(ControlBase& control)
	{
		m_control = &control;
	}

	void ButtonReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->Background, true);
		//if (!enabled)
		//{
		//	graphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->ButtonDisabledBackground, true);
		//}
		//else if (m_status == State::Normal)
		//{
		//	graphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->ButtonBackground, true);
		//}
		//else if (m_status == State::Hovered)
		//{
		//	graphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->ButtonHighlightBackground, true);
		//}
		//else if (m_status == State::Pressed)
		//{
		//	graphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->ButtonPressedBackground, true);
		//}
		//graphics.DrawRoundRectBox(window->Size.ToRectangle(), enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);
		//graphics.DrawGradientFill(window->Size.ToRectangle(), window->Appearance->BoxBorderColor, window->Appearance->BoxBorderDisabledColor);
		
		graphics.DrawButton(window->Size.ToRectangle(), window->Appearance->ButtonTest1, window->Appearance->ButtonTest2, window->Appearance->BoxBorderColor);
		//graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);

		auto caption = m_control->GetCaption();
		auto center = window->Size - graphics.GetTextExtent(caption);
		center = center * 0.5f;
		graphics.DrawString({ (int)center.Width,(int)center.Height }, caption, enabled ? window->Appearance->Foreground : window->Appearance->BoxBorderDisabledColor);
	}

	void ButtonReactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Hovered;

		Update(graphics);
		GUI::MarkAsUpdated(*m_control);
	}

	void ButtonReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Normal;

		Update(graphics);
		GUI::MarkAsUpdated(*m_control);
	}

	void ButtonReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Pressed;

		Update(graphics);
		GUI::MarkAsUpdated(*m_control);

		GUI::Capture(*m_control);
	}

	void ButtonReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (m_control->Handle()->Size.IsInside(args.Position))
		{
			m_status = State::Hovered;
		}
		else
		{
			m_status = State::Normal;
		}

		GUI::ReleaseCapture(*m_control);

		Update(graphics);
		GUI::MarkAsUpdated(*m_control);
	}

	Button::Button(Window* parent, const Rectangle& rectangle, const std::wstring& text)
	{
		Create(parent, true, rectangle);
		SetCaption(text);

#if BT_DEBUG
		m_handle->Name = "Button";
#endif
	}
}