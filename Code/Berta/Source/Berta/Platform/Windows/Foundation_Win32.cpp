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
#include "Berta/GUI/Window.h"
#include "Berta/GUI/CommonEvents.h"

namespace Berta
{
	LRESULT CALLBACK Foundation_WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);

	HINSTANCE g_hModuleInstance;
	HINSTANCE GetModuleInstance()
	{
		if (g_hModuleInstance == nullptr)
		{
			g_hModuleInstance = GetModuleHandle(NULL);
		}

		return g_hModuleInstance;
	}

	Foundation::Foundation()
	{
		InitializeCore();
		m_logger = Log::GetCoreLogger();
		BT_CORE_TRACE << "Foundation init..." << std::endl;

		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
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
	std::map<uint32_t, std::string> g_debugWndMessages
	{
		{WM_CREATE,			"WM_CREATE"},
		{WM_NCCREATE,		"WM_NCCREATE"},
		{WM_SIZE,			"WM_SIZE"},
		{WM_SIZING,			"WM_SIZING"},
		{WM_ENTERSIZEMOVE,	"WM_ENTERSIZEMOVE"},
		{WM_EXITSIZEMOVE,	"WM_EXITSIZEMOVE"},

		{WM_DESTROY,		"WM_DESTROY"},
		{WM_NCDESTROY,		"WM_NCDESTROY"},
		{WM_SETFOCUS,		"WM_SETFOCUS"},
		{WM_KILLFOCUS,		"WM_KILLFOCUS"},

		{WM_CHAR,			"WM_CHAR"},
		{WM_KEYDOWN,		"WM_KEYDOWN"},
		{WM_KEYUP,			"WM_KEYUP"},
		{WM_SYSKEYDOWN,		"WM_SYSKEYDOWN"},
		{WM_SYSKEYUP,		"WM_SYSKEYUP"},

		{WM_SHOWWINDOW,		"WM_SHOWWINDOW"},
		{WM_ACTIVATEAPP,	"WM_ACTIVATEAPP"},
		{WM_PAINT,			"WM_PAINT"},
		{WM_DPICHANGED,		"WM_DPICHANGED"},
		{WM_SETCURSOR,		"WM_SETCURSOR"},

		//{WM_MOUSELEAVE,		"WM_MOUSELEAVE"},
		/*{WM_LBUTTONDOWN,	"WM_LBUTTONDOWN"},
		{WM_MBUTTONDOWN,	"WM_MBUTTONDOWN"},
		{WM_RBUTTONDOWN,	"WM_RBUTTONDOWN"},
		{WM_LBUTTONUP,		"WM_LBUTTONUP"},
		{WM_MBUTTONUP,		"WM_MBUTTONUP"},
		{WM_RBUTTONUP,		"WM_RBUTTONUP"},*/
		//{WM_MOUSEMOVE,		"WM_MOUSEMOVE"}
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
		static TRACKMOUSEEVENT trackEvent = {sizeof(trackEvent), TME_LEAVE };
		auto& foundation = Foundation::GetInstance();

		API::NativeWindowHandle nativeWindowHandle{ hWnd };
		auto& windowManager = foundation.GetWindowManager();
		auto nativeWindow = windowManager.Get(nativeWindowHandle);
		if (nativeWindow == nullptr)
		{
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		bool defaultToWindowProc = true;
		auto& rootWindowData = *windowManager.GetWindowData(nativeWindowHandle);

		switch (message)
		{
		case WM_ACTIVATEAPP:
		{
			ArgActivated argActivated;
			argActivated.IsActivated = wParam ? true : false;
			auto events = dynamic_cast<RootEvents*>(nativeWindow->Events.get());
			events->Activated.Emit(argActivated);
			break;
		}
		case WM_SHOWWINDOW:
		{
			windowManager.UpdateTree(nativeWindow);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			::BeginPaint(nativeWindow->RootHandle.Handle, &ps);

			Rectangle areaToUpdate;
			areaToUpdate.FromRECT(ps.rcPaint);
			nativeWindow->Renderer.Map(nativeWindow, areaToUpdate);  // Copy from control's graphics to native hwnd window.

			::EndPaint(nativeWindow->RootHandle.Handle, &ps);
			defaultToWindowProc = false;
			break;
		}
		case WM_SIZE:
		{
			uint32_t newWidth = (uint32_t)LOWORD(lParam);
			uint32_t newHeight = (uint32_t)HIWORD(lParam);

			if (newWidth > 0 && newHeight > 0)
			{
				ArgSize argSize;
				argSize.NewSize.Width = newWidth;
				argSize.NewSize.Height = newHeight;
				BT_CORE_DEBUG << "Size: new size " << argSize.NewSize << std::endl;

				windowManager.Resize(nativeWindow, argSize.NewSize);
				nativeWindow->Events->Size.Emit(argSize);
			}
			break;
		}
		case WM_DPICHANGED:
		{
			uint32_t newDPI = (uint32_t)HIWORD(wParam);
			windowManager.ChangeDPI(nativeWindow, newDPI);

			auto rect = reinterpret_cast<const RECT*>(lParam);

			::SetWindowPos(hWnd,
				NULL,
				rect->left,
				rect->top,
				rect->right - rect->left,
				rect->bottom - rect->top,
				SWP_NOZORDER | SWP_NOACTIVATE);

			defaultToWindowProc = false;
			break;
		}
		case WM_SETFOCUS:
		{
			if (rootWindowData.Focused)
			{
				ArgFocus argFocus{ true };
				rootWindowData.Focused->Renderer.Focus(argFocus);
				rootWindowData.Focused->Events->Focus.Emit(argFocus);
			}
			defaultToWindowProc = false;
			break;
		}
		case WM_KILLFOCUS:
		{
			if (rootWindowData.Focused)
			{
				ArgFocus argFocus{ false };
				rootWindowData.Focused->Renderer.Focus(argFocus);
				rootWindowData.Focused->Events->Focus.Emit(argFocus);
			}
			defaultToWindowProc = false;
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			ArgMouse argMouseDown;
			argMouseDown.Position = { x, y };
			argMouseDown.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
			argMouseDown.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
			argMouseDown.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

			auto window = windowManager.Find(nativeWindow, { x, y });
			rootWindowData.Pressed = window;

			if (window)
			{
				window->Renderer.MouseDown(argMouseDown);
				window->Events->MouseDown.Emit(argMouseDown);
			}
			if (rootWindowData.Focused != window)
			{
				if (rootWindowData.Focused)
				{
					ArgFocus argFocus{ false };
					rootWindowData.Focused->Renderer.Focus(argFocus);
					rootWindowData.Focused->Events->Focus.Emit(argFocus);
				}
				if (window)
				{
					ArgFocus argFocus{ true };
					window->Renderer.Focus(argFocus);
					window->Events->Focus.Emit(argFocus);
				}
			}
			rootWindowData.Focused = window;
			break;
		}
		case WM_MOUSEMOVE:
		{
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			ArgMouse argMouseMove;
			argMouseMove.Position = { x, y };
			argMouseMove.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
			argMouseMove.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
			argMouseMove.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

			auto window = windowManager.Find(nativeWindow, {x, y});
			if (window != rootWindowData.Hovered)
			{
				/*if (window)
				{
					std::string debugWindow(window->Title.begin(), window->Title.end());
					BT_CORE_DEBUG << "mouse move: window: " << debugWindow << std::endl;
				}
				else
				{
					BT_CORE_DEBUG << "mouse move: window: NULL." << std::endl;
				}*/

				if (rootWindowData.Hovered)
				{
					ArgMouse argMouseLeave;
					argMouseLeave.Position = { x, y };
					argMouseLeave.ButtonState.LeftButton = false;
					argMouseLeave.ButtonState.RightButton = false;
					argMouseLeave.ButtonState.MiddleButton = false;

					rootWindowData.Hovered->Renderer.MouseLeave(argMouseMove);
					rootWindowData.Hovered->Events->MouseLeave.Emit(argMouseLeave);

					rootWindowData.Hovered = nullptr;
				}

				if (window)
				{
					ArgMouse argMouseEnter;
					argMouseEnter.Position = { x, y };
					argMouseEnter.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
					argMouseEnter.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
					argMouseEnter.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

					window->Renderer.MouseEnter(argMouseMove);
					window->Events->MouseEnter.Emit(argMouseEnter);
				}
			}

			if (window)
			{
				window->Renderer.MouseMove(argMouseMove);
				window->Events->MouseMove.Emit(argMouseMove);

				trackEvent.hwndTrack = hWnd;
				TrackMouseEvent(&trackEvent); //Keep track of mouse position to Emit WM_MOUSELEAVE message.
			}
			
			rootWindowData.Hovered = window;
			break;
		}
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			ArgMouse argMouseUp;
			argMouseUp.Position = { x, y };
			argMouseUp.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
			argMouseUp.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
			argMouseUp.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

			auto window = windowManager.Find(nativeWindow, { x, y });
			if (window && window == rootWindowData.Pressed)
			{
				window->Renderer.MouseUp(argMouseUp);
				window->Events->MouseUp.Emit(argMouseUp);

				ArgClick argClick;
				window->Renderer.Click(argClick);
				window->Events->Click.Emit(argClick);
			}
			rootWindowData.Pressed = nullptr;
			break;
		}
		case WM_MOUSELEAVE:
		{
			if (rootWindowData.Hovered)
			{
				ArgMouse argMouseLeave;
				rootWindowData.Hovered->Renderer.MouseLeave(argMouseLeave);
				rootWindowData.Hovered->Events->MouseLeave.Emit(argMouseLeave);

				rootWindowData.Hovered = nullptr;
			}
			break;
		}
		case WM_CHAR:
		{
			ArgKeyboard argKeyboard;

			argKeyboard.ButtonState.Alt = (0 != (::GetKeyState(VK_MENU) & 0x80));
			argKeyboard.ButtonState.Ctrl = (0 != (::GetKeyState(VK_CONTROL) & 0x80));
			argKeyboard.ButtonState.Shift = (0 != (::GetKeyState(VK_SHIFT) & 0x80));

			argKeyboard.Key = static_cast<wchar_t>(wParam);

			auto window = rootWindowData.Focused;
			if (window == nullptr) window = nativeWindow;

			window->Renderer.KeyChar(argKeyboard);
			window->Events->KeyChar.Emit(argKeyboard);

			defaultToWindowProc = false;
			break;
		}
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		{
			ArgKeyboard argKeyboard;

			argKeyboard.ButtonState.Alt = (0 != (::GetKeyState(VK_MENU) & 0x80));
			argKeyboard.ButtonState.Ctrl = (0 != (::GetKeyState(VK_CONTROL) & 0x80));
			argKeyboard.ButtonState.Shift = (0 != (::GetKeyState(VK_SHIFT) & 0x80));
			argKeyboard.Key = static_cast<wchar_t>(wParam);

			auto window = rootWindowData.Focused;
			if (window == nullptr) window = nativeWindow;
			
			WORD keyFlags = HIWORD(lParam);
			BOOL isKeyReleased = (keyFlags & KF_UP) == KF_UP;
			if (isKeyReleased)
			{
				window->Renderer.KeyReleased(argKeyboard);
				window->Events->KeyReleased.Emit(argKeyboard);
			}
			else
			{
				window->Renderer.KeyPressed(argKeyboard);
				window->Events->KeyPressed.Emit(argKeyboard);
			}
			defaultToWindowProc = false;
			break;
		}
		case WM_ENTERSIZEMOVE:
		{
			ArgSizeMove argSizeMove;
			auto events = dynamic_cast<RootEvents*>(nativeWindow->Events.get());
			events->EnterSizeMove.Emit(argSizeMove);
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			ArgSizeMove argSizeMove;
			auto events = dynamic_cast<RootEvents*>(nativeWindow->Events.get());
			events->ExitSizeMove.Emit(argSizeMove);
			break;
		}
		case WM_DESTROY:
			::PostQuitMessage(0);
			//if (windowManager.Exists(nativeWindow))
			//{
			//	windowManager.Dispose(nativeWindow);
			//}
			defaultToWindowProc = false;
			break;
		}

		windowManager.UpdateDeferredRequests(nativeWindow);

		if (defaultToWindowProc)
		{
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}
}
#endif