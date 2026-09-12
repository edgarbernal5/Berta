/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SHORTCUT_MANAGER_HEADER
#define BT_SHORTCUT_MANAGER_HEADER

#include <functional>
#include <unordered_map>

#include "Shortcut.h"

namespace Berta
{
    class ShortcutManager
    {
    public:
        using ShortcutCallback = std::function<void()>;

        void Register(const Shortcut& shortcut, ShortcutCallback callback);

        void Unregister(const Shortcut& shortcut);

        // Retorna true si el atajo fue consumido
        bool Execute(const Shortcut& shortcut) const;

    private:
        std::unordered_map<Shortcut, ShortcutCallback, ShortcutHasher> m_shortcuts;
    };
}

#endif