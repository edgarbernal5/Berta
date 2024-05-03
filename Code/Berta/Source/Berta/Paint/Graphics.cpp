/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Graphics.h"

namespace Berta
{
	Graphics::Graphics()
	{
	}

	void Graphics::DrawRectangle(const Rectangle& rectangle, bool solid)
	{
		auto brush = ::CreateSolidBrush(13160660);
		RECT nativeRect{ rectangle.X, rectangle.Y, rectangle.X + rectangle.Width,rectangle.Y + rectangle.Height };
		FillRect(m_hdc, &nativeRect, brush);

		::DeleteObject(brush);
	}
}