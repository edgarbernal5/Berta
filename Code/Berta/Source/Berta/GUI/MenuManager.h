/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_MENU_MANAGER_HEADER
#define BT_MENU_MANAGER_HEADER

#include "Berta/GUI/Window.h"
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

		void Close(const Window* popupWindow);
		void CloseAll();
		void CloseChildrenOf(Window* parent);
		
		Window* GetRootPopup() const { return m_popups.empty() ? nullptr : m_popups.front(); }
		Window* GetTopPopup() const;
		Window* FindMenu(const Point& mousePosition) const;
		Window* GetOwner() const { return m_owner; }

		void ShowContextMenu(Menu& menuData, Window* owner, const Point& position);
		void ShowMenuBarPopup(Menu& menuData, Window* owner, const Point& position);
		
		void ClearListeners();
		uint32_t SubscribeOnClose(std::function<void()> listener);
		void UnsubscribeOnClose(uint32_t listenerId);
		
		void NavigateTopLevel(int step);
		
	private:
		void ShowPopup(Window* window, Window* owner, bool fromMenuBar);
		
		Window* m_owner{ nullptr };
		uint32_t m_nextListenerId { 1 };
		std::vector<std::pair<uint32_t, std::function<void()>>> m_onCloseListeners;
		std::vector<Window*> m_popups;
		bool m_fromMenuBar{ false };
	};
}

#endif