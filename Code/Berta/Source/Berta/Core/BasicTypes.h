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

		int Bottom() const noexcept;
		int Right() const noexcept;
		
		Point Position() const;
		
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
		bool Contains(const Point& point) const;
#ifdef BT_PLATFORM_WINDOWS
		bool Contains(const D2D1_POINT_2F& pointF) const;
#endif
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

		Size operator-(const Size& other) const;
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

	struct SizeF
	{
		float Width{ 0 };
		float Height{ 0 };

		SizeF() = default;
		SizeF(float width, float height) : 
			Width(width), Height(height)
		{
		}

		bool IsEmpty() const
		{
			return Width == 0.0f && Height == 0.0f ;
		}

		bool operator==(const SizeF& rhs) const
		{
			return (Width == rhs.Width) && (Height == rhs.Height);
		}

		bool operator!=(const SizeF& rhs) const
		{
			return (Width != rhs.Width) || (Height != rhs.Height);
		}

		SizeF operator-(const SizeF& other) const;
		SizeF operator*(float scalar) const;
		SizeF& operator*= (float scalar) noexcept;
		SizeF& operator/= (float scalar) noexcept;
		
		static const SizeF Zero;
	};
	
	struct FormStyle
	{
		static FormStyle Float(bool sizeable = true);
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
			uint8_t B;
			uint8_t G;
			uint8_t R;
			uint8_t A;
		}Channels;
		uint32_t BGRA; //Format: 0xBBGGRR
	};
	
	struct Padding
	{
		int Top = 0;
		int Bottom = 0;
		int Left = 0;
		int Right = 0;
	};
	
	struct Color
	{
		Color() = default;
		Color(uint32_t colorABGR);
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

		uint32_t ToBGR() const;
		operator uint32_t() const;
#ifdef BT_PLATFORM_WINDOWS
		operator D2D1_COLOR_F() const;
#endif
		uint8_t GetR() const { return R; }
		uint8_t GetG() const { return G; }
		uint8_t GetB() const { return B; }
		uint8_t GetA() const { return A; }
		
		void SetR(uint8_t newR) { R = newR; }
		void SetG(uint8_t newG) { G = newG; }
		void SetB(uint8_t newB) { B = newB; }
		void SetA(uint8_t newA) { A = newA; }
	private:
		uint8_t R{ 255 };
		uint8_t G{ 255 };
		uint8_t B{ 255 };
		uint8_t A{ 255 };

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
	
	enum class HorizontalAlign : uint8_t
	{
		Left, 
		Center, 
		Right
	};

	enum class VerticalAlign : uint8_t
	{
		Top,
		Center, 
		Bottom
	};
	
	enum class DropPosition : uint8_t
	{
		None,
		Before,
		Inside,
		After
	};
	
	enum class CheckState : uint8_t
	{ 
		None,
		Unchecked,
		Checked,
		Indeterminate
	};
	
	enum class LineStyle : uint8_t
	{
		Solid,
		Dash,
		Dotted
	};
}

#endif