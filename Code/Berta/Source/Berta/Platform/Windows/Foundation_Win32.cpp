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
#include "Berta/Platform/Windows/Messages.h"

#include "Berta/Controls/Menu.h"
#include "Berta/Controls/MenuBar.h"
#include "Berta/Paint/DrawBatchActivator.h"

#if BT_DEBUG
#ifndef BT_PRINT_WND_MESSAGES
#define BT_PRINT_WND_MESSAGES
#endif // !BT_PRINT_WND_MESSAGES
#endif

//TODO: flickering!! https://stackoverflow.com/questions/50898990/reduce-flickering-when-using-setwindowpos-to-change-the-left-edge-of-a-window
//Google search: SWP_NOCOPYBITS
namespace Berta
{
	Foundation Foundation::g_foundation;

	static LRESULT CALLBACK Foundation_WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);
	bool IsDefaultMessage(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam, LRESULT& result);

	static HINSTANCE g_hModuleInstance;

	static HINSTANCE GetModuleInstance()
	{
		if (g_hModuleInstance == nullptr)
		{
			g_hModuleInstance = GetModuleHandle(nullptr);
		}

		return g_hModuleInstance;
	}

	Foundation::Foundation()
	{
		InitializeCore();
		BT_CORE_TRACE << "Foundation init..." << std::endl;

		//TODO: proper way of implementing dpi awareness.
		//JustCtrl_Init(): pGetDpiForSystem, pGetDpiForWindow...
		//https://github.com/sullewarehouse/JustCtrl/blob/main/source/JustCtrl.cpp#L35

		//https://github.com/b-sullender/WinGui/blob/main/WinGui.cpp#L363
		::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		HINSTANCE hInstance = GetModuleInstance();

		//Don't use either CS_HREDRAW or CS_VREDRAW flags. Could cause flicking when window is resized.
		{
			WNDCLASSEXW wcex = {};
			wcex.cbSize = sizeof(WNDCLASSEXW);
			wcex.style = /*CS_HREDRAW | CS_VREDRAW |*/ CS_OWNDC | CS_DBLCLKS; // Enable double-click messages
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
		{
			WNDCLASSEXW wcex = {};
			wcex.cbSize = sizeof(WNDCLASSEXW);
			wcex.style = /*CS_HREDRAW | CS_VREDRAW |*/ CS_OWNDC | CS_DBLCLKS; // Enable double-click messages
			wcex.lpfnWndProc = Foundation_WndProc;
			wcex.hInstance = hInstance;
			//wcex.hIcon = LoadIconW(hInstance, L"IDI_ICON");
			wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
			wcex.hbrBackground = NULL;
			wcex.lpszClassName = L"BertaNestedInternalClass";
			//wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");

			if (!RegisterClassExW(&wcex))
			{
				BT_CORE_ERROR << "RegisterClassExW Failed." << std::endl;
				return;
			}
		}
	}

	Foundation::~Foundation()
	{
		BT_CORE_TRACE << "Releasing foundation..." << std::endl;

		UnregisterClass(L"BertaInternalClass", g_hModuleInstance);
		UnregisterClass(L"BertaNestedInternalClass", g_hModuleInstance);
		g_hModuleInstance = nullptr;

		ShutdownCore();
	}

	void Foundation::ProcessMessages()
	{
		auto& windowManager = GetWindowManager();
		std::vector<API::NativeWindowHandle> allHandles;

		MSG msg = { 0 };
		while (msg.message != WM_QUIT)
		{
			if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			else
			{
				windowManager.GetNativeWindows(allHandles);
				for (auto& handle : allHandles)
				{
					auto window = windowManager.Get(handle);
					if (window->RenderForAttributes.AutoRefresh && window->HasCustomPaint())
					{
						API::RefreshWindow(window->RootHandle);
					}
				}
			}
		}
	}

#ifdef BT_PRINT_WND_MESSAGES
	std::map<HWND, uint32_t> g_debugLastMessageId{};
	std::map<HWND, uint32_t> g_debugLastMessageCount;

	//Short list.
	std::map<uint32_t, std::string> g_debugWndMessages
	{
		//{WM_MOVE,			"WM_MOVE"},
		//{WM_MOVING,			"WM_MOVING"},
		//{WM_SIZE,			"WM_SIZE"},
		//{WM_SIZING,			"WM_SIZING"},

		{WM_SHOWWINDOW,		"WM_SHOWWINDOW"},
		//{WM_PAINT,			"WM_PAINT"},
		//{WM_DPICHANGED,		"WM_DPICHANGED"},

		{WM_LBUTTONDOWN,	"WM_LBUTTONDOWN"},
		{WM_MBUTTONDOWN,	"WM_MBUTTONDOWN"},
		{WM_RBUTTONDOWN,	"WM_RBUTTONDOWN"},

		{WM_LBUTTONUP,		"WM_LBUTTONUP"},
		{WM_MBUTTONUP,		"WM_MBUTTONUP"},
		{WM_RBUTTONUP,		"WM_RBUTTONUP"},
		//{WM_MOUSEMOVE,		"WM_MOUSEMOVE"},

		{WM_SETFOCUS,		"WM_SETFOCUS"},
		{WM_KILLFOCUS,		"WM_KILLFOCUS"},

		{WM_MOUSELEAVE,		"WM_MOUSELEAVE"},
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
			if (g_debugLastMessageId[hWnd] != message)
			{
				g_debugLastMessageCount[hWnd] = 1;
			}
			else
			{
				++g_debugLastMessageCount[hWnd];
			}
			if (g_debugLastMessageCount[hWnd] == 1)
			{
				printedMessage = true;
				debugBuilder << ">> WndProc message: " << it->second << ". hWnd = " << hWnd;// << std::endl;
			}
			if (g_debugLastMessageCount[hWnd] > 0)
				g_debugLastMessageCount[hWnd] = 0;

			//debugBuilder << "WndProc message: " << it->second << ". hWnd = " << hWnd << std::endl;
			g_debugLastMessageId[hWnd] = message;
		}
		else {
			//printedMessage = true;
			//debugBuilder << "WndProc message: UNKNOWN (" << message << ") .hWnd = " << hWnd;
		}
#endif
		LRESULT innerResult;
		if (IsDefaultMessage(hWnd, message, wParam, lParam, innerResult))
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
				BT_CORE_DEBUG << "native is null. " << debugBuilder.str() << " <<" << std::endl;
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

		auto rootWindowData = windowManager.GetFormData(nativeWindowHandle);
		if (!rootWindowData)
		{
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		bool wasHandled = false;
		auto& menuManager = foundation.GetMenuManager();
		auto& trackEvent = rootWindowData->TrackEvent;
		auto rootPressedWindow = rootWindowData->Pressed;
		auto rootHoveredWindow = rootWindowData->Hovered;
		auto rootFocusedWindow = rootWindowData->Focused;
		auto rootReleasedWindow = rootWindowData->Released;

		DrawBatchActivator drawBatch(nativeWindow);
		Foundation::RootGuard rootGuard(nativeWindow);

		//ver lecui para manejar bien los mensajes.
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
			wasHandled = false;
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
			auto events = dynamic_cast<FormEvents*>(nativeWindow->Events.get());
			events->Activated.Emit(argActivated);

			wasHandled = false;
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
			wasHandled = false;
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
					API::RefreshWindow(nativeWindowHandle);
				}
			}
			wasHandled = false;
			break;
		}
		case WM_PAINT:
		{
			//std::cout << "  - PAINT. wnd=" << nativeWindow->Name << std::endl;
			if (nativeWindow->Type == WindowType::RenderForm)
			{
				::PAINTSTRUCT ps;
				auto hdc = ::BeginPaint(nativeWindow->RootHandle.Handle, &ps);

				HBRUSH hBrush = ::CreateSolidBrush(nativeWindow->Appearance->Background.ToBGR());
				::FillRect(hdc, &ps.rcPaint, hBrush);
				::DeleteObject(hBrush);

				Rectangle areaToUpdate;
				areaToUpdate.FromRECT(ps.rcPaint);
#if BT_DEBUG
				//BT_CORE_DEBUG << "   area to update " << areaToUpdate << ". window = " << nativeWindow->Name << std::endl;
				//BT_CORE_DEBUG << "   client size " << nativeWindow->ClientSize << ". window = " << nativeWindow->Name << std::endl;
#else
				//BT_CORE_DEBUG << "   area to update " << areaToUpdate << std::endl;
				//BT_CORE_DEBUG << "   client size " << nativeWindow->ClientSize << std::endl;
#endif
				if (nativeWindow->HasCustomPaint())
				{
					nativeWindow->RenderForAttributes.CustomPaint();
				}
				::EndPaint(hWnd, &ps);
				//::BeginPaint() already validated the update area.
			}
			else
			{
#if BT_DEBUG
				//ScopedTimer scopedTimer("WM_PAINT / window = " + nativeWindow->Name);
#else
				//ScopedTimer scopedTimer("WM_PAINT");
#endif
				windowManager.UpdateTree(nativeWindow);

				//nativeWindow->Flags.isBatching = false;
				::ValidateRect(hWnd, nullptr);
			}

			wasHandled = true;
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

			wasHandled = true;
			break;
		}
		//case WM_SIZING:
		//{
		//	::RECT* rect = reinterpret_cast<RECT*>(lParam);
		//	uint32_t newWidth = static_cast<uint32_t>(rect->right - rect->left) - nativeWindow->BorderSize.Width;
		//	uint32_t newHeight = static_cast<uint32_t>(rect->bottom - rect->top) - nativeWindow->BorderSize.Height;
		//	
		//	Size newSize{ newWidth , newHeight };
		//	if (nativeWindow->Type == WindowType::RenderForm && nativeWindow->CustomPaint)
		//	{
		//		windowManager.Resize(nativeWindow, newSize, false);
		//	}
		//	wasHandled = true;
		//	break;
		//}
		case WM_SIZE:
		{
			Size newSize{ (uint32_t)LOWORD(lParam) , (uint32_t)HIWORD(lParam) };
#if BT_DEBUG
			//BT_CORE_DEBUG << "   Size: new size " << newSize << ". window = " << nativeWindow->Name << std::endl;
#else
			BT_CORE_DEBUG << "   Size: new size " << newSize << std::endl;
#endif
			if (newSize.Width > 0 && newSize.Height > 0)
			{
				if (nativeWindow->RootPaintHandle.RenderTarget)
				{
					auto hr = nativeWindow->RootPaintHandle.RenderTarget->Resize(D2D1::SizeU(newSize.Width, newSize.Height));
					if (FAILED(hr))
					{
						BT_CORE_ERROR << "Error while resizing HWND render target." << std::endl;
					}
				}

				windowManager.Resize(nativeWindow, newSize, false);

				if (nativeWindow->HasCustomPaint())
				{
					API::RefreshWindow(nativeWindowHandle);
				}
				//else
				//{
				//	API::RefreshWindow(nativeWindowHandle);
				//}
			}
			
			wasHandled = true;
			break;
		}
		//case WM_WINDOWPOSCHANGING:
		//{
		//	::WINDOWPOS* pwp = (::WINDOWPOS*)lParam;
		//	Size newSize{ (uint32_t)pwp->cx , (uint32_t)pwp->cy };
		//	if (nativeWindow->Type == WindowType::RenderForm && nativeWindow->CustomPaint)
		//	{
		//		//windowManager.Resize(nativeWindow, newSize, false);
		//		//nativeWindow->CustomPaint();
		//	}

		//	wasHandled = true;
		//	break;
		//}
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

			API::RefreshWindow(nativeWindowHandle, true);
			wasHandled = false;
			break;
		}
		case WM_SETFOCUS:
		{
			if (rootFocusedWindow)
			{
				ArgFocus argFocus{ true };
				foundation.ProcessEvents(rootFocusedWindow, &Renderer::Focus, &ControlEvents::Focus, argFocus);
			}
			wasHandled = false;
			break;
		}
		case WM_KILLFOCUS:
		{
			if (menuManager.AnyPopupActive())
			{
				menuManager.CloseAll();
			}

			if (rootFocusedWindow)
			{
				ArgFocus argFocus{ false };
				foundation.ProcessEvents(rootFocusedWindow, &Renderer::Focus, &ControlEvents::Focus, argFocus);
			}
			wasHandled = false;
			break;
		}
		case WM_MOUSEACTIVATE: //This is not sent while mouse is captured
		{
			if (!nativeWindow->Flags.MakeActive)
			{
				return MA_NOACTIVATE;
			}
			wasHandled = false;
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			wasHandled = true;
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			auto window = windowManager.Find(nativeWindow, { x, y });
			if (window && window->Flags.IsEnabled)
			{
				rootPressedWindow = window;

				auto focusWindow = window->Flags.MakeActive ? window : window->MakeTargetWhenInactive;
				if (focusWindow && !focusWindow->Flags.IgnoreMouseFocus)
				{
					if (rootFocusedWindow != focusWindow)
					{
						if (rootFocusedWindow)
						{
							ArgFocus argFocus{ false };
							foundation.ProcessEvents(rootFocusedWindow, &Renderer::Focus, &ControlEvents::Focus, argFocus);
						}
						if (focusWindow)
						{
							ArgFocus argFocus{ true, ArgFocus::Reason::MousePress };
							foundation.ProcessEvents(focusWindow, &Renderer::Focus, &ControlEvents::Focus, argFocus);
						}
					}
					rootFocusedWindow = focusWindow;
				}

				auto pointToScreen = API::GetPointClientToScreen(nativeWindowHandle, { x,y });
				auto pointToClient = API::GetPointScreenToClient(window->RootHandle, pointToScreen);

				ArgMouse argMouseDown;
				argMouseDown.Position = pointToClient - windowManager.GetWindowRootPosition(window);
				argMouseDown.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
				argMouseDown.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
				argMouseDown.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

				foundation.ProcessEvents(window, &Renderer::MouseDown, &ControlEvents::MouseDown, argMouseDown);
			}
			
			break;
		}
		case WM_MOUSEMOVE:
		{
			wasHandled = true;
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));
			
			auto window = windowManager.Find(nativeWindow, { x, y });
			if (window && window != rootHoveredWindow)
			{
				if (rootHoveredWindow && windowManager.Exists(rootHoveredWindow))
				{
					auto pointToScreen = API::GetPointClientToScreen(nativeWindowHandle, { x,y });
					auto pointToClient = API::GetPointScreenToClient(rootHoveredWindow->RootHandle, pointToScreen);

					ArgMouse argMouseLeave;
					argMouseLeave.Position = pointToClient - windowManager.GetWindowRootPosition(rootHoveredWindow);
					argMouseLeave.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
					argMouseLeave.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
					argMouseLeave.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

					foundation.ProcessEvents(rootHoveredWindow, &Renderer::MouseLeave, &ControlEvents::MouseLeave, argMouseLeave);
				}
				rootHoveredWindow = nullptr;
			}

			if (window && window->Flags.IsEnabled && !window->Flags.IsDisposed)
			{
				auto pointToScreen = API::GetPointClientToScreen(nativeWindowHandle, { x,y });
				auto pointToClient = API::GetPointScreenToClient(window->RootHandle, pointToScreen);
				Point position = pointToClient - windowManager.GetWindowRootPosition(window);
				if (window != rootHoveredWindow)
				{
					if (window->ClientSize.IsInside(position))
					{
						ArgMouse argMouseEnter;
						argMouseEnter.Position = position;
						argMouseEnter.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
						argMouseEnter.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
						argMouseEnter.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

						foundation.ProcessEvents(window, &Renderer::MouseEnter, &ControlEvents::MouseEnter, argMouseEnter);
					}
					rootHoveredWindow = window;
				}

				if (rootHoveredWindow)
				{
					ArgMouse argMouseMove;
					argMouseMove.Position = position;
					argMouseMove.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
					argMouseMove.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
					argMouseMove.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

					foundation.ProcessEvents(window, &Renderer::MouseMove, &ControlEvents::MouseMove, argMouseMove);
				}
				if (!rootWindowData->IsTracking && window->ClientSize.IsInside(position))
				{
#if BT_DEBUG
					//BT_CORE_DEBUG << " - keep track / name " << window->Name << ". hWnd " << hWnd << std::endl;
#else
					//BT_CORE_DEBUG << " - keep track / window " << window << ". hWnd " << hWnd << std::endl;
#endif
					trackEvent.hwndTrack = hWnd;
					::TrackMouseEvent(&trackEvent);
					rootWindowData->IsTracking = true;
				}
			}
			
			break;
		}
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			wasHandled = true;
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			auto window = windowManager.Find(nativeWindow, { x, y });
			if (window && window->Flags.IsEnabled)
			{
				auto pointToScreen = API::GetPointClientToScreen(nativeWindowHandle, { x,y });
				auto pointToClient = API::GetPointScreenToClient(window->RootHandle, pointToScreen);
				Point position = pointToClient - windowManager.GetWindowRootPosition(window);

				ArgMouse argMouseUp;
				argMouseUp.Position = position;
				argMouseUp.ButtonState.LeftButton = message == WM_LBUTTONUP;
				argMouseUp.ButtonState.RightButton = message == WM_RBUTTONUP;
				argMouseUp.ButtonState.MiddleButton = message == WM_MBUTTONUP;

				if (window->ClientSize.IsInside(argMouseUp.Position) && window == rootPressedWindow)
				{
					ArgClick argClick;
					foundation.ProcessEvents(window, &Renderer::Click, &ControlEvents::Click, argClick);
				}

				foundation.ProcessEvents(window, &Renderer::MouseUp, &ControlEvents::MouseUp, argMouseUp);

				rootReleasedWindow = rootPressedWindow;
			}
			rootPressedWindow = nullptr;

			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			wasHandled = true;
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));

			auto window = windowManager.Find(nativeWindow, { x, y });
			if (window && window->Flags.IsEnabled && window == rootReleasedWindow)
			{
				ArgMouse argMouse{};
				argMouse.Position = Point{ x, y } - windowManager.GetWindowRootPosition(window);
				argMouse.ButtonState.LeftButton = (wParam & MK_LBUTTON) != 0;
				argMouse.ButtonState.RightButton = (wParam & MK_RBUTTON) != 0;
				argMouse.ButtonState.MiddleButton = (wParam & MK_MBUTTON) != 0;

				foundation.ProcessEvents(window, &Renderer::DblClick, &ControlEvents::DblClick, argMouse);
			}
			rootReleasedWindow = nullptr;
			break;
		}
		case WM_MOUSELEAVE:
		{
			wasHandled = true;
			rootWindowData->IsTracking = false;
			if (rootHoveredWindow && windowManager.Exists(rootHoveredWindow))
			{
				ArgMouse argMouseLeave;
				foundation.ProcessEvents(rootHoveredWindow, &Renderer::MouseLeave, &ControlEvents::MouseLeave, argMouseLeave);

				rootHoveredWindow = nullptr;
			}
			break;
		}
		case WM_MOUSEHWHEEL:
		case WM_MOUSEWHEEL:
		{
			wasHandled = true;

			int wheelDelta = ((int)(short)HIWORD(wParam));
			int x = ((int)(short)LOWORD(lParam));
			int y = ((int)(short)HIWORD(lParam));
			auto screenToClientPoint = API::GetPointScreenToClient(nativeWindow->RootHandle, { x, y });

			auto window = windowManager.Find(nativeWindow, { static_cast<int>(screenToClientPoint.X), static_cast<int>(screenToClientPoint.Y) });
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
			wasHandled = true;

			ArgKeyboard argKeyboard{};
			argKeyboard.ButtonState.Alt = (0 != (::GetKeyState(VK_MENU) & 0x80));
			argKeyboard.ButtonState.Ctrl = (0 != (::GetKeyState(VK_CONTROL) & 0x80));
			argKeyboard.ButtonState.Shift = (0 != (::GetKeyState(VK_SHIFT) & 0x80));

			argKeyboard.Key = static_cast<wchar_t>(wParam);

			auto window = rootFocusedWindow;
			if (window == nullptr)
			{
				window = nativeWindow;
			}

			foundation.ProcessEvents(window, &Renderer::KeyChar, &ControlEvents::KeyChar, argKeyboard);

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

			auto window = rootFocusedWindow;
			if (window == nullptr)
			{
				window = nativeWindow;
			}
			
			WORD keyFlags = HIWORD(lParam);
			BOOL isKeyReleased = (keyFlags & KF_UP) == KF_UP;

			auto target = window;
			if (menuManager.AnyPopupActive())
			{
				target = menuManager.GetActiveMenu(true);
			}
			if (isKeyReleased)
			{
				foundation.ProcessEvents(target, &Renderer::KeyReleased, &ControlEvents::KeyReleased, argKeyboard);
			}
			else
			{
				foundation.ProcessEvents(target, &Renderer::KeyPressed, &ControlEvents::KeyPressed, argKeyboard);
			}
			
			break;
		}
		case WM_ENTERSIZEMOVE:
		{
			foundation.EventEnterSizeMove(nativeWindow);
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			foundation.EventExitSizeMove(nativeWindow);
			break;
		}
		case WM_CLOSE:
		{
			ArgDisposing argDisposing{ false };
			auto events = dynamic_cast<FormEvents*>(nativeWindow->Events.get());
			events->Disposing.Emit(argDisposing);
			if (argDisposing.Cancel)
			{
				wasHandled = true;
			}
			
			break;
		}
		case WM_DESTROY: // WM_DESTROY, next WM_NCDESTROY
		{
			windowManager.Destroy(nativeWindow);
			wasHandled = true;
			
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
				wasHandled = true;
			}
			break;
		}
		}

#ifdef BT_PRINT_WND_MESSAGES
		if (it != g_debugWndMessages.end())
		{
			//BT_CORE_DEBUG << "<< WndProc message: " << it->second << ". hWnd = " << hWnd << ". window = " << nativeWindow->Name << std::endl;
		}
#endif
		rootWindowData = windowManager.GetFormData(nativeWindowHandle);
		if (rootWindowData)
		{
			rootWindowData->Focused = rootFocusedWindow;
			rootWindowData->Hovered = rootHoveredWindow;
			rootWindowData->Pressed = rootPressedWindow;
			rootWindowData->Released = rootReleasedWindow;
		}
		if (!wasHandled)
		{
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}

	bool IsDefaultMessage(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam, LRESULT& result)
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
		default:
			if ((WM_MOUSEFIRST <= message && message <= WM_MOUSELAST) || (WM_KEYFIRST <= message && message <= WM_KEYLAST))
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