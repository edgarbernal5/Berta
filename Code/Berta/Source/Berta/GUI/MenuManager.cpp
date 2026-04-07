/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "MenuManager.h"

#include "Berta/GUI/Interface.h"
#include "Berta/Controls/Menu.h"

#include <stack>

namespace Berta
{
    bool MenuManager::AnyPopupActive() const
    {
        return !m_popups.empty();
    }

    void MenuManager::Close(const Window* popupWindow)
    {
        if (!popupWindow || m_popups.empty())
        {
            return;
        }

        auto it = std::find(m_popups.begin(), m_popups.end(), popupWindow);
        if (it == m_popups.end())
        {
            return;
        }

        std::vector<Window*> popupsToClose(it, m_popups.end());

        m_popups.erase(it, m_popups.end());
        
        if (m_popups.empty())
        {
            if (m_owner)
            {
                GUI::ReleaseCapture(m_owner);
                m_owner = nullptr;
            }
            m_fromMenuBar = false;
        }

        for (auto rit = popupsToClose.rbegin(); rit != popupsToClose.rend(); ++rit)
        {
            if (*rit)
            {
                GUI::DisposeWindow(*rit);
            }
        }
    }

    void MenuManager::CloseAll()
    {
        if (m_popups.empty())
        {
            return;
        }
        
        auto popupsToClose = m_popups;
        Window* safeOwner = m_owner;
        
        m_popups.clear();
        m_owner = nullptr;
        m_fromMenuBar = false;
        
        if (safeOwner)
        {
            GUI::ReleaseCapture(safeOwner);
        }
        
        for (auto rit = popupsToClose.rbegin(); rit != popupsToClose.rend(); ++rit)
        {
            if (*rit)
            {
                GUI::DisposeWindow(*rit);
            }
        }
        
        for (const auto& listener : m_onCloseListeners)
        {
            if (listener)
            {
                listener();
            }
        }
    }

    void MenuManager::CloseChildrenOf(Window* parent)
    {
        auto it = std::find(m_popups.begin(), m_popups.end(), parent);
        if (it != m_popups.end() && std::next(it) != m_popups.end())
        {
            Window* firstChild = *std::next(it);
            Close(firstChild); 
        }
    }

    Window* MenuManager::GetActiveMenu(bool fromKeyboard) const
    {
        if (m_popups.empty())
        {
            return nullptr;
        }

        if (fromKeyboard && m_fromMenuBar)
        {
            return m_popups.front();
        }

        return m_popups.back();
    }

    Window* MenuManager::FindMenu(const Point& mousePosition) const
    {
        for (auto it = m_popups.rbegin(); it != m_popups.rend(); ++it)
        {
            Window* menuWindow = *it;
            Point localPosition = GUI::GetPointScreenToClient(menuWindow, mousePosition);

            if (menuWindow->ClientSize.IsInside(localPosition))
            {
                return menuWindow;
            }
        }
        return nullptr;
    }

    size_t MenuManager::GetPopupCount() const
    {
        return m_popups.size();
    }

    void MenuManager::ShowContextMenu(Menu& menuData, Window* owner, const Point& position)
    {
        auto menuBox = new Berta::MenuBox(owner, position);
        menuBox->InitFromData(menuData); 
        
        ShowPopup(menuBox->Handle(), owner, false);
    }

    void MenuManager::ShowMenuBarPopup(Menu& menuData, Window* owner, const Point& position)
    {
        auto menuBox = new Berta::MenuBox(owner, position);
        menuBox->InitFromData(menuData); 
        
        ShowPopup(menuBox->Handle(), owner, true);
    }

    void MenuManager::ClearListeners()
    {
        m_onCloseListeners.clear();
    }

    void MenuManager::SubscribeOnClose(std::function<void()> listener)
    {
        m_onCloseListeners.push_back(std::move(listener));
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
            m_fromMenuBar = true;
        }
        
        m_popups.emplace_back(window);
        GUI::ShowWindow(window, true);
    }

    void MenuManager::NavigateTopLevel(int step)
    {
        if (!m_fromMenuBar || m_popups.empty() || m_owner == nullptr)
        {
            return;
        }
        
        CloseAll();

        //ArgKeyboard args;
        //args.Key = (step > 0) ? VK_RIGHT : VK_LEFT;
    
        // Le decimos a la MenuBar que se mueva y vuelva a abrir el nuevo submenú
        //Foundation::GetInstance().ProcessEvents(m_owner, nullptr, &ControlEvents::KeyPressed, args);
    }
}
