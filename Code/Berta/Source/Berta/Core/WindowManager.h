/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WINDOW_MANAGER_HEADER
#define BT_WINDOW_MANAGER_HEADER

#include <map>
#include "Berta/API/WindowAPI.h"

namespace Berta
{
	struct BasicWindow;

	class WindowManager
	{
	public:
		void Add(BasicWindow* basicWindow);
		void Show(BasicWindow* basicWindow, bool visible);
	private:

		std::map<BasicWindow*, API::NativeWindowHandle> m_windowRegistry;
	};
}

#endif