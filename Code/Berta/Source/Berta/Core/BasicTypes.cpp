/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "BasicTypes.h"

namespace Berta
{
	const Size Size::Zero = { 0,0 };

	Rectangle::operator Size() const
	{
		return Size{ Width, Height };
	}

	Rectangle Size::ToRectangle()
	{
		return { 0, 0, Width, Height };
	}
}
