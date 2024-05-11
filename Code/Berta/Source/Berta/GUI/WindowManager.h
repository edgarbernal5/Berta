/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WINDOW_MANAGER_HEADER
#define BT_WINDOW_MANAGER_HEADER

#include <map>
#include <set>
#include <string>
#include "Berta/API/WindowAPI.h"
#include "Berta/Paint/Graphics.h"

namespace Berta
{
	struct Window;

	class WindowManager
	{
	public:

		struct WindowData
		{
			Window* WindowPtr;
			Graphics RootGraphics;

			Window* Pressed{ nullptr };
			Window* Hovered{ nullptr };

			WindowData(Window* _window, const Size& size) :
				WindowPtr(_window),
				RootGraphics(size)
			{}
		};

		void Add(Window* window);
		void AddNative(API::NativeWindowHandle nativeWindowHandle, const WindowData& append);
		void Caption(Window* window, const std::wstring& caption);
		void Dispose(Window* window);
		Window* Get(API::NativeWindowHandle nativeWindowHandle);
		WindowData* GetWindowData(API::NativeWindowHandle nativeWindowHandle);
		bool Exists(Window* window);

		Window* Find(Window* window, const Point& point);
		void UpdateTree(Window* window);
		void Show(Window* window, bool visible);

	private:
		bool IsPointOnWindow(Window* window, const Point& point);
		Window* FindInTree(Window* window, const Point& point);
		std::map<API::NativeWindowHandle, WindowData> m_windowNativeRegistry;
		std::set<Window*> m_windowRegistry;
	};
}

#endif