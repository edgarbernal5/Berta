/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Berta/Core/Foundation.h"

#include "Berta/Core/Base.h"
#include "Berta/Core/Log.h"

namespace Berta
{
	HINSTANCE g_hModuleInstance;

	static LRESULT CALLBACK Foundation_WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);
	static HINSTANCE GetModuleInstance();

	Foundation::Foundation()
	{
		InitializeCore();
		m_logger = Log::GetCoreLogger();
		BT_CORE_TRACE << "Foundation init..." << std::endl;

		SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		HINSTANCE hInstance = GetModuleHandle(NULL);

		// Register class
		WNDCLASSEXW wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = Foundation_WndProc;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIconW(hInstance, L"IDI_ICON");
		wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wcex.lpszClassName = L"BertaInternalClass";
		wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");
		if (!RegisterClassExW(&wcex))
		{
			BT_CORE_ERROR << "RegisterClassExW Failed." << std::endl;
			return;
		}

		g_hModuleInstance = hInstance;
	}

	Foundation::~Foundation()
	{
		BT_CORE_TRACE << "Releasing foundation..." << std::endl;

		UnregisterClass(L"BertaInternalClass", g_hModuleInstance);
		g_hModuleInstance = nullptr;

		ShutdownCore();
	}

	void Foundation::ProcessMessages()
	{
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

	LRESULT CALLBACK Foundation_WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = 0;

		auto& foundation = Foundation::GetInstance();

		Berta::API::NativeWindowHandle nativeWindowHandle{ hWnd };

		auto nativeWindow = foundation.GetWindowManager().Get(nativeWindowHandle);

		if (nativeWindow == nullptr)
		{
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		switch (message)
		{
		case WM_PAINT:
		{
			BT_CORE_TRACE << "WM_PAINT" << std::endl;

			PAINTSTRUCT paint;
			::BeginPaint(hWnd, &paint);

			::EndPaint(hWnd, &paint);
		}break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
		default:
			result = ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		return result;
	}

	HINSTANCE GetModuleInstance()
	{
		if (g_hModuleInstance == nullptr)
			g_hModuleInstance = GetModuleHandle(NULL);

		return g_hModuleInstance;
	}
}