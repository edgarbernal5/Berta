/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_INTERFACE_HEADER
#define BT_INTERFACE_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/BasicWindow.h"

namespace Berta
{
	class WidgetBase;
	class WidgetRenderer;

	namespace GUI
	{
		BasicWindow* CreateNativeWindow(const Rectangle& rectangle, const WindowStyle& windowStyle);
		BasicWindow* CreateWidget(BasicWindow* parent, const Rectangle& rectangle);

		void CaptionWindow(BasicWindow* basicWindow, const std::wstring& caption);
		std::wstring GetCaptionWindow(BasicWindow* basicWindow);
		void DestroyWindow(BasicWindow* basicWindow);
		void ShowBasicWindow(BasicWindow* basicWindow, bool visible);

		void InitRenderer(WidgetBase* basicWindow, WidgetRenderer& widgetRenderer);

		void SetAppearance(BasicWindow* basicWindow, WidgetAppearance* widgetAppearance);
		Color GetBackgroundColor(BasicWindow* basicWindow);
		Color GetForegroundColor(BasicWindow* basicWindow);
	}
}

#endif