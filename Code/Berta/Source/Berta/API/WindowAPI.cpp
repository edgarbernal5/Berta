/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "WindowAPI.h"

namespace Berta
{
#ifdef BT_PLATFORM_WINDOWS
	HINSTANCE GetModuleInstance();
#endif

	namespace API
	{
		NativeWindowResult CreateNativeWindow(const Rectangle& rectangle, const FormStyle& formStyle)
		{
#ifdef BT_PLATFORM_WINDOWS
			DWORD style = WS_SYSMENU | WS_CLIPCHILDREN;
			DWORD styleEx = WS_EX_NOPARENTNOTIFY;

			if (formStyle.Minimize) style |= WS_MINIMIZEBOX;
			if (formStyle.Maximize) style |= WS_MAXIMIZEBOX;

			if (formStyle.Sizable) style |= WS_THICKFRAME;

			style |= WS_OVERLAPPED | WS_CAPTION;
			style |= WS_POPUP;
			styleEx |= WS_EX_APPWINDOW;

			uint32_t dpi = GetNativeWindowDPI({});
			float scalingFactor = static_cast<float>(dpi) / 96.0f;

			// Actually set the appropriate window size
			RECT scaledWindowRect;
			scaledWindowRect.left = 0;
			scaledWindowRect.top = 0;
			scaledWindowRect.right = static_cast<LONG>(rectangle.Width * scalingFactor);
			scaledWindowRect.bottom = static_cast<LONG>(rectangle.Height * scalingFactor);

			if (!::AdjustWindowRectExForDpi(&scaledWindowRect, style, false, 0, dpi))
			{
				BT_CORE_ERROR << "AdjustWindowRectExForDpi Failed." << std::endl;
				return {};
			}

			std::wstring windowTitle = L"Berta Window";
			HINSTANCE hInstance = GetModuleInstance();
			HWND hwnd = ::CreateWindowEx
			(
				styleEx,
				L"BertaInternalClass",
				windowTitle.c_str(),
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

			::RECT client;
			::GetClientRect(hwnd, &client);

			return NativeWindowResult{ NativeWindowHandle{ hwnd },{static_cast<uint32_t>(client.right - client.left), static_cast<uint32_t>(client.bottom - client.top) }, dpi };
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

		std::wstring GetCaptionNativeWindow(NativeWindowHandle nativeHandle)
		{
			std::wstring result;
#ifdef BT_PLATFORM_WINDOWS
			int length = ::GetWindowTextLength(nativeHandle.Handle);
			if (length > 0)
			{
				result.resize(length + 1);
				::GetWindowText(nativeHandle.Handle, result.data(), static_cast<int>(result.size()));
			}
#else
#endif
			return result;
		}

		void DestroyNativeWindow(NativeWindowHandle nativeHandle)
		{
#ifdef BT_PLATFORM_WINDOWS
			if (!::DestroyWindow(nativeHandle.Handle))
			{
			}
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

		void RefreshWindow(NativeWindowHandle nativeHandle)
		{
#ifdef BT_PLATFORM_WINDOWS
			RECT rect;
			::GetClientRect(nativeHandle.Handle, &rect);
			::InvalidateRect(nativeHandle.Handle, &rect, FALSE);
#endif
		}

		uint32_t GetNativeWindowDPI(NativeWindowHandle nativeHandle)
		{
#ifdef BT_PLATFORM_WINDOWS
			if (nativeHandle.Handle == nullptr)
			{
				return ::GetDpiForSystem();
			}
			return ::GetDpiForWindow(nativeHandle.Handle);
#else
			return 96;
#endif
		}
	}
}