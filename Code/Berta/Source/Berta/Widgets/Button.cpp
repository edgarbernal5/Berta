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
	void ButtonRenderer::Init(WidgetBase& widget)
	{
		m_widget = &widget;
	}

	void ButtonRenderer::Update(Graphics& graphics)
	{
		auto window = m_widget->Handle();

		if (m_status == State::Normal)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), GUI::GetBackgroundColor(window), true);
		}
		else if (m_status == State::Hovered)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), { 0xE6E1D9 }, true);
		}
		else if (m_status == State::Pressed)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), { 0xBFBCB4 }, true);
		}
		graphics.DrawRectangle(window->Size.ToRectangle(), { 0x918F89 }, false);

		auto caption = m_widget->Caption();
		auto center = window->Size - graphics.GetStringSize(caption);
		center = center * 0.5f;
		graphics.DrawString({ (int)center.Width,(int)center.Height }, caption, GUI::GetForegroundColor(window));
	}

	void ButtonRenderer::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Hovered;
		Update(graphics);
		GUI::UpdateDeferred(*m_widget);
	}

	void ButtonRenderer::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Normal;
		Update(graphics);
		GUI::UpdateDeferred(*m_widget);
	}

	Button::Button(Window* parent, const Rectangle& rectangle, std::wstring text)
	{
		Create(parent, rectangle);
		Caption(text);
	}
}