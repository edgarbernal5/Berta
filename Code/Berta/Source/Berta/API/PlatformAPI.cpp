/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PlatformAPI.h"

namespace Berta::Platform
{
    void GetClipboardText(std::wstring& output)
    {
#ifdef BT_PLATFORM_WINDOWS
        if(::OpenClipboard(::GetFocus()))
        {
            HANDLE hData = ::GetClipboardData(CF_UNICODETEXT);
            if (hData == nullptr)
            {
                ::CloseClipboard();
                return;
            }
            
            wchar_t* pszText = static_cast<wchar_t*>(::GlobalLock(hData));
            if (pszText == nullptr)
            {
                ::CloseClipboard();
                return;
            }

            output = pszText;
            ::GlobalUnlock(hData);
            ::CloseClipboard();
        }
#else
       
#endif
    }

    bool SetClipboardText(const std::wstring& text, API::NativeWindowHandle owner)
    {
        bool success = false;
#ifdef BT_PLATFORM_WINDOWS
        if (!OpenClipboard(owner.Handle))
        {
            return false;
        }
        ::EmptyClipboard();

        size_t size = (text.size() + 1) * sizeof(wchar_t);
        HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, size);
        if (hMem)
        {
            ::memcpy(::GlobalLock(hMem), text.c_str(), size);
            ::GlobalUnlock(hMem);
            success = ::SetClipboardData(CF_UNICODETEXT, hMem) != nullptr;
        }
        ::CloseClipboard();
#endif
        return success;
    }
}
