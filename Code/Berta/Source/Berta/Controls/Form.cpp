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
		graphics.DrawRectangle(m_control->Handle()->Appearance->Background, true);
	}

	Form::Form(const Size& size, const FormStyle& windowStyle)
	{
		Create(nullptr, GUI::GetCenteredOnScreen(size), windowStyle);

#if BT_DEBUG
		m_handle->Name = "Form";
#endif
	}

	Form::Form(const Rectangle& rectangle, const FormStyle& windowStyle)
	{
		Create(nullptr, rectangle, windowStyle);

#if BT_DEBUG
		m_handle->Name = "Form";
#endif
	}

	void Form::Exec()
	{
		Foundation::GetInstance().ProcessMessages();
	}
}