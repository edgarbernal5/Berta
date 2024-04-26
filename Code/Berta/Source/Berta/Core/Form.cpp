/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Form.h"

namespace Berta
{
	Form::Form(const Rectangle& rectangle)
	{
		Create(rectangle);
	}

	void Form::Create(const Rectangle& rectangle)
	{
		UINT dpi = GetDpiForSystem();
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
			return;
		}

		std::wstring mainWndTitle = L"Form";
		HINSTANCE hInstance = GetModuleHandle(NULL);
		m_hwnd = CreateWindowEx
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
			this
		);

		if (!m_hwnd)
		{
			BT_CORE_ERROR << "CreateWindow Failed." << std::endl;
			return;
		}
		//TODO: HACK
		ShowWindow(m_hwnd, SW_SHOW);
		//m_hModuleInstance = hInstance;
		MSG msg = { 0 };

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}