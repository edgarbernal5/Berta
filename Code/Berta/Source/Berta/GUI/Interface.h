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
	class WidgetBase;
	class WidgetRenderer;
	struct CommonEvents;

	namespace GUI
	{
		Window* CreateForm(const Rectangle& rectangle, const FormStyle& windowStyle);
		Window* CreateWidget(Window* parent, const Rectangle& rectangle);

		void CaptionWindow(Window* window, const std::wstring& caption);
		std::wstring GetCaptionWindow(Window* window);
		void DisposeWindow(Window* window);
		void ShowWindow(Window* window, bool visible);

		void InitRenderer(WidgetBase* window, WidgetRenderer& widgetRenderer);

		void SetEvents(Window* window, std::shared_ptr<CommonEvents> events);
		void SetAppearance(Window* window, WidgetAppearance* widgetAppearance);
		Color GetBackgroundColor(Window* window);
		Color GetForegroundColor(Window* window);

		void UpdateDeferred(Window* window);
	}
}

#endif