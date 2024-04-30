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
	Form::Form(const Rectangle& rectangle)
	{
		Create(rectangle);
	}

	void Form::Exec()
	{
		Foundation::GetInstance().ProcessMessages();
	}

	void Form::Create(const Rectangle& rectangle)
	{
		m_handle = GUI::CreateBasicWindow(rectangle);
	}
}