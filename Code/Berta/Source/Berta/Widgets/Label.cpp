/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Label.h"

namespace Berta
{
	Label::Label(BasicWindow* parent, const Rectangle& rectangle, std::wstring text)
	{
		Create(rectangle);
		Caption(text);
	}
}