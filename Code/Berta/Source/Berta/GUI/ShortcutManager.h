/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SHORTCUT_MANAGER_HEADER
#define BT_SHORTCUT_MANAGER_HEADER

#include <functional>

namespace Berta
{
    class ShortcutManager
    {
    public:
        void RegisterShortcut(int modifierKeys, int vKey, std::function<void()> action);
        bool ProcessKey(int modifierKeys, int vKey);
    };
}

#endif