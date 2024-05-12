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
		graphics.DrawRectangle(window->Size.ToRectangle(), GUI::GetBackgroundColor(*m_widget), true);
		
		graphics.DrawLine({ 0,0 }, { (int)window->Size.Width - 1,0 }, { 0x0000FF });
		graphics.DrawLine({ 0,0 }, { 0, (int)window->Size.Height - 1 }, { 0x0000FF });
		graphics.DrawLine({ 0,(int)(window->Size.Height - 1) }, { (int)(window->Size.Width - 1), (int)(window->Size.Height - 1) }, { 0x0000FF });
		graphics.DrawLine({ (int)(window->Size.Width - 1),0 }, { (int)(window->Size.Width - 1), (int)(window->Size.Height) }, { 0x0000FF });
		
		graphics.DrawString({ 0,0 }, m_widget->Caption(), GUI::GetForegroundColor(*m_widget));
	}

	Button::Button(Window* parent, const Rectangle& rectangle, std::wstring text)
	{
		Create(parent, rectangle);
		Caption(text);
	}
}