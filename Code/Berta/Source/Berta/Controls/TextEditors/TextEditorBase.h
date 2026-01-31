/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TEXT_EDITOR_BASE_HEADER
#define BT_TEXT_EDITOR_BASE_HEADER

#if BT_PLATFORM_WINDOWS
#include <wrl/client.h>
#include "Berta/Platform/Windows/D2D.h"
#endif

namespace Berta
{
	enum class TextFocusBehavior : uint8_t
	{
		None,
		Select,
		SelectOnClick
	};
	
	struct TextPosition
	{
		size_t line = 0;
		size_t column = 0;

		bool operator==(const TextPosition& other) const
		{
			return line == other.line && column == other.column;
		}

		bool operator!=(const TextPosition& other) const
		{
			return !(*this == other);
		}

		bool operator<(const TextPosition& other) const
		{
			if (line != other.line)
				return line < other.line;
			
			return column < other.column;
		}

		bool operator>(const TextPosition& other) const
		{
			return other < *this;
		}

		bool operator<=(const TextPosition& other) const
		{
			return !(*this > other);
		}

		bool operator>=(const TextPosition& other) const
		{
			return !(*this < other);
		}
	};
	
	struct VisualLine
	{
		size_t logicalLineIndex;
		size_t charStart;
		size_t charLength;
		uint32_t y;
		
#if BT_PLATFORM_WINDOWS
		mutable Microsoft::WRL::ComPtr<IDWriteTextLayout> layout = nullptr;
#endif
		
		VisualLine(size_t li, size_t cs, size_t cl, uint32_t py) : 
			logicalLineIndex(li), charStart(cs), charLength(cl), y(py)
		{
		}
	};
}

#endif