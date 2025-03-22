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
		auto backgroundRect = window->Size.ToRectangle();
		graphics.DrawRectangle(backgroundRect, enabled ? window->Appearance->Background : window->Appearance->ButtonDisabledBackground, true);

		auto color = window->Appearance->Background;
		if (!enabled)
		{
			color = window->Appearance->ButtonDisabledBackground;
		}
		else if (m_status == State::Normal)
		{
			color = window->Appearance->ButtonBackground;
		}
		else if (m_status == State::Hovered)
		{
			color = window->Appearance->ButtonHighlightBackground;
		}
		else if (m_status == State::Pressed)
		{
			color = window->Appearance->ButtonPressedBackground;
		}
		graphics.DrawRoundRectBox(backgroundRect, color, enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, true);

		auto caption = m_control->GetCaption();
		Point textExtent = graphics.GetTextExtent(caption);
		Point windowSize = window->Size;
		auto center = windowSize - textExtent;
		center /= 2;
		graphics.DrawString({ center.X, center.Y }, caption, enabled ? window->Appearance->Foreground : window->Appearance->BoxBorderDisabledColor);
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