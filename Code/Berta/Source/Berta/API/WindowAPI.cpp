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

		NativeWindowResult CreateNativeWindow(NativeWindowHandle parentHandle, const Rectangle& rectangle, const FormStyle& formStyle, bool isNested)
		{
#ifdef BT_PLATFORM_WINDOWS
			DWORD style = WS_SYSMENU | WS_CLIPCHILDREN;
			DWORD styleEx = WS_EX_NOPARENTNOTIFY;

			if (formStyle.Minimize)
			{
				style |= WS_MINIMIZEBOX;
			}
			if (formStyle.Maximize)
			{
				style |= WS_MAXIMIZEBOX;
			}
			if (formStyle.Sizable)
			{
				style |= WS_THICKFRAME;
			}
			if (formStyle.TitleBarAndCaption)
			{
				style |= WS_OVERLAPPED | WS_CAPTION;
			}
			style |= (isNested ? WS_CHILD : WS_POPUP);

			if (formStyle.AppWindow)
			{
				styleEx |= WS_EX_APPWINDOW;
			}
			else
			{
				styleEx |= WS_EX_TOOLWINDOW;
			}

			if (formStyle.Floating)
			{
				styleEx |= WS_EX_TOPMOST;
			}

			::POINT windowPosition = { rectangle.X, rectangle.Y };
			if (parentHandle && !isNested)
			{
				::ClientToScreen(parentHandle.Handle, &windowPosition);
			}

			::RECT rect = rectangle.ToRECT();

			HINSTANCE hInstance = GetModuleInstance();
			HWND hwnd = ::CreateWindowEx
			(
				styleEx,
				L"BertaInternalClass",
				DefaultWindowTitle.data(),
				style,
				windowPosition.x,
				windowPosition.y,
				50,
				50,
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

			::RECT clientRect;
			::GetClientRect(hwnd, &clientRect);
			::RECT areaWithNonClientRect;
			::GetWindowRect(hwnd, &areaWithNonClientRect);

			areaWithNonClientRect.right -= areaWithNonClientRect.left;	// width
			areaWithNonClientRect.bottom -= areaWithNonClientRect.top;	// height

			if (isNested)
			{
				areaWithNonClientRect.left = windowPosition.x;
				areaWithNonClientRect.top = windowPosition.y;
			}

			int deltaWidth = static_cast<int>(rectangle.Width) - clientRect.right;
			int deltaHeight = static_cast<int>(rectangle.Height) - clientRect.bottom;

			::MoveWindow(hwnd, areaWithNonClientRect.left, areaWithNonClientRect.top, areaWithNonClientRect.right + deltaWidth, areaWithNonClientRect.bottom + deltaHeight, true);

			::GetClientRect(hwnd, &clientRect);
			::GetWindowRect(hwnd, &areaWithNonClientRect);

			areaWithNonClientRect.right -= areaWithNonClientRect.left;
			areaWithNonClientRect.bottom -= areaWithNonClientRect.top;

			auto extraWidth = static_cast<uint32_t>(areaWithNonClientRect.right - clientRect.right);
			auto extraHeight = static_cast<uint32_t>(areaWithNonClientRect.bottom - clientRect.bottom);

			return NativeWindowResult
			{
				{ hwnd },
				{ static_cast<uint32_t>(clientRect.right), static_cast<uint32_t>(clientRect.bottom) },
				{ extraWidth, extraHeight},
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
			else if (!::ReleaseCapture())
			{
				BT_CORE_ERROR << "ReleaseCapture ::GetLastError() = " << ::GetLastError() << std::endl;
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
			return { ::GetAncestor(nativeHandle.Handle, GA_PARENT) };
#else
			return {};
#endif
		}

		void MoveWindow(NativeWindowHandle nativeHandle, const Rectangle& newArea)
		{
#ifdef BT_PLATFORM_WINDOWS
			::MoveWindow(nativeHandle.Handle, newArea.X, newArea.Y, newArea.Width, newArea.Height, true);
#endif
		}

		void MoveWindow(NativeWindowHandle nativeHandle, const Point& newPosition)
		{
#ifdef BT_PLATFORM_WINDOWS
			::RECT nativeRECT;
			::GetWindowRect(nativeHandle.Handle, &nativeRECT);
			HWND owner = ::GetWindow(nativeHandle.Handle, GW_OWNER);

			auto adjustedPosition = newPosition;
			if (owner)
			{
				::RECT ownerRECT;
				::GetWindowRect(owner, &ownerRECT);
				::POINT ownerPosition = { ownerRECT.left, ownerRECT.top };
				::ScreenToClient(owner, &ownerPosition);
				adjustedPosition.X += (ownerRECT.left - ownerPosition.x);
				adjustedPosition.Y += (ownerRECT.top - ownerPosition.y);
			}
			::MoveWindow(nativeHandle.Handle, adjustedPosition.X, adjustedPosition.Y, nativeRECT.right - nativeRECT.left, nativeRECT.bottom - nativeRECT.top, true);
#endif
		}

		void ResizeWindow(NativeWindowHandle nativeHandle, const Size& newSize)
		{
#ifdef BT_PLATFORM_WINDOWS
			::RECT nativeRECT;
			::GetWindowRect(nativeHandle.Handle, &nativeRECT);
			::MoveWindow(nativeHandle.Handle, nativeRECT.left, nativeRECT.top, static_cast<int>(newSize.Width), static_cast<int>(newSize.Height), true);
#else

#endif
		}

		Point GetWindowPosition(NativeWindowHandle nativeHandle)
		{
#ifdef BT_PLATFORM_WINDOWS
			::RECT nativeRECT;
			::GetWindowRect(nativeHandle.Handle, &nativeRECT);
			HWND ownerOrParentHwnd = ::GetWindow(nativeHandle.Handle, GW_OWNER);

			if (!ownerOrParentHwnd)
			{
				ownerOrParentHwnd = ::GetParent(nativeHandle.Handle);
			}

			if (ownerOrParentHwnd)
			{
				::POINT position = { nativeRECT.left, nativeRECT.top };
				::ScreenToClient(ownerOrParentHwnd, &position);
				return { position.x, position.y };
			}
			return { nativeRECT.left, nativeRECT.top };
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
				cursorName = IDC_ARROW;
				break;

			case Cursor::Wait:
				cursorName = IDC_WAIT;
				break;

			case Cursor::IBeam:
				cursorName = IDC_IBEAM;
				break;

			case Cursor::SizeWE:
				cursorName = IDC_SIZEWE;
				break;

			case Cursor::SizeNS:
				cursorName = IDC_SIZENS;
				break;
			}

			nativeCursor.Handle = ::LoadCursor(nullptr, cursorName);
			auto thisCursor = reinterpret_cast<HCURSOR>(::GetClassLongPtr(nativeHandle.Handle, GCLP_HCURSOR));
			if (thisCursor != nativeCursor.Handle)
			{
				::SetClassLongPtr(nativeHandle.Handle, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(nativeCursor.Handle));
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
			if (::ClientToScreen(nativeHandle.Handle, &pointNative))
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
			if (::ScreenToClient(nativeHandle.Handle, &pointNative))
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