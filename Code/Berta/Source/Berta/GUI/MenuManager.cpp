/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "MenuManager.h"

#include "Berta/GUI/Interface.h"

#include <stack>

namespace Berta
{
    bool MenuManager::AnyPopupActive() const
    {
        return !m_popups.empty();
    }

    void MenuManager::Close(Window* popupWindow)
    {
        if (m_popups.empty())
            return;

        std::stack<Window*> popups;
        for (size_t i = 0; i < m_popups.size(); i++)
        {
            if (m_popups[i] == popupWindow)
            {
                for (size_t j = i; j < m_popups.size(); j++)
                {
                    popups.push(m_popups[j]);
                }
                break;
            }
        }

        while (!popups.empty())
        {
            auto current = popups.top();
            popups.pop();

            for (size_t i = 0; i < m_popups.size(); i++)
            {
                if (m_popups[i] == current)
                {
                    m_popups.erase(m_popups.begin() + i);
                    break;
                }
            }

            if (m_fromMenuBar && m_popups.empty())
            {
                break;
            }
            GUI::DisposeWindow(current);
        }
    }

    void MenuManager::CloseAll()
    {
        if (m_popups.empty())
        {
            return;
        }

        GUI::ReleaseCapture(m_owner);
        Close(m_popups[0]);
        m_fromMenuBar = false;
        m_owner = nullptr;
    }

    Window* MenuManager::GetActiveMenu(bool fromKeyboard) const
    {
        if (m_popups.empty())
        {
            return nullptr;
        }

        if (fromKeyboard && m_fromMenuBar)
        {
            return m_popups[0];
        }

        return m_popups.back();
    }

    Window* MenuManager::FindMenu(const Point& mousePosition) const
    {
#ifdef BT_PLATFORM_WINDOWS
        for (int i = m_popups.size() - 1; i >=0 ; --i)
        {
            POINT screenToClientPoint{ mousePosition.X, mousePosition.Y };
            auto menuWindow = m_popups[i];
            ::ScreenToClient(menuWindow->RootHandle.Handle, &screenToClientPoint);

            auto localPosition = Point{ (int)screenToClientPoint.x, (int)screenToClientPoint.y } - GUI::GetWindowRootPosition(menuWindow);
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
        if (m_popups.empty())
        {
            m_owner = fromMenuBar ? owner : window;
            GUI::Capture(m_owner);
        }
        if (fromMenuBar)
        {
            if (std::find(m_popups.begin(), m_popups.end(), owner) == m_popups.end())
            {
                m_popups.emplace_back(owner);
            }
            m_fromMenuBar = true;
        }
        m_popups.emplace_back(window);
    }
}