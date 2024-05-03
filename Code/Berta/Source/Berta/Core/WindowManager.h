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

namespace Berta
{
	struct BasicWindow;

	class WindowManager
	{
	public:
		void Add(BasicWindow* basicWindow);
		void AddNative(API::NativeWindowHandle nativeWindowHandle, BasicWindow* basicWindow);
		void Caption(BasicWindow* basicWindow, const std::wstring& caption);
		void Destroy(BasicWindow* basicWindow);
		BasicWindow* Get(API::NativeWindowHandle nativeWindowHandle);
		bool Exists(BasicWindow* basicWindow);
		void UpdateTree(BasicWindow* basicWindow);
		void Show(BasicWindow* basicWindow, bool visible);

	private:
		std::map<API::NativeWindowHandle, BasicWindow*> m_windowNativeRegistry;
		std::set<BasicWindow*> m_windowRegistry;
	};
}

#endif