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
	Form::Form(const Rectangle& rectangle, const WindowStyle& windowStyle)
	{
		Create(rectangle, windowStyle);
	}

	void Form::Exec()
	{
		Foundation::GetInstance().ProcessMessages();
	}
}