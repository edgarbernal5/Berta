/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "MenuManager.h"

#include "Berta/Core/Foundation.h"
#include "Berta/GUI/Interface.h"

namespace Berta
{
    bool MenuManager::AnyPopupActive() const
    {
        return !m_popups.empty();
    }

    Window* MenuManager::GetActiveMenu() const
    {
        if (m_popups.empty())
            return nullptr;

        return m_popups.back();
    }

    Window* MenuManager::FindMenu(const Point& mousePosition) const
    {
#ifdef BT_PLATFORM_WINDOWS
        auto& windowManager = Foundation::GetInstance().GetWindowManager();

        for (size_t i = 0; i < m_popups.size(); i++)
        {
            POINT screenToClientPoint{ mousePosition.X,   mousePosition.Y };
            auto menuWindow = m_popups[i];
            ::ScreenToClient(menuWindow->RootHandle.Handle, &screenToClientPoint);

            auto localPosition = Point{ (int)screenToClientPoint.x, (int)screenToClientPoint.y } - windowManager.GetAbsoluteRootPosition(menuWindow);
            if (menuWindow->ClientSize.IsInside(localPosition))
            {
                return menuWindow;
            }
        }

        return nullptr;
#else
        return nullptr;
#endif
    }

    void MenuManager::ShowPopup(Window* window, Window* owner, bool fromMenuBar)
    {
        GUI::Capture(owner);
        m_popups.emplace_back(window);
    }
}