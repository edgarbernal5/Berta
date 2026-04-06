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

    void MenuManager::Close(Window* popupWindow)
    {
        if (m_popups.empty())
        {
            return;
        }

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
            return m_popups.front();
        }

        return m_popups.back();
    }

    Window* MenuManager::FindMenu(const Point& mousePosition) const
    {
        // Iteradores modernos (sin advertencias de signed/unsigned)
        for (auto it = m_popups.rbegin(); it != m_popups.rend(); ++it)
        {
            Window* menuWindow = *it;
            // DIP: Delegamos la conversión Win32 a la abstracción de GUI
            Point localPosition = GUI::GetPointScreenToClient(menuWindow, mousePosition);

            if (menuWindow->ClientSize.IsInside(localPosition))
            {
                return menuWindow;
            }
        }
        return nullptr; // No hay llamadas nativas Win32 aquí
        /*
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
#endif*/
    }

    void MenuManager::ShowContextMenu(Menu& menuData, Window* owner, const Point& position)
    {
        // 1. MenuManager crea la representación visual
        auto menuBox = new Berta::MenuBox(owner, position);
    
        // 2. Transfiere los datos al Reactor de la vista
        menuBox->InitFromData(menuData); 
        
        // 3. Reutilizamos tu lógica probada de control de ventanas
        ShowPopup(menuBox->Handle(), owner, false);
        
        /*i
        // 3. Reutiliza tu lógica existente de gestión de ventanas
        // (Esto es exactamente lo que hacías antes en MenuBox::Popup)
        f (m_popups.empty())
        {
            m_owner = owner;
            GUI::Capture(m_owner); // Mantiene el mismo comportamiento de foco
        }
    
        m_popups.push_back(menuBox->Handle());
        GUI::MakeWindowActive(menuBox->Handle(), true, nullptr);*/
    }

    void MenuManager::ShowMenuBarPopup(const Menu& menuData, Window* owner)
    {
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
            if (std::find(m_popups.begin(), m_popups.end(), owner) == m_popups.end())
            {
                m_popups.emplace_back(owner);
            }
        }
        m_popups.emplace_back(window);
        GUI::MakeWindowActive(window, true, nullptr);
    }

    void MenuManager::NavigateTopLevel(int step)
    {
        // 1. Verificamos que el menú se haya originado desde una MenuBar
        if (!m_fromMenuBar || m_popups.empty() || m_owner == nullptr)
        {
            return;
        }
        
        // 2. Cerramos todos los MenuBoxes flotantes limpiecita
        CloseAll();

        // 3. Enviamos un evento "sintético" de teclado directamente a la MenuBar 
        // para decirle que se mueva a la Izquierda (-1) o Derecha (1)
        ArgKeyboard args;
        args.Key = (step > 0) ? VK_RIGHT : VK_LEFT;
    
        // Le decimos a la MenuBar que se mueva y vuelva a abrir el nuevo submenú
        //Foundation::GetInstance().ProcessEvents(m_owner, nullptr, &ControlEvents::KeyPressed, args);
    }

    size_t MenuManager::GetPopupCount() const
    {
        return m_popups.size();
    }
}
