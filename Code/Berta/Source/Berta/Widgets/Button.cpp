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
		graphics.DrawRectangle(window->Size.ToRectangle(), GUI::GetBackgroundColor(window), true);
		
		graphics.DrawRectangle(window->Size.ToRectangle(), { 0x0000FF }, false);

		auto caption = m_widget->Caption();
		auto center = window->Size - graphics.GetStringSize(caption);
		center = center * 0.5f;
		graphics.DrawString({ (int)center.Width,(int)center.Height }, caption, GUI::GetForegroundColor(window));
	}

	Button::Button(Window* parent, const Rectangle& rectangle, std::wstring text)
	{
		Create(parent, rectangle);
		Caption(text);
	}
}