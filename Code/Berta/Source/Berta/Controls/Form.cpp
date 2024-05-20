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
	void FormReactor::Init(ControlBase& control)
	{
		m_control = &control;
	}

	void FormReactor::Update(Graphics& graphics)
	{
		graphics.DrawRectangle(GUI::GetBackgroundColor(*m_control), true);
	}

	Form::Form(const Rectangle& rectangle, const FormStyle& windowStyle)
	{
		Create(nullptr, rectangle, windowStyle);
	}

	void Form::Exec()
	{
		Foundation::GetInstance().ProcessMessages();
	}
}