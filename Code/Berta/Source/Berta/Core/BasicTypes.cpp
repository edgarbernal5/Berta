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
		return RECT{ static_cast<LONG>(X), static_cast<LONG>(Y), static_cast<LONG>(X + Width), static_cast<LONG>(Y + Height) };
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

	Size Size::operator-(const Size& other) const
	{
		return { Width - other.Width, Height - other.Height };
	}

	Size Size::operator*(float other) const
	{
		return { static_cast<uint32_t>(Width * other), static_cast<uint32_t>(Height * other) };
	}

	Size& Size::operator*=(uint32_t s) noexcept
	{
		Width *= s;
		Height *= s;
		return *this;
	}

	Size& Size::operator/=(uint32_t s) noexcept
	{
		Width /= s;
		Height /= s;
		return *this;
	}

	Rectangle Size::ToRectangle() const
	{
		return { 0, 0, Width, Height };
	}

	std::ostream& operator<<(std::ostream& os, const Size& size)
	{
		os << "{ Width=" << size.Width << "; Height=" << size.Height << "}";
		return os;
	}
}
