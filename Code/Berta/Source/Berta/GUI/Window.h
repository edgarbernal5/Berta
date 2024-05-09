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
	struct WidgetAppearance;

	enum class WindowType
	{
		Native = 0,
		Widget
	};

	struct Window
	{
		Window() = default;
		Window(WindowType type) : Type(type) {}

		std::wstring Title;
		bool Visible{ false };
		Size Size;
		Point Position;

		uint32_t DPI{ 0 };

		WindowType Type;
		API::NativeWindowHandle Root{};

		Renderer Renderer;
		Graphics* RootGraphics{ nullptr };
		WidgetAppearance* Appereance{ nullptr };

		Window* Parent{ nullptr };
		std::vector<Window*> Children;
	};
}

#endif