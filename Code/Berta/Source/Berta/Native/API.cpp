#include "btpch.h"
#include "API.h"


namespace Berta::API
{
	NativeWindowHandle Create_Window(const Rectangle& rectangle)
	{
#ifdef BT_PLATFORM_WINDOWS
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
		HWND hwnd = CreateWindowEx
		(
			0,
			L"BertaInternalClass",
			mainWndTitle.c_str(),
			WS_OVERLAPPEDWINDOW,
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

		return NativeWindowHandle{ hwnd };
#endif
	}
}