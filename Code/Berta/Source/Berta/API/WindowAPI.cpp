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
		namespace
		{
#ifdef BT_PLATFORM_WINDOWS
			constexpr std::wstring_view DefaultWindowTitle = L"Berta Window";

			constexpr RECT CreateScaledRect(const Rectangle& rectangle, float scalingFactor)
			{
				return RECT{
					static_cast<LONG>(rectangle.X * scalingFactor),
					static_cast<LONG>(rectangle.Y * scalingFactor),
					static_cast<LONG>((rectangle.X + rectangle.Width) * scalingFactor),
					static_cast<LONG>((rectangle.Y + rectangle.Height) * scalingFactor)
				};
			}
		}
#endif

		NativeWindowResult CreateNativeWindow(NativeWindowHandle parentHandle, const Rectangle& rectangle, const FormStyle& formStyle)
		{
#ifdef BT_PLATFORM_WINDOWS
			DWORD style = WS_SYSMENU | WS_CLIPCHILDREN;
			DWORD styleEx = WS_EX_NOPARENTNOTIFY;

			if (formStyle.Minimize)
				style |= WS_MINIMIZEBOX;
			if (formStyle.Maximize)
				style |= WS_MAXIMIZEBOX;
			if (formStyle.Sizable)
				style |= WS_THICKFRAME;

			if (formStyle.TitleBarAndCaption)
				style |= WS_OVERLAPPED | WS_CAPTION;

			style |= WS_POPUP;
			if (formStyle.AppWindow)
				styleEx |= WS_EX_APPWINDOW;
			else
				styleEx |= WS_EX_TOOLWINDOW;

			if (formStyle.Floating)
				styleEx |= WS_EX_TOPMOST;

			RECT rect = rectangle.ToRECT();

			HINSTANCE hInstance = GetModuleInstance();
			HWND hwnd = ::CreateWindowEx
			(
				styleEx,
				L"BertaInternalClass",
				DefaultWindowTitle.data(),
				style,
				rect.left,
				rect.top,
				rect.right - rect.left,
				rect.bottom - rect.top,
				parentHandle.Handle,	// Parent
				nullptr,				// We aren't using menus.
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

			return NativeWindowResult
			{
				{ hwnd },
				{static_cast<uint32_t>(client.right - client.left), static_cast<uint32_t>(client.bottom - client.top) },
				GetNativeWindowDPI(parentHandle) 
			};
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
#ifdef BT_PLATFORM_WINDOWS
			int length = ::GetWindowTextLength(nativeHandle.Handle);
			if (length > 0)
			{
				std::wstring result(length + 1, L'\0');
				::GetWindowText(nativeHandle.Handle, result.data(), length + 1);
				result.resize(length);
				return result;
			}
#else
#endif
			return {};
		}

		void DestroyNativeWindow(NativeWindowHandle nativeHandle)
		{
#ifdef BT_PLATFORM_WINDOWS
			if (!::DestroyWindow(nativeHandle.Handle))
			{
				BT_CORE_ERROR << "DestroyWindow Failed. GetLastError() = " << ::GetLastError() <<  std::endl;
			}
#else
#endif
		}

		void ShowNativeWindow(NativeWindowHandle nativeHandle, bool visible, bool active)
		{
#ifdef BT_PLATFORM_WINDOWS
			::ShowWindow(nativeHandle.Handle, visible ? (active ? SW_SHOW : SW_SHOWNA) : SW_HIDE);
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

		void CaptureWindow(NativeWindowHandle nativeHandle, bool capture)
		{
#ifdef BT_PLATFORM_WINDOWS
			if (capture)
			{
				::SetCapture(nativeHandle.Handle);
			}
			else
			{
				if (!::ReleaseCapture())
				{
					BT_CORE_ERROR << "ReleaseCapture ::GetLastError() = " << ::GetLastError() << std::endl;
				}
			}
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
			return 96u;
#endif
		}

		void EnableWindow(NativeWindowHandle nativeHandle, bool isEnabled)
		{
#ifdef BT_PLATFORM_WINDOWS
			::EnableWindow(nativeHandle.Handle, isEnabled);
#else
#endif
		}

		NativeWindowHandle GetParentWindow(NativeWindowHandle nativeHandle)
		{
#ifdef BT_PLATFORM_WINDOWS
			return { ::GetAncestor(reinterpret_cast<HWND>(nativeHandle.Handle), GA_PARENT) };
#else
			return {};
#endif
		}

		bool ChangeCursor(NativeWindowHandle nativeHandle, Cursor newCursor, NativeCursor& nativeCursor)
		{
#ifdef BT_PLATFORM_WINDOWS
			const wchar_t* cursorName = IDC_ARROW;

			switch (newCursor)
			{
			case Cursor::Default:
				cursorName = IDC_ARROW;	break;

			case Cursor::Wait:
				cursorName = IDC_WAIT;	break;

			case Cursor::IBeam:
				cursorName = IDC_IBEAM;	break;

			case Cursor::SizeWE:
				cursorName = IDC_SIZEWE;	break;
			}

			nativeCursor.Handle = ::LoadCursor(nullptr, cursorName);
			auto thisCursor = reinterpret_cast<HCURSOR>(::GetClassLongPtr(reinterpret_cast<HWND>(nativeHandle.Handle), GCLP_HCURSOR));
			if (thisCursor != nativeCursor.Handle)
			{
				::SetClassLongPtr(reinterpret_cast<HWND>(nativeHandle.Handle), GCLP_HCURSOR,
					reinterpret_cast<LONG_PTR>(nativeCursor.Handle));
			}
#endif
			return true;
		}

		Size GetPrimaryMonitorSize()
		{
#ifdef BT_PLATFORM_WINDOWS
			return {
				static_cast<uint32_t>(::GetSystemMetrics(SM_CXSCREEN)), 
				static_cast<uint32_t>(::GetSystemMetrics(SM_CYSCREEN))
			};
#else
			return {};
#endif
		}

		Point GetPointClientToScreen(NativeWindowHandle nativeHandle, const Point& point)
		{
#ifdef BT_PLATFORM_WINDOWS
			::POINT pointNative = { point.X, point.Y };
			if (::ClientToScreen(reinterpret_cast<HWND>(nativeHandle.Handle), &pointNative))
			{
				return { static_cast<int>(pointNative.x), static_cast<int>(pointNative.y) };
			}
			return {};
#else
			return {};
#endif
		}

		Point GetPointScreenToClient(NativeWindowHandle nativeHandle, const Point& point)
		{
#ifdef BT_PLATFORM_WINDOWS
			::POINT pointNative = { point.X, point.Y };
			if (::ScreenToClient(reinterpret_cast<HWND>(nativeHandle.Handle), &pointNative))
			{
				return { static_cast<int>(pointNative.x), static_cast<int>(pointNative.y) };
			}

			return point;
#else
			return {};
#endif
		}

		void SendCustomMessage(API::NativeWindowHandle nativeHandle, std::function<void()> body)
		{
#ifdef BT_PLATFORM_WINDOWS
			auto param = new CustomCallbackMessage();
			param->Body = body;
			::PostMessage(nativeHandle.Handle, static_cast<UINT>(CustomMessageId::CustomCallback), reinterpret_cast<WPARAM>(param), 0);
#endif
		}

		Point GetScreenMousePosition()
		{
#ifdef BT_PLATFORM_WINDOWS
			::POINT nativePoint;
			::GetCursorPos(&nativePoint);
			return Point(nativePoint.x, nativePoint.y);
#else
			return {};
#endif
		}
	}
}