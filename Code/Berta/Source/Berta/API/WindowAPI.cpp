/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "WindowAPI.h"

namespace Berta::API
{
	NativeWindowHandle CreateNativeWindow(const Rectangle& rectangle, const WindowStyle& windowStyle)
	{
#ifdef BT_PLATFORM_WINDOWS
		DWORD style = WS_SYSMENU | WS_CLIPCHILDREN;
		DWORD style_ex = WS_EX_NOPARENTNOTIFY;

		if (windowStyle.Minimize) style |= WS_MINIMIZEBOX;
		if (windowStyle.Maximize) style |= WS_MAXIMIZEBOX;

		if (windowStyle.Sizable) style |= WS_THICKFRAME;

		style |= WS_OVERLAPPED | WS_CAPTION;
		style |= WS_POPUP;
		style_ex |= WS_EX_APPWINDOW;

		UINT dpi = ::GetDpiForSystem();
		float scalingFactor = static_cast<float>(dpi) / 96.0f;

		// Actually set the appropriate window size
		RECT scaledWindowRect;
		scaledWindowRect.left = 0;
		scaledWindowRect.top = 0;
		scaledWindowRect.right = static_cast<LONG>(rectangle.width * scalingFactor);
		scaledWindowRect.bottom = static_cast<LONG>(rectangle.height * scalingFactor);

		if (!AdjustWindowRectExForDpi(&scaledWindowRect, WS_OVERLAPPEDWINDOW, false, 0, dpi))
		{
			BT_CORE_ERROR << "AdjustWindowRectExForDpi Failed." << std::endl;
			return {};
		}

		std::wstring mainWndTitle = L"Berta Window";
		HINSTANCE hInstance = GetModuleHandle(NULL);
		HWND hwnd = ::CreateWindowEx
		(
			style_ex,
			L"BertaInternalClass",
			mainWndTitle.c_str(),
			style,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			scaledWindowRect.right - scaledWindowRect.left,
			scaledWindowRect.bottom - scaledWindowRect.top,
			nullptr,			// We have no parent window.
			nullptr,			// We aren't using menus.
			hInstance,
			0
		);

		if (!hwnd)
		{
			BT_CORE_ERROR << "CreateWindow Failed." << std::endl;

			return {};
		}

		return NativeWindowHandle { hwnd };
#else
		return {};
#endif
	}

	void CaptionNativeWindow(NativeWindowHandle nativeHandle, const std::wstring& caption)
	{
#ifdef BT_PLATFORM_WINDOWS
		::SetWindowText(nativeHandle.Handle, caption.c_str());
#else
#endif
	}

	void DestroyNativeWindow(NativeWindowHandle nativeHandle)
	{
#ifdef BT_PLATFORM_WINDOWS
		::DestroyWindow(nativeHandle.Handle);
#else
#endif
	}

	void ShowNativeWindow(NativeWindowHandle nativeHandle, bool visible)
	{
#ifdef BT_PLATFORM_WINDOWS
		::ShowWindow(nativeHandle.Handle, visible ? SW_SHOW : SW_HIDE);
#else
#endif
	}
}