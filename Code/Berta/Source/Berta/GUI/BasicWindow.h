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
#include "Berta/GUI/Renderer.h"
#include "Berta/API/WindowAPI.h"

namespace Berta
{
	class Graphics;

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

		Renderer Renderer;
		Graphics* RootGraphics;

		BasicWindow* Parent;
		std::vector<BasicWindow*> Children;
	};
}

#endif