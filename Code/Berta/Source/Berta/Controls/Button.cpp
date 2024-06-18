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
		
		if (!enabled)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->ButtonDisabledBackground, true);
		}
		else if (m_status == State::Normal)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->ButtonBackground, true);
		}
		else if (m_status == State::Hovered)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->ButtonHighlightBackground, true);
		}
		else if (m_status == State::Pressed)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->ButtonPressedBackground, true);
		}
		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor, false);

		auto caption = m_control->GetCaption();
		auto center = window->Size - graphics.GetTextExtent(caption);
		center = center * 0.5f;
		graphics.DrawString({ (int)center.Width,(int)center.Height }, caption, enabled ? window->Appereance->Foreground : window->Appereance->BoxBorderDisabledColor);
	}

	void ButtonReactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Hovered;
		Update(graphics);
		GUI::UpdateDeferred(*m_control);
	}

	void ButtonReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Normal;
		Update(graphics);
		GUI::UpdateDeferred(*m_control);
	}

	void ButtonReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Pressed;
		Update(graphics);
		GUI::UpdateDeferred(*m_control);

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

		Update(graphics);
		GUI::UpdateDeferred(*m_control);
		GUI::ReleaseCapture(*m_control);
	}

	Button::Button(Window* parent, const Rectangle& rectangle, std::wstring text)
	{
		Create(parent, true, rectangle);
		SetCaption(text);
	}
}