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
		SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		HINSTANCE hInstance = GetModuleHandle(NULL);

		// Register class
		WNDCLASSEXW wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIconW(hInstance, L"IDI_ICON");
		wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wcex.lpszClassName = ApplicationClassName;
		wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");
		if (!RegisterClassExW(&wcex))
		{
			//BR_CORE_ERROR << "RegisterClassExW Failed." << std::endl;
			return;
		}

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
			//BR_CORE_ERROR << "AdjustWindowRectExForDpi Failed." << std::endl;
			return;
		}

		std::wstring mainWndTitle = L"Form";

		m_hwnd = CreateWindowEx
		(
			0,
			ApplicationClassName,
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
			//BR_CORE_ERROR << "CreateWindow Failed." << std::endl;
			return;
		}

		m_hModuleInstance = hInstance;
	}

	LRESULT Form::WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
}