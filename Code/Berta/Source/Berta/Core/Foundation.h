/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FOUNDATION_HEADER
#define BT_FOUNDATION_HEADER

#include "Berta/GUI/WindowManager.h"

namespace Berta
{
	class Logger;

	class Foundation
	{
	public:
		Foundation();
		~Foundation();

		Foundation(const Foundation&) = delete;
		Foundation& operator=(const Foundation&) = delete;

		WindowManager& GetWindowManager() { return m_windowManager; }
		void ProcessMessages();

		static Foundation& GetInstance();

	private:
		static Foundation g_foundation;
		std::shared_ptr<Logger> m_logger;
		WindowManager m_windowManager;
	};
}

#endif