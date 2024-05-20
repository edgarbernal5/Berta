/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_INTERFACE_HEADER
#define BT_INTERFACE_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/Window.h"

namespace Berta
{
	class ControlBase;
	class ControlReactor;
	struct CommonEvents;

	namespace GUI
	{
		Window* CreateForm(const Rectangle& rectangle, const FormStyle& windowStyle);
		Window* CreateControl(Window* parent, const Rectangle& rectangle);

		void CaptionWindow(Window* window, const std::wstring& caption);
		std::wstring GetCaptionWindow(Window* window);
		void DisposeWindow(Window* window);
		void ShowWindow(Window* window, bool visible);
		void RefreshWindow(Window* window);

		void InitRendererReactor(ControlBase* window, ControlReactor& controlReactor);

		void SetEvents(Window* window, std::shared_ptr<CommonEvents> events);
		void SetAppearance(Window* window, ControlAppearance* controlAppearance);
		Color GetBackgroundColor(Window* window);
		Color GetBoxBackgroundColor(Window* window);
		Color GetForegroundColor(Window* window);

		void UpdateDeferred(Window* window);
		//void CreateCaret(Window* window);
	}
}

#endif