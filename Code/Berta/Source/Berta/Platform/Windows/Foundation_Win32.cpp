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
#include "Berta/GUI/ControlEvents.h"
#include "Berta/GUI/EnumTypes.h"
#include "Berta/Platform/Windows/Messages.h"

#include "Berta/Controls/Menu.h"
#include "Berta/Controls/MenuBar.h"

#if BT_DEBUG
#ifndef BT_PRINT_WND_MESSAGES
#define BT_PRINT_WND_MESSAGES
#endif // !BT_PRINT_WND_MESSAGES
#endif

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

		//TODO: proper way of implementing dpi awareness.
		//JustCtrl_Init(): pGetDpiForSystem, pGetDpiForWindow...
		//https://github.com/sullewarehouse/JustCtrl/blob/main/source/JustCtrl.cpp#L35

		//https://github.com/b-sullender/WinGui/blob/main/WinGui.cpp#L363
		::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		HINSTANCE hInstance = GetModuleInstance();
		
		//Don't use either CS_HREDRAW or CS_VREDRAW flags. Could case flicking when window is resized.
		WNDCLASSEXW wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = CS_OWNDC | CS_DBLCLKS; // Enable double-click messages
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

#ifdef BT_PRINT_WND_MESSAGES
	uint32_t g_debugLastMessageId{};
	uint32_t g_debugLastMessageCount{0};
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
		{WM_CLOSE,			"WM_CLOSE"},

		{WM_CHAR,			"WM_CHAR"},
		{WM_KEYDOWN,		"WM_KEYDOWN"},
		{WM_KEYUP,			"WM_KEYUP"},
		{WM_SYSKEYDOWN,		"WM_SYSKEYDOWN"},
		{WM_SYSKEYUP,		"WM_SYSKEYUP"},

		{WM_SHOWWINDOW,		"WM_SHOWWINDOW"},
		{WM_ACTIVATEAPP,	"WM_ACTIVATEAPP"},
		{WM_PAINT,			"WM_PAINT"},
		{WM_DPICHANGED,		"WM_DPICHANGED"},
		//{WM_SETCURSOR,		"WM_SETCURSOR"},

		{WM_ACTIVATE,		"WM_ACTIVATE"},
		{WM_CAPTURECHANGED,	"WM_CAPTURECHANGED"},

		{WM_LBUTTONDBLCLK,	"WM_LBUTTONDBLCLK"},

		{WM_MOUSEACTIVATE,	"WM_MOUSEACTIVATE"},

		{WM_MOUSELEAVE,		"WM_MOUSELEAVE"},
		{WM_LBUTTONDOWN,	"WM_LBUTTONDOWN"},
		{WM_MBUTTONDOWN,	"WM_MBUTTONDOWN"},
		{WM_RBUTTONDOWN,	"WM_RBUTTONDOWN"},

		{WM_LBUTTONUP,		"WM_LBUTTONUP"},
		{WM_MBUTTONUP,		"WM_MBUTTONUP"},
		{WM_RBUTTONUP,		"WM_RBUTTONUP"},
		//{WM_MOUSEMOVE,		"WM_MOUSEMOVE"},
		{WM_MOUSEHWHEEL,	"WM_MOUSEHWHEEL"},
		{WM_MOUSEWHEEL,		"WM_MOUSEWHEEL"},

	};
#endif

	LRESULT CALLBACK Foundation_WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
	{
#ifdef BT_PRINT_WND_MESSAGES
		auto it = g_debugWndMessages.find(message);
		if (it != g_debugWndMessages.end())
		{
			if (g_debugLastMessageId != message)
			{
				g_debugLastMessageCount = 1;
			}
			else
			{
				++g_debugLastMessageCount;
			}
			if (g_debugLastMessageCount == 1)
			{
				BT_CORE_DEBUG << "WndProc message: " << it->second << ". hWnd = " << hWnd << std::endl;
			}
			if (g_debugLastMessageCount > 0)
				g_debugLastMessageCount = 0;

			//BT_CORE_DEBUG << "WndProc message: " << it->second << ". hWnd = " << hWnd << std::endl;
			g_debugLastMessageId = message;
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

		bool defaultToWindowProc = true;
		auto& rootWindowData = *windowManager.GetFormData(nativeWindowHandle);
		auto& trackEvent = rootWindowData.TrackEvent;

		Berta::Foundation::RootGuard rootGuard(nativeWindow);

		switch (message)
		{
		case static_cast<uint32_t>(CustomMessageId::CustomCallback):
		{
			if (wParam)
			{
				auto argParam = reinterpret_cast<CustomCallbackMessage*>(wParam);
				if (argParam->Body)
				{
					argParam->Body();
				}

				//TODO: improve memory management here.
				delete argParam;
			}
			break;
		}
		case WM_ACTIVATEAPP:
		{
			ArgActivated argActivated;
			argActivated.IsActivated = wParam ? true : false;
			//BT_CORE_TRACE << "    IsActivated = " << argActivated.IsActivated << ". " << hWnd << std::endl;
			auto events = dynamic_cast<FormEvents*>(nativeWindow->Events.get());
			events->Activated.Emit(argActivated);
			break;
		}
		case WM_SHOWWINDOW:
		{
			bool isVisible = wParam == TRUE;
			nativeWindow->Visible = isVisible;

			ArgVisibility argVisibility;
			argVisibility.IsVisible = isVisible;
			nativeWindow->Events->Visibility.Emit(argVisibility);

			windowManager.UpdateTree(nativeWindow);
			//nativeWindow->Renderer.Map(nativeWindow, nativeWindow->Size.ToRectangle());
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			::BeginPaint(nativeWindow->RootHandle.Handle, &ps);

			Rectangle areaToUpdate;
			areaToUpdate.FromRECT(ps.rcPaint);
			BT_CORE_DEBUG << "   areaToUpdate = { x=" << areaToUpdate.X << "; y=" << areaToUpdate.Y << "; w=" << areaToUpdate.Width << "; h=" << areaToUpdate.Height << "}" << std::endl;
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
				ArgResize argResize;
				argResize.NewSize.Width = newWidth;
				argResize.NewSize.Height = newHeight;
				//BT_CORE_DEBUG << "   Size: new size " << argResize.NewSize << std::endl;

				//TODO: esto es un hack!
				windowManager.Resize(nativeWindow, argResize.NewSize);
				windowManager.UpdateTree(nativeWindow);
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

			//This is called inside Resize method of WindowManager.
			//windowManager.UpdateTree(nativeWindow);
			defaultToWindowProc = false;
			break;
		}
		case WM_SETFOCUS:
		{
			if (rootWindowData.Focused)
			{
				ArgFocus argFocus{ true };
				foundation.ProcessEvents(rootWindowData.Focused, &Renderer::Focus, &ControlEvents::Focus, argFocus);
			}
			defaultToWindowProc = false;
			break;
		}
		case WM_KILLFOCUS:
		{
			if (rootWindowData.Focused)
			{
				ArgFocus argFocus{ false };
				foundation.ProcessEvents(rootWindowData.Focused, &Renderer::Focus, &ControlEvents::Focus, argFocus);
			}
			defaultToWindowProc = false;
			break;
		}
		case WM_MOUSEACTIVATE: //This is not sent while mouse is captured
		{
			if (!nativeWindow->Flags.MakeActive)
			{
				return MA_NOACTIVATE;
			}

			break;
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			auto window = windowManager.Find(nativeWindow, { x, y });
			if (window && window->Flags.IsEnabled)
			{
				rootWindowData.Pressed = window;

				ArgMouse argMouseDown;
				argMouseDown.Position = Point{ x, y } - windowManager.GetAbsolutePosition(window);
				argMouseDown.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
				argMouseDown.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
				argMouseDown.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

				foundation.ProcessEvents(window, &Renderer::MouseDown, &ControlEvents::MouseDown, argMouseDown);
				
				if (rootWindowData.Focused != window)
				{
					if (rootWindowData.Focused)
					{
						ArgFocus argFocus{ false };
						foundation.ProcessEvents(rootWindowData.Focused, &Renderer::Focus, &ControlEvents::Focus, argFocus);
					}
					if (window)
					{
						ArgFocus argFocus{ true };
						foundation.ProcessEvents(window, &Renderer::Focus, &ControlEvents::Focus, argFocus);
					}
				}
				rootWindowData.Focused = window;
			}
			
			break;
		}
		case WM_MOUSEMOVE:
		{
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			auto menuItemReactor = windowManager.GetMenu();
			if (menuItemReactor)
			{
				//TODO: mover esta logica a MenuManager (?)
				auto currentItemReactor = menuItemReactor;
				do
				{
					auto currentWindow = currentItemReactor->Owner();

					POINT screenToClientPoint;
					screenToClientPoint.x = x;
					screenToClientPoint.y = y;
					::ClientToScreen(hWnd, &screenToClientPoint);

					::ScreenToClient(currentWindow->RootHandle.Handle, &screenToClientPoint);

					auto localPosition = Point{ (int)screenToClientPoint.x, (int)screenToClientPoint.y } - windowManager.GetAbsolutePosition(currentWindow);
					if (currentWindow->Size.IsInside(localPosition))
					{
						if (rootWindowData.Hovered == nullptr)
						{
							ArgMouse argMouseEnter;
							argMouseEnter.Position = localPosition;
							argMouseEnter.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
							argMouseEnter.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
							argMouseEnter.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

							//BT_CORE_DEBUG << " - mouse enter / name " << window->Name << ".hovered " << rootWindowData.Hovered << std::endl;
							foundation.ProcessEvents(currentWindow, &Renderer::MouseEnter, &ControlEvents::MouseEnter, argMouseEnter);
						}

						ArgMouse argMouseMove;
						argMouseMove.Position = localPosition;
						argMouseMove.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
						argMouseMove.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
						argMouseMove.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

						//BT_CORE_DEBUG << " - MENU / mouse move name " << currentWindow->Name << ". hovered " << rootWindowData.Hovered << std::endl;
						foundation.ProcessEvents(currentWindow, &Renderer::MouseMove, &ControlEvents::MouseMove, argMouseMove);

						rootWindowData.Hovered = currentWindow;
						break;
					}
					
					currentItemReactor = currentItemReactor->Next();
				} while (currentItemReactor);

				if (currentItemReactor == nullptr && rootWindowData.Hovered)
				{
					ArgMouse argMouseLeave;
					argMouseLeave.Position = Point{ x, y } - windowManager.GetAbsolutePosition(rootWindowData.Hovered);
					argMouseLeave.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
					argMouseLeave.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
					argMouseLeave.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

					//BT_CORE_DEBUG << " - MENU / mouse leave / name " << rootWindowData.Hovered->Name << ". hovered " << rootWindowData.Hovered << std::endl;
					foundation.ProcessEvents(rootWindowData.Hovered, &Renderer::MouseLeave, &ControlEvents::MouseLeave, argMouseLeave);

					rootWindowData.Hovered = nullptr;
				}
			}
			else
			{
				auto window = windowManager.Find(nativeWindow, { x, y });
				//BT_CORE_DEBUG << " - window and hovered / window " << (window != nullptr ? window->Name :"NULL") << ". hovered " << (rootWindowData.Hovered != nullptr ? rootWindowData.Hovered->Name : "NULL") << std::endl;
				if (window && window != rootWindowData.Hovered)
				{
					if (rootWindowData.Hovered)
					{
						ArgMouse argMouseLeave;
						argMouseLeave.Position = Point{ x, y } - windowManager.GetAbsolutePosition(rootWindowData.Hovered);
						argMouseLeave.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
						argMouseLeave.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
						argMouseLeave.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

						//BT_CORE_DEBUG << " - mouse leave / name " << rootWindowData.Hovered->Name << ". hovered " << rootWindowData.Hovered << std::endl;
						foundation.ProcessEvents(rootWindowData.Hovered, &Renderer::MouseLeave, &ControlEvents::MouseLeave, argMouseLeave);

						rootWindowData.Hovered = nullptr;
					}
				}

				if (window && window->Flags.IsEnabled && !window->Flags.IsDisposed)
				{
					Point position = Point{ x, y } - windowManager.GetAbsolutePosition(window);
					if (window != rootWindowData.Hovered && window->Size.IsInside(position))
					{
						ArgMouse argMouseEnter;
						argMouseEnter.Position = position;
						argMouseEnter.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
						argMouseEnter.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
						argMouseEnter.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

						//BT_CORE_DEBUG << " - mouse enter / name " << window->Name << ".hovered " << rootWindowData.Hovered << std::endl;
						foundation.ProcessEvents(window, &Renderer::MouseEnter, &ControlEvents::MouseEnter, argMouseEnter);

						rootWindowData.Hovered = window;
					}

					if (rootWindowData.Hovered)
					{
						ArgMouse argMouseMove;
						argMouseMove.Position = position;
						argMouseMove.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
						argMouseMove.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
						argMouseMove.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

						//BT_CORE_DEBUG << " - window. MouseMove " << window->Name << std::endl;
						foundation.ProcessEvents(window, &Renderer::MouseMove, &ControlEvents::MouseMove, argMouseMove);
					}
					if (!rootWindowData.IsTracking && window->Size.IsInside(position))
					{
						//BT_CORE_DEBUG << " - keep track / name " << window->Name << ". hWnd " << hWnd << std::endl;
						trackEvent.hwndTrack = hWnd;
						::TrackMouseEvent(&trackEvent); //Keep track of mouse position to Emit WM_MOUSELEAVE message.
						rootWindowData.IsTracking = true;
					}
				}
			}
			break;
		}
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			auto window = windowManager.Find(nativeWindow, { x, y });
			if (window && window->Flags.IsEnabled)
			{
				ArgMouse argMouseUp;
				argMouseUp.Position = Point{ x, y } - windowManager.GetAbsolutePosition(window);
				argMouseUp.ButtonState.LeftButton = message == WM_LBUTTONUP;
				argMouseUp.ButtonState.RightButton = message == WM_RBUTTONUP;
				argMouseUp.ButtonState.MiddleButton = message == WM_MBUTTONUP;

				if (window->Size.IsInside(argMouseUp.Position))
				{
					ArgClick argClick;
					foundation.ProcessEvents(window, &Renderer::Click, &ControlEvents::Click, argClick);
				}

				foundation.ProcessEvents(window, &Renderer::MouseUp, &ControlEvents::MouseUp, argMouseUp);

				rootWindowData.Released = rootWindowData.Pressed;
			}
			rootWindowData.Pressed = nullptr;

			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			auto window = windowManager.Find(nativeWindow, { x, y });
			if (window && window->Flags.IsEnabled && window == rootWindowData.Released)
			{
				ArgClick argClick;
				argClick.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
				argClick.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
				argClick.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

				foundation.ProcessEvents(window, &Renderer::DblClick, &ControlEvents::DblClick, argClick);
			}
			rootWindowData.Released = nullptr;
			defaultToWindowProc = false;
			break;
		}
		case WM_MOUSELEAVE:
		{
			//BT_CORE_DEBUG << " - mouse leave 2 / preparing... " << std::endl;
			rootWindowData.IsTracking = false;
			if (rootWindowData.Hovered && windowManager.Exists(rootWindowData.Hovered))
			{
				//BT_CORE_DEBUG << " - mouse leave 2 / name " << rootWindowData.Hovered->Name << ". hovered " << rootWindowData.Hovered << ". hwnd " << hWnd << std::endl;
				ArgMouse argMouseLeave;
				foundation.ProcessEvents(rootWindowData.Hovered, &Renderer::MouseLeave, &ControlEvents::MouseLeave, argMouseLeave);

				rootWindowData.Hovered = nullptr;
			}
			defaultToWindowProc = false;
			break;
		}
		case WM_MOUSEHWHEEL:
		case WM_MOUSEWHEEL:
		{
			int wheelDelta = ((int)(short)HIWORD(wParam));
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));
			POINT screenToClientPoint;
			screenToClientPoint.x = x;
			screenToClientPoint.y = y;
			::ScreenToClient(hWnd, &screenToClientPoint);

			auto window = windowManager.Find(nativeWindow, { static_cast<int>(screenToClientPoint.x), static_cast<int>(screenToClientPoint.y) });
			if (window)
			{
				ArgWheel argWheel;
				argWheel.WheelDelta = wheelDelta;
				argWheel.IsVertical = message == WM_MOUSEWHEEL;

				foundation.ProcessEvents(window, &Renderer::MouseWheel, &ControlEvents::MouseWheel, argWheel);
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
			if (window == nullptr)
			{
				window = nativeWindow;
			}

			foundation.ProcessEvents(window, &Renderer::KeyChar, &ControlEvents::KeyChar, argKeyboard);

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
			if (window == nullptr)
			{
				window = nativeWindow;
			}
			
			WORD keyFlags = HIWORD(lParam);
			BOOL isKeyReleased = (keyFlags & KF_UP) == KF_UP;

			auto target = window;
			auto menuItemReactor = windowManager.GetMenu();
			if (menuItemReactor)
			{
				target = menuItemReactor->Owner();
			}
			if (isKeyReleased)
			{
				foundation.ProcessEvents(target, &Renderer::KeyReleased, &ControlEvents::KeyReleased, argKeyboard);
			}
			else
			{
				foundation.ProcessEvents(target, &Renderer::KeyPressed, &ControlEvents::KeyPressed, argKeyboard);
			}
			
			defaultToWindowProc = false;
			break;
		}
		case WM_ENTERSIZEMOVE:
		{
			ArgSizeMove argSizeMove;
			auto events = dynamic_cast<FormEvents*>(nativeWindow->Events.get());
			events->EnterSizeMove.Emit(argSizeMove);
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			ArgSizeMove argSizeMove;
			auto events = dynamic_cast<FormEvents*>(nativeWindow->Events.get());
			events->ExitSizeMove.Emit(argSizeMove);
			break;
		}
		case WM_CLOSE:
		{
			ArgDisposing argDisposing{ false };
			auto events = dynamic_cast<FormEvents*>(nativeWindow->Events.get());
			events->Disposing.Emit(argDisposing);
			if (argDisposing.Cancel)
			{
				defaultToWindowProc = false;
			}
			
			break;
		}
		case WM_DESTROY: // WM_DESTROY, next WM_NCDESTROY
		{
			windowManager.Destroy(nativeWindow);
			defaultToWindowProc = false;
			
			break;
		}
		case WM_NCDESTROY:
		{
			windowManager.Remove(nativeWindow);
			if (windowManager.NativeWindowCount() == 0)
			{
				::PostQuitMessage(0);
			}
			break;
		}
		}
		//TODO: no se si es mejor tener una lista global de peticiones en vez de por ventana nativa. Asi se estaria actualizando todo al mismo tiempo
		//(cuando se manipulan menu's es un caso especial, cada menu es una ventana nativa que es diferente a la ventana emisora.
		//Otra cosa que hay que mejorar es no repetir dos solicitudes para la misma ventana en el mismo "tick"
		//Otra mejora: solo actualizar el segmento (rectangulo) que necesita cambiar (por ejemplo al moverse dentro de un menu solo se deberia actualizar el rectangulo
		//del nuevo elemento seleccionado y no todo el menu
		windowManager.UpdateDeferredRequests(nativeWindow);

		if (defaultToWindowProc)
		{
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}
}
#endif