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
	struct CommonEvents;
	struct ControlAppearance;

	enum class WindowType
	{
		Native = 0,
		Control
	};

	struct Window
	{
		Window() = default;
		Window(WindowType type) : Type(type) {}

		std::wstring Title;
		bool Visible{ false };
		Size Size;
		Point Position;

		uint32_t DPI{ 0 }; //TODO:

		WindowType Type;
		API::NativeWindowHandle RootHandle{};

		Renderer Renderer;
		Graphics* RootGraphics{ nullptr };
		ControlAppearance* Appereance{ nullptr };
		std::shared_ptr<CommonEvents> Events{ nullptr };

		Window* Parent{ nullptr };
		std::vector<Window*> Children;

		Window* RootWindow{ nullptr };
		std::vector<Window*> DeferredRequests;
	};
}

#endif