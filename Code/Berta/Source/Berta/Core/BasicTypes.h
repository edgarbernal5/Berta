/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BASIC_TYPES_HEADER
#define BT_BASIC_TYPES_HEADER

#include <cstdint>

#ifdef BT_PLATFORM_WINDOWS
#include <d2d1.h>
#endif

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

		BasicPoint& operator>>= (const ValueType & other) noexcept
		{
			X >>= other;
			Y >>= other;
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

		BasicPoint operator-() const
		{
			auto ret = *this;
			ret.X = -ret.X;
			ret.Y = -ret.Y;
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
		uint32_t Width{ 0u };
		uint32_t Height{ 0u };

		Rectangle() = default;
		Rectangle(int x, int y, uint32_t width, uint32_t height);
		explicit Rectangle(const Size& s);
		explicit Rectangle(const Point& p, const Size& s);

#ifdef BT_PLATFORM_WINDOWS
		void FromRECT(const ::RECT& rect)
		{
			X = rect.left;
			Y = rect.top;
			Width = rect.right - rect.left;
			Height = rect.bottom - rect.top;
		}

		::RECT ToRECT() const;
#endif
		bool IsInside(const Point& point) const;
		bool IsEmpty() const
		{
			return Width == 0 && Height == 0;
		}
		bool Intersect(const Rectangle& other) const;
		bool Contains(const Rectangle& other) const;

		bool operator==(const Rectangle& other) const noexcept
		{
			return (X == other.X && Y == other.Y && Width == other.Width && Height == other.Height);
		}

		bool operator!=(const Rectangle& other) const noexcept
		{
			return (X != other.X || Y != other.Y || Width != other.Width || Height != other.Height);
		}

		friend std::ostream& operator<<(std::ostream& os, const Rectangle& rect)
		{
			os << "{ X=" << rect.X << "; Y=" << rect.Y << "; Width=" << rect.Width << "; Height=" << rect.Height << " }";
			return os;
		}

		operator Size() const;
		operator Point() const;
#ifdef BT_PLATFORM_WINDOWS
		operator D2D1_RECT_F() const;
#endif

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
		
		operator Point() const;

		friend std::ostream& operator<<(std::ostream& os, const Size& size);

		static const Size Zero;
	};

	struct FormStyle
	{
		static FormStyle Float();
		static FormStyle Flat();

		bool Minimize{ true };
		bool Maximize{ true };
		bool Sizable{ true };
		bool AppWindow{ true };
		bool Floating{ false };
		bool TitleBarAndCaption{ true };
	};

	union ColorABGR
	{
		struct
		{
			unsigned char B;
			unsigned char G;
			unsigned char R;
			unsigned char A;
		}Channels;
		uint32_t BGRA; //Format: 0xBBGGRR
	};
	
	struct Color
	{
		Color(uint32_t colorBGR);

		operator uint32_t() const;
#ifdef BT_PLATFORM_WINDOWS
		operator D2D1_COLOR_F() const;
#endif

	private:
		unsigned char R{ 255 };
		unsigned char G{ 255 };
		unsigned char B{ 255 };
		unsigned char A{ 255 };

		//ColorABGR Data;
	};

	enum class Cursor
	{
		Default,
		IBeam,
		Wait,
		SizeWE,
		SizeNS
	};

	using ScrollBarUnit = int;
}

#endif