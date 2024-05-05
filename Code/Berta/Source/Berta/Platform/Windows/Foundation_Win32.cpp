/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"

#ifdef BT_PLATFORM_WINDOWS
#include "Berta/Core/Foundation.h"

#include "Berta/Core/Base.h"
#include "Berta/Core/Log.h"
#include "Berta/GUI/BasicWindow.h"

namespace Berta
{
	LRESULT CALLBACK Foundation_WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);

	HINSTANCE g_hModuleInstance;
	HINSTANCE GetModuleInstance()
	{
		if (g_hModuleInstance == nullptr)
			g_hModuleInstance = GetModuleHandle(NULL);

		return g_hModuleInstance;
	}

	Foundation::Foundation()
	{
		InitializeCore();
		m_logger = Log::GetCoreLogger();
		BT_CORE_TRACE << "Foundation init..." << std::endl;

		SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		HINSTANCE hInstance = GetModuleInstance();

		// Register class
		WNDCLASSEXW wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
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

#ifdef BT_DEBUG
	std::map<uint32_t, std::string> g_debugWndMessages{
		//{WM_CREATE,			"WM_CREATE"},
		{WM_SIZE,			"WM_SIZE"},
		{WM_DESTROY,		"WM_DESTROY"},
		{WM_SHOWWINDOW,		"WM_SHOWWINDOW"},
		//{WM_ACTIVATEAPP,	"WM_ACTIVATEAPP"},
		{WM_PAINT,			"WM_PAINT"}
	};
#endif

	LRESULT CALLBACK Foundation_WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
	{
#ifdef BT_DEBUG
		auto it = g_debugWndMessages.find(message);
		if (it != g_debugWndMessages.end())
		{
			BT_CORE_DEBUG << "WndProc message: " << it->second << std::endl;
		}
#endif

		auto& foundation = Foundation::GetInstance();

		API::NativeWindowHandle nativeWindowHandle{ hWnd };
		auto& windowManager = foundation.GetWindowManager();
		auto nativeWindow = windowManager.Get(nativeWindowHandle);
		if (nativeWindow == nullptr)
		{
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		switch (message)
		{
		case WM_SHOWWINDOW:
		{
			windowManager.UpdateTree(nativeWindow);
		}break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			::BeginPaint(nativeWindow->Root.Handle, &ps);

			Rectangle areaToUpdate;
			areaToUpdate.FromRECT(ps.rcPaint);
			nativeWindow->Renderer.Map(nativeWindow, areaToUpdate);  // Copy from widget's graphics to native hwnd window.

			::EndPaint(nativeWindow->Root.Handle, &ps); 
			return 0;
		}
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
		}

		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}
}
#endif