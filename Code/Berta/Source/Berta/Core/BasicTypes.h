/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BASIC_TYPES_HEADER
#define BT_BASIC_TYPES_HEADER

#include <cstdint>

namespace Berta
{
	struct Size;
	struct Rectangle;

	struct Rectangle
	{
		int X{ 0 };
		int Y{ 0 };
		uint32_t Width{ 0 };
		uint32_t Height{ 0 };

#ifdef BT_PLATFORM_WINDOWS
		void FromRECT(const RECT& rect)
		{
			X = rect.left;
			Y = rect.top;
			Width = rect.right - rect.left;
			Height = rect.bottom - rect.top;
		}
#endif
		operator Size() const;
	};

	struct Size
	{
		uint32_t Width{ 0 };
		uint32_t Height{ 0 };

		bool IsEmpty()
		{
			return Width == 0 && Height == 0;
		}

		bool operator==(const Size& rhs) const
		{
			return (Width == rhs.Width) && (Height == rhs.Height);
		}

		bool operator!=(const Size& rhs) const
		{
			return (Width != rhs.Width) || (Height != rhs.Height);
		}

		Rectangle ToRectangle()
		{
			return { 0, 0, Width, Height };
		}

		static const Size Zero;
	};

	template<typename T>
	struct BasicPoint
	{
		using ValueType = T;

		ValueType X{};
		ValueType Y{};

		BasicPoint(ValueType x, ValueType y)
			: X{ x }, Y{ y }
		{}

		bool operator==(const BasicPoint& other) const noexcept
		{
			return (X == other.X && Y == other.Y);
		}

		bool operator!=(const BasicPoint& other) const noexcept
		{
			return (X != other.X || Y != other.Y);
		}

		BasicPoint operator-(const BasicPoint& other) const noexcept
		{
			return{ X - other.X, Y - other.Y };
		}

		BasicPoint operator+(const BasicPoint& other) const noexcept
		{
			return{ X + other.X, Y + other.Y };
		}

		BasicPoint& operator-=(const BasicPoint& other) noexcept
		{
			X -= other.X;
			Y -= other.Y;
			return *this;
		}

		BasicPoint& operator+=(const BasicPoint& other) noexcept
		{
			X += other.X;
			Y += other.Y;
			return *this;
		}

		BasicPoint& operator++() noexcept
		{
			++X;
			++Y;
			return *this;
		}

		BasicPoint operator++(int) noexcept
		{
			auto ret = *this;
			++X;
			++Y;
			return ret;
		}

		BasicPoint& operator--() noexcept
		{
			--X;
			--Y;
			return *this;
		}

		BasicPoint operator--(int) noexcept
		{
			auto ret = *this;
			--X;
			--Y;
			return ret;
		}
	};

	using Point = BasicPoint<int>;

	struct WindowStyle
	{
		bool Minimize{ true };
		bool Maximize{ true };
		bool Sizable{ true };
	};

	struct Color
	{
		union
		{
			uint32_t RGB; //Windows RGB format: 0xBBGGRR
		};
	};
}

#endif