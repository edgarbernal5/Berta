/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "BasicTypes.h"

namespace Berta
{
#define BT_WINDOWS_ABGR(r, g, b, a)	((uint32_t)(a)<<24 | (uint32_t)(b)<<16 | (uint32_t)(g)<<8 | (uint32_t)(r))

	const Size Size::Zero = { 0,0 };

	Rectangle::Rectangle(int x, int y, uint32_t width, uint32_t height) :
		X(x),
		Y(y),
		Width(width),
		Height(height)
	{
	}

	Rectangle::Rectangle(const Size& s) :
		Width(s.Width),
		Height(s.Height)
	{
	}

	Rectangle::Rectangle(const Point& p, const Size& s) :
		X(p.X),
		Y(p.Y),
		Width(s.Width),
		Height(s.Height)
	{
	}

#ifdef BT_PLATFORM_WINDOWS
	::RECT Rectangle::ToRECT() const
	{
		return RECT{ static_cast<LONG>(X), static_cast<LONG>(Y), static_cast<LONG>(X + Width), static_cast<LONG>(Y + Height) };
	}
#endif

	bool Rectangle::IsInside(const Point& point) const
	{
		return (X <= point.X && point.X <= X + static_cast<int>(Width) && Y <= point.Y && point.Y <= Y + static_cast<int>(Height));
	}

#ifdef BT_PLATFORM_WINDOWS
	bool Rectangle::IsInside(const D2D1_POINT_2F& pointF) const
	{
		return (X <= pointF.x && pointF.x <= static_cast<float>(X + Width) && Y <= pointF.y && pointF.y <= static_cast<float>(Y + Height));
	}
#endif

	bool Rectangle::Intersect(const Rectangle& other) const
	{
		return !(this->X + (int)this->Width <= other.X || other.X + (int)other.Width <= this->X ||
			this->Y + (int)this->Height <= other.Y || other.Y + (int)other.Height <= this->Y);
	}

	bool Rectangle::Contains(const Rectangle& other) const
	{
		return other.X >= this->X && other.X + (int)other.Width <= (int)this->Width &&
			other.Y >= this->Y && other.Y + (int)other.Height <= (int)this->Height;
	}

	Rectangle::operator Size() const
	{
		return Size{ Width, Height };
	}

	Rectangle::operator Point() const
	{
		return { X, Y };
	}

#ifdef BT_PLATFORM_WINDOWS
	Rectangle::operator D2D1_RECT_F() const
	{
		return { static_cast<FLOAT>(X), static_cast<FLOAT>(Y), static_cast<FLOAT>(X + Width), static_cast<FLOAT>(Y + Height) };
	}
#endif

	Size Size::operator-(const Size& other) const
	{
		return { Width - other.Width, Height - other.Height };
	}

	Size Size::operator*(float scalar) const
	{
		return { static_cast<uint32_t>(Width * scalar), static_cast<uint32_t>(Height * scalar) };
	}

	Size& Size::operator*=(uint32_t scalar) noexcept
	{
		Width *= scalar;
		Height *= scalar;
		return *this;
	}

	Size& Size::operator*=(float scalar) noexcept
	{
		Width *= scalar;
		Height *= scalar;
		return *this;
	}

	Size& Size::operator/=(uint32_t scalar) noexcept
	{
		Width /= scalar;
		Height /= scalar;
		return *this;
	}

	Rectangle Size::ToRectangle() const
	{
		return { 0, 0, Width, Height };
	}

	bool Size::IsInside(const Point& point) const
	{
		return point.X >= 0 && point.X < static_cast<int>(Width) && point.Y >= 0 && point.Y < static_cast<int>(Height);
	}

	Size::operator Point() const
	{
		return { (int)Width, (int)Height };
	}

	std::ostream& operator<<(std::ostream& os, const Size& size)
	{
		os << "{ Width=" << size.Width << "; Height=" << size.Height << "}";
		return os;
	}

	Color::Color(uint32_t colorABGR)
	{
		A = (uint8_t)((colorABGR & 0xFF000000) >> 24);
		B = (uint8_t)((colorABGR & 0x00FF0000) >> 16);
		G = (uint8_t)((colorABGR & 0x0000FF00) >> 8);
		R = (uint8_t)((colorABGR & 0x000000FF));
	}

	Color::operator uint32_t() const
	{
#if BT_PLATFORM_WINDOWS
		return BT_WINDOWS_ABGR(R, G, B, A);
#endif
		return 0;
	}

	Color::operator D2D1_COLOR_F() const
	{
		D2D1_COLOR_F color;
		color.a = A / 255.0f;
		color.r = R / 255.0f;
		color.g = G / 255.0f;
		color.b = B / 255.0f;
		return color;
	}

	FormStyle FormStyle::Float()
	{
		return FormStyle{ false, false, true, false, false, false };
	}

	FormStyle FormStyle::Flat()
	{
		return FormStyle{ false, false, false, false, false, false };
	}
}
