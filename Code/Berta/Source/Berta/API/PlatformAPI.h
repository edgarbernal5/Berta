/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PLATFORM_API_HEADER
#define BT_PLATFORM_API_HEADER

#include "Berta/API/WindowAPI.h"

namespace Berta::Platform
{
    bool GetClipboardText(std::wstring& output);
    bool SetClipboardText(const std::wstring& text, API::NativeWindowHandle owner);
}

#endif
