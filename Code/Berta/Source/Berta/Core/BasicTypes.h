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

	template<typename T>
	struct BasicPoint
	{
		using ValueType = T;

		ValueType X{};
		ValueType Y{};

		BasicPoint() {}
		BasicPoint(ValueType x, ValueType y)
			: X{ x }, Y{ y }
		{
		}

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

		BasicPoint& operator*=(const ValueType& other) noexcept
		{
			X *= other;
			Y *= other;
			return *this;
		}

		BasicPoint& operator/=(const ValueType& other) noexcept
		{
			X /= other;
			Y /= other;
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

		friend std::ostream& operator<<(std::ostream& os, const BasicPoint& point)
		{
			os << "{ X=" << point.X << "; Y=" << point.Y << "}";
			return os;
		}

	};

	using Point = BasicPoint<int>;

	struct Rectangle
	{
		int X{ 0 };
		int Y{ 0 };
		uint32_t Width{ 0 };
		uint32_t Height{ 0 };

		Rectangle() = default;
		Rectangle(int x, int y, uint32_t width, uint32_t height);
		explicit Rectangle(const Size& s);
		explicit Rectangle(const Point& p, const Size& s);

#ifdef BT_PLATFORM_WINDOWS
		void FromRECT(const RECT& rect)
		{
			X = rect.left;
			Y = rect.top;
			Width = rect.right - rect.left;
			Height = rect.bottom - rect.top;
		}

		RECT ToRECT() const;
#endif
		bool IsInside(const Point& point);

		bool operator==(const Rectangle& other) const noexcept
		{
			return (X == other.X && Y == other.Y && Width == other.Width && Height == other.Height);
		}

		bool operator!=(const Rectangle& other) const noexcept
		{
			return (X != other.X || Y != other.Y || Width != other.Width || Height != other.Height);
		}

		operator Size() const;
		operator Point() const;
	};

	struct Size
	{
		uint32_t Width{ 0 };
		uint32_t Height{ 0 };

		Size() = default;
		Size(uint32_t width, uint32_t height) : 
			Width(width), Height(height)
		{
		}

		bool IsEmpty() const
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

		Size operator-(const Size& size) const;
		Size operator*(float scalar) const;
		Size& operator*= (uint32_t scalar) noexcept;
		Size& operator*= (float scalar) noexcept;
		Size& operator/= (uint32_t scalar) noexcept;

		Rectangle ToRectangle() const;
		bool IsInside(const Point& point) const;

		friend std::ostream& operator<<(std::ostream& os, const Size& size);

		static const Size Zero;
	};

	struct FormStyle
	{
		bool Minimize{ true };
		bool Maximize{ true };
		bool Sizable{ true };
		bool AppWindow{ true };
		bool Floating{ false };
		bool TitleBarAndCaption{ true };
	};

	//struct Color
	//{
		union Color
		{
			//struct
			//{
			//	unsigned char B;
			//	unsigned char G;
			//	unsigned char R;
			//	//unsigned char A;
			//}Channels;
			uint32_t BGR; //Format: 0xBBGGRR
		};
	//};

	enum class Cursor
	{
		Default,
		IBeam,
		Wait
	};

	using ScrollBarUnit = int;
}

#endif