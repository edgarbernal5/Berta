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

			WindowData(Window* _window, Size size) :
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
		void UpdateTree(Window* window);
		void Show(Window* window, bool visible);

	private:
		std::map<API::NativeWindowHandle, WindowData> m_windowNativeRegistry;
		std::set<Window*> m_windowRegistry;
	};
}

#endif