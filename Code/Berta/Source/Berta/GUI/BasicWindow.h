/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BASIC_WINDOW_HEADER
#define BT_BASIC_WINDOW_HEADER

#include <string>
#include <vector>
#include "Berta/Core/BasicTypes.h"
#include "Berta/Paint/Graphics.h"
#include "Berta/API/WindowAPI.h"

namespace Berta
{
	enum class WindowType
	{
		Native = 0,
		Widget
	};

	struct BasicWindow
	{
		BasicWindow() = default;
		BasicWindow(WindowType type) : Type(type) {}

		std::wstring Title;
		bool Visible{ false };
		Size Size;

		WindowType Type;
		API::NativeWindowHandle Root{};

		Graphics Graphics;

		BasicWindow* Parent;
		std::vector<BasicWindow*> Children;
	};
}

#endif