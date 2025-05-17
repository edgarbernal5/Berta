/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Berta/Core/Foundation.h"

#ifdef BT_PLATFORM_WINDOWS

#include "Berta/Core/Base.h"
#include "Berta/Core/Log.h"
#include "Berta/GUI/Window.h"
#include "Berta/GUI/ControlEvents.h"
#include "Berta/GUI/EnumTypes.h"
#include "Berta/Platform/Windows/Messages.h"

#include "Berta/Controls/Menu.h"
#include "Berta/Controls/MenuBar.h"
#include "Berta/Paint/DrawBatch.h"

#if BT_DEBUG
#ifndef BT_PRINT_WND_MESSAGES
#define BT_PRINT_WND_MESSAGES
#endif // !BT_PRINT_WND_MESSAGES
#endif

//TODO: flickering!! https://stackoverflow.com/questions/50898990/reduce-flickering-when-using-setwindowpos-to-change-the-left-edge-of-a-window
//Google search: SWP_NOCOPYBITS
namespace Berta
{
	LRESULT CALLBACK Foundation_WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);
	bool ProcessMessage(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam, LRESULT& result);

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
		
		//Don't use either CS_HREDRAW or CS_VREDRAW flags. Could cause flicking when window is resized.
		WNDCLASSEXW wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = CS_OWNDC | CS_DBLCLKS; // Enable double-click messages
		wcex.lpfnWndProc = Foundation_WndProc;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIconW(hInstance, L"IDI_ICON");
		wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wcex.hbrBackground = NULL;
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
			if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}

#ifdef BT_PRINT_WND_MESSAGES
	uint32_t g_debugLastMessageId{};
	uint32_t g_debugLastMessageCount{0};

	//Short list.
	std::map<uint32_t, std::string> g_debugWndMessages
	{
		//{WM_MOVE,			"WM_MOVE"},
		//{WM_MOVING,			"WM_MOVING"},
		{WM_SIZE,			"WM_SIZE"},
		{WM_SIZING,			"WM_SIZING"},

		{WM_SHOWWINDOW,		"WM_SHOWWINDOW"},
		{WM_PAINT,			"WM_PAINT"},
		{WM_DPICHANGED,		"WM_DPICHANGED"},

		{WM_LBUTTONDOWN,	"WM_LBUTTONDOWN"},
		{WM_MBUTTONDOWN,	"WM_MBUTTONDOWN"},
		{WM_RBUTTONDOWN,	"WM_RBUTTONDOWN"},

		{WM_LBUTTONUP,		"WM_LBUTTONUP"},
		{WM_MBUTTONUP,		"WM_MBUTTONUP"},
		{WM_RBUTTONUP,		"WM_RBUTTONUP"},
		//{WM_MOUSEMOVE,		"WM_MOUSEMOVE"},

		//{WM_MOUSELEAVE,		"WM_MOUSELEAVE"},
		//{WM_ERASEBKGND,		"WM_ERASEBKGND"},
		//{WM_WINDOWPOSCHANGED,		"WM_WINDOWPOSCHANGED"},
		//{WM_WINDOWPOSCHANGING,		"WM_WINDOWPOSCHANGING"},

		//{ WM_NCACTIVATE, "WM_NCACTIVATE" },
		//{ WM_GETMINMAXINFO, "WM_GETMINMAXINFO" },
	};

	//Long list.
	//std::map<uint32_t, std::string> g_debugWndMessages
	//{
	//	{WM_CREATE,			"WM_CREATE"},
	//	{WM_NCCREATE,		"WM_NCCREATE"},
	//	{WM_MOVE,			"WM_MOVE"},
	//	//{WM_MOVING,			"WM_MOVING"},
	//	{WM_SIZE,			"WM_SIZE"},
	//	{WM_SIZING,			"WM_SIZING"},
	//	//{WM_ENTERSIZEMOVE,	"WM_ENTERSIZEMOVE"},
	//	//{WM_EXITSIZEMOVE,	"WM_EXITSIZEMOVE"},
	//	//{WM_ERASEBKGND,	"WM_ERASEBKGND"},

	//	{WM_DESTROY,		"WM_DESTROY"},
	//	{WM_NCDESTROY,		"WM_NCDESTROY"},
	//	{WM_SETFOCUS,		"WM_SETFOCUS"},
	//	{WM_KILLFOCUS,		"WM_KILLFOCUS"},
	//	{WM_CLOSE,			"WM_CLOSE"},

	//	{WM_CHAR,			"WM_CHAR"},
	//	{WM_KEYDOWN,		"WM_KEYDOWN"},
	//	{WM_KEYUP,			"WM_KEYUP"},
	//	{WM_SYSKEYDOWN,		"WM_SYSKEYDOWN"},
	//	{WM_SYSKEYUP,		"WM_SYSKEYUP"},

	//	{WM_SHOWWINDOW,		"WM_SHOWWINDOW"},
	//	{WM_ACTIVATEAPP,	"WM_ACTIVATEAPP"},
	//	{WM_PAINT,			"WM_PAINT"},
	//	{WM_DPICHANGED,		"WM_DPICHANGED"},
	//	{WM_NCCALCSIZE,		"WM_NCCALCSIZE"},
	//	{WM_NCPAINT,		"WM_NCPAINT"},
	//	//{WM_SETCURSOR,		"WM_SETCURSOR"},

	//	{WM_ACTIVATE,		"WM_ACTIVATE"},
	//	{WM_CAPTURECHANGED,	"WM_CAPTURECHANGED"},

	//	{WM_LBUTTONDBLCLK,	"WM_LBUTTONDBLCLK"},

	//	{WM_MOUSEACTIVATE,	"WM_MOUSEACTIVATE"},

	//	{WM_MOUSELEAVE,		"WM_MOUSELEAVE"},
	//	{WM_LBUTTONDOWN,	"WM_LBUTTONDOWN"},
	//	{WM_MBUTTONDOWN,	"WM_MBUTTONDOWN"},
	//	{WM_RBUTTONDOWN,	"WM_RBUTTONDOWN"},

	//	{WM_LBUTTONUP,		"WM_LBUTTONUP"},
	//	{WM_MBUTTONUP,		"WM_MBUTTONUP"},
	//	{WM_RBUTTONUP,		"WM_RBUTTONUP"},
	//	//{WM_MOUSEMOVE,		"WM_MOUSEMOVE"},
	//	{WM_MOUSEHWHEEL,	"WM_MOUSEHWHEEL"},
	//	{WM_MOUSEWHEEL,		"WM_MOUSEWHEEL"},
	//	{ WM_NCACTIVATE, "WM_NCACTIVATE" },
	//};
#endif

	LRESULT CALLBACK Foundation_WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
	{
#ifdef BT_PRINT_WND_MESSAGES
		bool printedMessage = false;
		std::ostringstream debugBuilder;
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
			//if (g_debugLastMessageCount == 1)
			{
				printedMessage = true;
				debugBuilder << ">> WndProc message: " << it->second << ". hWnd = " << hWnd;// << std::endl;
			}
			//if (g_debugLastMessageCount > 0)
			//	g_debugLastMessageCount = 0;

			//debugBuilder << "WndProc message: " << it->second << ". hWnd = " << hWnd << std::endl;
			g_debugLastMessageId = message;
		}
		else {
			//debugBuilder << "WndProc message: UNKNOWN (" << message << ") .hWnd = " << hWnd << std::endl;
		}
#endif
		LRESULT innerResult;
		if (ProcessMessage(hWnd, message, wParam, lParam, innerResult))
		{
#ifdef BT_PRINT_WND_MESSAGES
			if (printedMessage)
				BT_CORE_DEBUG << debugBuilder.str() << " <<" << std::endl;
#endif
			return innerResult;
		}
		auto& foundation = Foundation::GetInstance();

		API::NativeWindowHandle nativeWindowHandle{ hWnd };
		auto& windowManager = foundation.GetWindowManager();
		auto nativeWindow = windowManager.Get(nativeWindowHandle);
		if (nativeWindow == nullptr)
		{
#ifdef BT_PRINT_WND_MESSAGES
			if (printedMessage)
				BT_CORE_DEBUG << debugBuilder.str() << " <<" << std::endl;
#endif
			//debugBuilder << " *** native is null (" << message << ") .hWnd = " << hWnd << std::endl;
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

#ifdef BT_PRINT_WND_MESSAGES
		if (printedMessage)
		{
#if BT_DEBUG
			debugBuilder << ". window = " << nativeWindow->Name;
#endif
			BT_CORE_DEBUG << debugBuilder.str() << std::endl;
		}
#endif

		bool defaultToWindowProc = true;
		auto& rootWindowData = *windowManager.GetFormData(nativeWindowHandle);
		auto& trackEvent = rootWindowData.TrackEvent;

		DrawBatch drawBatch(nativeWindow);
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
		case WM_ERASEBKGND:
		{
			return TRUE;
		}

		//WM_NCPAINT, WM_NCCALCSIZE
		//https://github.com/rossy/borderless-window/blob/master/borderless-window.c#L347
		//https://devblog.cyotek.com/post/painting-the-borders-of-a-custom-control-using-wm-ncpaint

		case WM_ACTIVATEAPP:
		{
			ArgActivated argActivated{};
			argActivated.IsActivated = wParam ? true : false;
			//BT_CORE_TRACE << "    IsActivated = " << argActivated.IsActivated << ". " << hWnd << std::endl;
			auto events = dynamic_cast<FormEvents*>(nativeWindow->Events.get());
			events->Activated.Emit(argActivated);
			break;
		}

		case WM_NCACTIVATE:
			return ::DefWindowProc(hWnd, message, wParam, -1);	//DefWindowProc won't repaint the window border if lParam (normally a HRGN) is - 1.
																//https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-ncactivate
		case WM_GETMINMAXINFO:
		{
			::MINMAXINFO* pMinMax = (::MINMAXINFO*)lParam;
			bool changed = false;
			if (!nativeWindow->MinSize.IsEmpty())
			{
				pMinMax->ptMinTrackSize.x = nativeWindow->MinSize.Width;
				pMinMax->ptMinTrackSize.y = nativeWindow->MinSize.Height;
				changed = true;
			}

			if (!nativeWindow->MaxSize.IsEmpty())
			{
				pMinMax->ptMaxTrackSize.x = nativeWindow->MaxSize.Width;
				pMinMax->ptMaxTrackSize.y = nativeWindow->MaxSize.Height;
				changed = true;
			}

			if (changed)
			{
				return 0;
			}
			defaultToWindowProc = true;
			break;
		}
		case WM_SHOWWINDOW:
		{
			bool isVisible = (wParam == TRUE);
			if (nativeWindow->Visible != isVisible)
			{
				nativeWindow->Visible = isVisible;

				ArgVisibility argVisibility;
				argVisibility.IsVisible = isVisible;
				nativeWindow->Events->Visibility.Emit(argVisibility);

				auto targetWindow = isVisible ? nativeWindow : nativeWindow->FindFirstNonPanelAncestor();
				if (targetWindow)
				{
					windowManager.UpdateTree(targetWindow);
				}
			}
			break;
		}
		case WM_PAINT:
		{
			::PAINTSTRUCT ps;
			::BeginPaint(nativeWindow->RootHandle.Handle, &ps);

			Rectangle areaToUpdate;
			areaToUpdate.FromRECT(ps.rcPaint);
#if BT_DEBUG
			BT_CORE_DEBUG << " areaToUpdate = " << areaToUpdate << ". window = " << nativeWindow->Name << std::endl;
#else
			BT_CORE_DEBUG << " areaToUpdate = " << areaToUpdate << std::endl;
#endif
			nativeWindow->Renderer.Map(nativeWindow, areaToUpdate);  // Copy from control's graphics to native hwnd window.

			::EndPaint(nativeWindow->RootHandle.Handle, &ps);
			
			defaultToWindowProc = false;
			break;
		}
		//case WM_MOVING:
		case WM_MOVE:
		{
			int x = (int)(short)LOWORD(lParam);
			int y = (int)(short)HIWORD(lParam);
#if BT_DEBUG
			//BT_CORE_DEBUG << " move x = " << x << ", y = " << y << ". window = " << nativeWindow->Name << std::endl;
#else
//			BT_CORE_DEBUG << " move x = " << x << ", y = " << y << std::endl;
#endif
			ArgMove argMove;
			argMove.NewPosition.X = x;
			argMove.NewPosition.Y = y;
			foundation.ProcessEvents(nativeWindow, &Renderer::Move, &ControlEvents::Move, argMove);
			
			defaultToWindowProc = false;
			break;
		}
		//case WM_SIZING:
		//{
		//	::RECT* rect = reinterpret_cast<RECT*>(lParam);
		//	uint32_t newWidth = static_cast<uint32_t>(rect->right - rect->left) - nativeWindow->BorderSize.Width;
		//	uint32_t newHeight = static_cast<uint32_t>(rect->bottom - rect->top) - nativeWindow->BorderSize.Height;
		//	
		//	Size newSize{ newWidth , newHeight };

		//	defaultToWindowProc = false;
		//	break;
		//}
		case WM_SIZE:
		{
			uint32_t newWidth = (uint32_t)LOWORD(lParam);
			uint32_t newHeight = (uint32_t)HIWORD(lParam);

			Size newSize{ newWidth , newHeight };
#if BT_DEBUG
			BT_CORE_DEBUG << "   Size: new size " << newSize << ". window = " << nativeWindow->Name << std::endl;
#else
			BT_CORE_DEBUG << "   Size: new size " << newSize << std::endl;
#endif

			if (newWidth > 0 && newHeight > 0)
			{
				windowManager.Resize(nativeWindow, newSize, false);
				windowManager.UpdateTree(nativeWindow);
			}
			defaultToWindowProc = false;
			break;
		}
		//case WM_WINDOWPOSCHANGING:
		//case WM_WINDOWPOSCHANGED:
		case WM_DPICHANGED:
		{
			uint32_t newDPI = (uint32_t)HIWORD(wParam);
			windowManager.ChangeDPI(nativeWindow, newDPI, nativeWindow->RootHandle);

			auto rect = reinterpret_cast<const RECT*>(lParam);

			::SetWindowPos(hWnd,
				NULL,
				rect->left,
				rect->top,
				rect->right - rect->left,
				rect->bottom - rect->top,
				SWP_NOZORDER | SWP_NOACTIVATE);

			//This is called inside Resize method of WindowManager.
			windowManager.UpdateTree(nativeWindow);

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
			break;
		}
		case WM_KILLFOCUS:
		{
			if (rootWindowData.Focused)
			{
				ArgFocus argFocus{ false };
				foundation.ProcessEvents(rootWindowData.Focused, &Renderer::Focus, &ControlEvents::Focus, argFocus);
			}
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
			defaultToWindowProc = false;
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			auto window = windowManager.Find(nativeWindow, { x, y });
			if (window && window->Flags.IsEnabled)
			{
				rootWindowData.Pressed = window;

				ArgMouse argMouseDown;
				argMouseDown.Position = Point{ x, y } - windowManager.GetAbsoluteRootPosition(window);
				argMouseDown.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
				argMouseDown.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
				argMouseDown.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

				foundation.ProcessEvents(window, &Renderer::MouseDown, &ControlEvents::MouseDown, argMouseDown);
				
				auto focusWindow = window->Flags.MakeActive ? window : window->MakeTargetWhenInactive;
				if (focusWindow && !focusWindow->Flags.IgnoreMouseFocus)
				{
					if (rootWindowData.Focused != focusWindow)
					{
						if (rootWindowData.Focused)
						{
							ArgFocus argFocus{ false };
							foundation.ProcessEvents(rootWindowData.Focused, &Renderer::Focus, &ControlEvents::Focus, argFocus);
						}
						if (focusWindow)
						{
							ArgFocus argFocus{ true };
							foundation.ProcessEvents(focusWindow, &Renderer::Focus, &ControlEvents::Focus, argFocus);
						}
					}
					rootWindowData.Focused = focusWindow;
				}
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

					POINT screenToClientPoint{};
					screenToClientPoint.x = x;
					screenToClientPoint.y = y;
					::ClientToScreen(hWnd, &screenToClientPoint);

					::ScreenToClient(currentWindow->RootHandle.Handle, &screenToClientPoint);

					auto localPosition = Point{ (int)screenToClientPoint.x, (int)screenToClientPoint.y } - windowManager.GetAbsoluteRootPosition(currentWindow);
					if (currentWindow->ClientSize.IsInside(localPosition))
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
					argMouseLeave.Position = Point{ x, y } - windowManager.GetAbsoluteRootPosition(rootWindowData.Hovered);
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
					if (rootWindowData.Hovered && windowManager.Exists(rootWindowData.Hovered))
					{
						ArgMouse argMouseLeave;
						argMouseLeave.Position = Point{ x, y } - windowManager.GetAbsoluteRootPosition(rootWindowData.Hovered);
						argMouseLeave.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
						argMouseLeave.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
						argMouseLeave.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

						//BT_CORE_DEBUG << " - mouse leave / name " << rootWindowData.Hovered->Name << ". hovered " << rootWindowData.Hovered << std::endl;
						foundation.ProcessEvents(rootWindowData.Hovered, &Renderer::MouseLeave, &ControlEvents::MouseLeave, argMouseLeave);
					}
					rootWindowData.Hovered = nullptr;
				}

				if (window && window->Flags.IsEnabled && !window->Flags.IsDisposed)
				{
					Point position = Point{ x, y } - windowManager.GetAbsoluteRootPosition(window);
					if (window != rootWindowData.Hovered && window->ClientSize.IsInside(position))
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
					if (!rootWindowData.IsTracking && window->ClientSize.IsInside(position))
					{
						//BT_CORE_DEBUG << " - keep track / name " << window->Name << ". hWnd " << hWnd << std::endl;
						trackEvent.hwndTrack = hWnd;
						::TrackMouseEvent(&trackEvent); //Keep track of mouse position to Emit WM_MOUSELEAVE message.
						rootWindowData.IsTracking = true;
					}
				}
			}
			defaultToWindowProc = false;
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
				argMouseUp.Position = Point{ x, y } - windowManager.GetAbsoluteRootPosition(window);
				argMouseUp.ButtonState.LeftButton = message == WM_LBUTTONUP;
				argMouseUp.ButtonState.RightButton = message == WM_RBUTTONUP;
				argMouseUp.ButtonState.MiddleButton = message == WM_MBUTTONUP;

				if (window->ClientSize.IsInside(argMouseUp.Position))
				{
					ArgClick argClick;
					foundation.ProcessEvents(window, &Renderer::Click, &ControlEvents::Click, argClick);
				}

				foundation.ProcessEvents(window, &Renderer::MouseUp, &ControlEvents::MouseUp, argMouseUp);

				rootWindowData.Released = rootWindowData.Pressed;
			}
			rootWindowData.Pressed = nullptr;
			defaultToWindowProc = false;

			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			auto window = windowManager.Find(nativeWindow, { x, y });
			if (window && window->Flags.IsEnabled && window == rootWindowData.Released)
			{
				ArgMouse argMouse{};
				argMouse.Position = Point{ x, y } - windowManager.GetAbsoluteRootPosition(window);
				argMouse.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
				argMouse.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
				argMouse.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

				foundation.ProcessEvents(window, &Renderer::DblClick, &ControlEvents::DblClick, argMouse);
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
			POINT screenToClientPoint{};
			screenToClientPoint.x = x;
			screenToClientPoint.y = y;
			::ScreenToClient(hWnd, &screenToClientPoint);

			auto window = windowManager.Find(nativeWindow, { static_cast<int>(screenToClientPoint.x), static_cast<int>(screenToClientPoint.y) });
			if (window)
			{
				ArgWheel argWheel{};
				argWheel.WheelDelta = wheelDelta;
				argWheel.IsVertical = message == WM_MOUSEWHEEL;

				foundation.ProcessEvents(window, &Renderer::MouseWheel, &ControlEvents::MouseWheel, argWheel);
			}
			break;
		}
		case WM_CHAR:
		{
			ArgKeyboard argKeyboard{};
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
			ArgKeyboard argKeyboard{};
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
			else
			{
				defaultToWindowProc = false;
			}
			break;
		}
		}

#ifdef BT_PRINT_WND_MESSAGES
		if (it != g_debugWndMessages.end())
		{
			BT_CORE_DEBUG << "<< WndProc message: " << it->second << ". hWnd = " << hWnd << ". window = " << nativeWindow->Name << std::endl;
		}
#endif

		if (defaultToWindowProc)
		{
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}

	bool ProcessMessage(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam, LRESULT& result)
	{
		result = 0;

		switch (message)
		{
		case static_cast<uint32_t>(CustomMessageId::CustomCallback):

		case WM_ERASEBKGND:
		case WM_ACTIVATEAPP:
		//case WM_ACTIVATE:
		case WM_NCACTIVATE:
		case WM_GETMINMAXINFO:
		case WM_SHOWWINDOW:
		case WM_PAINT:
		//case WM_MOVING:
		case WM_MOVE:
		//case WM_SIZING:
		case WM_SIZE:
		//case WM_WINDOWPOSCHANGING:
		//case WM_WINDOWPOSCHANGED:
		case WM_DPICHANGED:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		case WM_MOUSEACTIVATE:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MOUSELEAVE:
		case WM_MOUSEHWHEEL:
		case WM_MOUSEWHEEL:
		case WM_CHAR:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_ENTERSIZEMOVE:
		case WM_EXITSIZEMOVE:
		case WM_CLOSE:
		case WM_DESTROY:
		case WM_NCDESTROY:
			return false;
		}

		result = ::DefWindowProc(hWnd, message, wParam, lParam);
		return true;
	}
}

/*
using Clock = std::chrono::steady_clock;
auto lastPaintTime = Clock::now();
const int frameDelayMs = 1000 / 60; // ~16ms for 60 FPS

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		return TRUE;

	case WM_SIZING:
	{
		// Optional: force a redraw but only if enough time has passed
		auto now = Clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPaintTime).count();

		if (ms >= frameDelayMs) {
			InvalidateRect(hwnd, nullptr, FALSE);  // Mark the whole window dirty
			lastPaintTime = now;
		}

		return TRUE;
	}

*/
#endif