/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_MENU_MANAGER_HEADER
#define BT_MENU_MANAGER_HEADER

#include "Berta/GUI/Window.h"
//#include "Berta/Controls/Menu.h"
namespace Berta
{
	struct Menu;
}
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
		size_t GetPopupCount() const;

		void ShowContextMenu(Menu& menuData, Window* owner, const Point& position);
		void ShowMenuBarPopup(Menu& menuData, Window* owner, const Point& position);
		
		void ClearListeners();
		void SubscribeOnClose(std::function<void()> listener);
		
		void NavigateTopLevel(int step);
		
	private:
		void ShowPopup(Window* window, Window* owner, bool fromMenuBar);
		
		Window* m_owner{ nullptr };
		std::vector<Window*> m_popups;
		std::vector<std::function<void()>> m_onCloseListeners;
		bool m_fromMenuBar{ false };
	};
}

#endif