/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SHORTCUT_HEADER
#define BT_SHORTCUT_HEADER

#include <cstdint>
#include <functional>

namespace Berta
{
    struct Shortcut
    {
        wchar_t Key{ 0 };
        bool Ctrl{ false };
        bool Shift{ false };
        bool Alt{ false };

        bool operator==(const Shortcut& other) const
        {
            return Key == other.Key && Ctrl == other.Ctrl && 
                   Shift == other.Shift && Alt == other.Alt;
        }
    };

    // Hashing para usarlo en std::unordered_map
    struct ShortcutHasher
    {
        std::size_t operator()(const Shortcut& s) const
        {
            std::size_t hash = s.Key;
            hash ^= (s.Ctrl ? 0b001 : 0) << 16;
            hash ^= (s.Shift ? 0b010 : 0) << 16;
            hash ^= (s.Alt ? 0b100 : 0) << 16;
            return hash;
        }
    };
}

#endif