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
#include "Berta/GUI/ControlWindow.h"
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
		~Window();

		WindowType Type;
		API::NativeWindowHandle RootHandle{};

		std::wstring Title;
		bool Visible{ false };
		Size Size;
		Point Position;

		uint32_t DPI{ 0 };
		float DPIScaleFactor{ 1.0f };

		Renderer Renderer;
		Graphics* RootGraphics{ nullptr };
		std::shared_ptr<ControlAppearance> Appereance{ nullptr };
		std::shared_ptr<CommonEvents> Events{ nullptr };
		std::unique_ptr<ControlWindowInterface> ControlWindowPtr{ nullptr };

		Window* Parent{ nullptr };
		Window* Owner{ nullptr };
		std::vector<Window*> Children;

		Window* RootWindow{ nullptr };
		std::vector<Window*> DeferredRequests;

		struct Flags
		{
			bool IsEnabled{ true };
			bool IsDestroying{ false };
			bool MakeActive{ true };
		}Flags;

#if BT_DEBUG
		std::string Name;
#endif
	};
}

#endif