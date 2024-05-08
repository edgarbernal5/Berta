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
	void LabelRenderer::Init(WidgetBase& widget)
	{
		m_widget = &widget;
	}

	void LabelRenderer::Update(Graphics& graphics)
	{
		graphics.DrawRectangle(GUI::GetBackgroundColor(*m_widget), true);
		graphics.DrawString({ 0,0 }, m_widget->Caption(), GUI::GetForegroundColor(*m_widget));
	}

	Label::Label(Window* parent, const Rectangle& rectangle, std::wstring text)
	{
		Create(parent, rectangle);
		Caption(text);
	}
}