/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Form.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void FormRenderer::Init(WidgetBase& widget)
	{
		m_widget = &widget;
	}

	void FormRenderer::Update(Graphics& graphics)
	{
		graphics.DrawRectangle(GUI::GetBackgroundColor(*m_widget), true);
	}

	Form::Form(const Rectangle& rectangle, const WindowStyle& windowStyle)
	{
		Create(nullptr, rectangle, windowStyle);
	}

	void Form::Exec()
	{
		Foundation::GetInstance().ProcessMessages();
	}
}