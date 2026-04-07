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
        if (!popupWindow || m_popups.empty()) return;

        // 1. Buscamos en qué posición de la pila está la ventana que queremos cerrar
        auto it = std::find(m_popups.begin(), m_popups.end(), popupWindow);
    
        // Si no la encontramos (ya fue cerrada o no es nuestra), abortamos
        if (it == m_popups.end()) return;

        // 2. SALVAVIDAS: Copiamos la ventana objetivo y TODOS SUS DESCENDIENTES
        // Todo lo que esté desde 'it' hasta el final del vector es la ventana y sus hijos
        std::vector<Window*> popupsToClose(it, m_popups.end());

        // 3. Limpiamos nuestra lista interna ANTES de llamar a los destructores
        m_popups.erase(it, m_popups.end());

        // 4. Si cerramos la ventana raíz, reseteamos el estado del Manager
        if (m_popups.empty())
        {
            m_owner = nullptr;
            m_fromMenuBar = false;
        }

        // 5. Destruimos físicamente las ventanas (en orden inverso es ideal, de hijos a padres)
        for (auto rit = popupsToClose.rbegin(); rit != popupsToClose.rend(); ++rit)
        {
            Window* popup = *rit;
            if (popup)
            {
                GUI::DisposeWindow(popup);
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

        for (Window* popup : popupsToClose)
        {
            if (popup)
            {
                GUI::DisposeWindow(popup);
            }
        }
        if (safeOwner)
        {
            GUI::MarkAsNeedUpdate(safeOwner);
        }
        /*if (m_popups.empty())
        {
            return;
        }

        GUI::ReleaseCapture(m_owner);
        Close(m_popups[0]);
        m_fromMenuBar = false;
        m_owner = nullptr;*/
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
