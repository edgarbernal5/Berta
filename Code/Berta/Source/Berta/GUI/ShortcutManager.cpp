/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ShortcutManager.h"

namespace Berta
{
    void ShortcutManager::Register(const Shortcut& shortcut, ShortcutCallback callback)
    {
        m_shortcuts[shortcut] = std::move(callback);
    }

    void ShortcutManager::Unregister(const Shortcut& shortcut)
    {
        m_shortcuts.erase(shortcut);
    }

    bool ShortcutManager::Execute(const Shortcut& shortcut) const
    {
        auto it = m_shortcuts.find(shortcut);
        if (it != m_shortcuts.end())
        {
            if (it->second)
            {
                it->second();
            }
            return true; 
        }
        return false;
    }
}
