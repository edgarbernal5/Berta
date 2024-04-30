/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WINDOW_MANAGER_HEADER
#define BT_WINDOW_MANAGER_HEADER

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
		void Caption(BasicWindow* basicWindow, const std::wstring& caption);
		
		bool Exists(BasicWindow* basicWindow);
		void Show(BasicWindow* basicWindow, bool visible);

	private:

		std::set<BasicWindow*> m_windowRegistry;
	};
}

#endif