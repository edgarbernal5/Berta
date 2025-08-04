/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_MENU_MANAGER_HEADER
#define BT_MENU_MANAGER_HEADER

#include "Berta/GUI/Window.h"
#include <vector>

namespace Berta
{
	class MenuManager
	{
	public:

		bool AnyPopupActive() const;

		void Close(Window* popupWindow);
		void CloseAll();
		Window* GetActiveMenu(bool fromKeyboard = false) const;
		Window* FindMenu(const Point& mousePosition) const;

		void ShowPopup(Window* window, Window* owner, bool fromMenuBar);
	private:
		std::vector<Window*> m_popups;
		bool m_fromMenuBar{ false };
		Window* m_owner{ nullptr };
	};
}

#endif