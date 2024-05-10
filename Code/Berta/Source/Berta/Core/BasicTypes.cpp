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

#ifdef BT_PLATFORM_WINDOWS
	RECT Rectangle::ToRECT() const
	{
		return RECT{ static_cast<LONG>(X), static_cast<LONG>(Y), static_cast<LONG>(X + Width), static_cast<LONG>(Y + Height)};
	}
#endif

	bool Rectangle::IsInside(const Point& point)
	{
		return (X <= point.X && point.X <= X + static_cast<int>(Width) && Y <= point.Y && point.Y <= Y + static_cast<int>(Height));
	}

	Rectangle::operator Size() const
	{
		return Size{ Width, Height };
	}

	Rectangle::operator Point() const
	{
		return { X, Y };
	}

	Rectangle Size::ToRectangle()
	{
		return { 0, 0, Width, Height };
	}
}
