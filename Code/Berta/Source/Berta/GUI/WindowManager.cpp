/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "WindowManager.h"

#include "Berta/GUI/Window.h"
#include "Berta/Controls/MenuBar.h"
#include <stack>

namespace Berta
{
	WindowManager::FormData::FormData(FormData&& other) noexcept :
		WindowPtr(other.WindowPtr),
		RootGraphics(std::move(other.RootGraphics))
	{
	}

	WindowManager::FormData::FormData(Window* window, const Size& size) :
		WindowPtr(window),
		RootGraphics(size)
	{
	}

	void WindowManager::Add(Window* window)
	{
		m_windowRegistry.emplace(window);
	}

	void WindowManager::AddNative(API::NativeWindowHandle nativeWindowHandle, FormData&& append)
	{
		m_windowNativeRegistry.emplace(nativeWindowHandle, std::move(append));
	}

	void WindowManager::Caption(Window* window, const std::wstring& caption)
	{
		window->Title = caption;
		if (window->Type == WindowType::Form)
		{
			API::CaptionNativeWindow(window->RootHandle, caption);
		}
	}

	void WindowManager::Destroy(Window* window)
	{
		if (!Exists(window))
		{
			return;
		}

		if (window->Parent)
		{
			BT_CORE_DEBUG << "  Window " << window << " name = " << window->Name << " has parent ." << window->Parent << std::endl;
			for (size_t i = 0; i < window->Parent->Children.size(); i++)
			{
				if (window->Parent->Children[i] == window)
				{
					BT_CORE_DEBUG << "    - Child index " << i << std::endl;
					window->Parent->Children.erase(window->Parent->Children.begin() + i);
					break;
				}
			}
		}

		DestroyInternal(window);
	}

	void WindowManager::DestroyInternal(Window* window)
	{
		if (window->Flags.IsDestroyed)
		{
			return;
		}

		window->Flags.IsDestroyed = true;

		ArgDestroy argDestroy;
		window->Events->Destroy.Emit(argDestroy);

		for (size_t i = 0; i < window->Children.size(); i++)
		{
			auto child = window->Children[i];
			//BT_CORE_DEBUG << "    - DestroyInternal. Child Window " << child << std::endl;
			DestroyInternal(child);
			//delete child; //TODO: make it shared ptr (Window*) or maybe move this deallocation to Remove method here (when WM_NCDESTROY is sent)
		}
		window->Children.clear();

		//BT_CORE_TRACE << "DestroyInternal / Release Capture = " << m_capture.WindowPtr << ". window " << window << std::endl;
		if (m_capture.WindowPtr == window)
		{
			ReleaseCapture(m_capture.WindowPtr);
		}
		
		window->Renderer.Shutdown();
		window->ControlWindowPtr->Destroy();
		if (window->Type != WindowType::Form)
		{
			m_windowRegistry.erase(window);
		}

		window->Renderer.GetGraphics().Release();
	}

	void WindowManager::UpdateTreeInternal(Window* window, Graphics& rootGraphics)
	{
		if (window == nullptr)
		{
			return;
		}

		for (auto& child : window->Children)
		{
			if (!child->Visible)
				continue;

			if (child->Type != WindowType::Panel)
			{
				child->Renderer.Update();
				auto absolutePosition = GetAbsolutePosition(child);
				Rectangle childRectangle{ absolutePosition.X, absolutePosition.Y, child->Size.Width, child->Size.Height };
				rootGraphics.BitBlt(childRectangle, child->Renderer.GetGraphics(), { 0,0 });
			}
			UpdateTreeInternal(child, rootGraphics);
		}
	}

	void WindowManager::RefreshInternal(Window* window, Graphics& rootGraphics)
	{
		if (window == nullptr)
		{
			return;
		}

		for (auto& child : window->Children)
		{
			if (!child->Visible)
				continue;

			if (child->Type != WindowType::Panel)
			{
				auto childAbsolutePosition = GetAbsolutePosition(child);
				Rectangle childRectangle{ childAbsolutePosition.X, childAbsolutePosition.Y, child->Size.Width, child->Size.Height };
				rootGraphics.BitBlt(childRectangle, child->Renderer.GetGraphics(), { 0,0 });
			}
			RefreshInternal(child, rootGraphics);
		}
	}

	void WindowManager::UpdateDeferredRequestsInternal(Window* request, Graphics& rootGraphics)
	{
		if (request == nullptr)
		{
			return;
		}

		for (auto& child : request->Children)
		{
			if (!child->Visible || !Exists(child))
				continue;

			if (child->Type != WindowType::Panel)
			{
				auto absolutePosition = GetAbsolutePosition(child);
				Rectangle childRectangle{ absolutePosition.X, absolutePosition.Y, child->Size.Width, child->Size.Height };

				rootGraphics.BitBlt(childRectangle, child->Renderer.GetGraphics(), { 0,0 });
			}
			UpdateDeferredRequestsInternal(child, rootGraphics);
		}
	}

	void WindowManager::Dispose(Window* window)
	{
		if (window->Flags.IsDestroyed)
		{
			return;
		}

		if (window->Type == WindowType::Form)
		{
			ArgClosing argClosing{ false };
			auto events = dynamic_cast<FormEvents*>(window->Events.get());
			events->Closing.Emit(argClosing);

			if (!argClosing.Cancel)
			{
				if (!window->Flags.IsDestroyed)
				{
					window->Renderer.Shutdown();
					window->ControlWindowPtr->Destroy();
				}
				API::DestroyNativeWindow(window->RootHandle);
			}
		}
		else
		{
			Destroy(window);
		}
	}

	void WindowManager::Remove(Window* window)
	{
		if (window->Type == WindowType::Form)
		{
			m_windowNativeRegistry.erase(window->RootHandle);
			m_windowRegistry.erase(window);
		}
	}

	Window* WindowManager::Get(API::NativeWindowHandle nativeWindowHandle) const
	{
		auto it = m_windowNativeRegistry.find(nativeWindowHandle);
		if (it != m_windowNativeRegistry.end())
		{
			return it->second.WindowPtr;
		}

		return nullptr;
	}

	Berta::WindowManager::FormData* WindowManager::GetFormData(API::NativeWindowHandle nativeWindowHandle)
	{
		auto it = m_windowNativeRegistry.find(nativeWindowHandle);
		if (it != m_windowNativeRegistry.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	bool WindowManager::Exists(Window* window) const
	{
		return m_windowRegistry.find(window) != m_windowRegistry.end();
	}

	uint32_t WindowManager::NativeWindowCount()
	{
		return static_cast<uint32_t>(m_windowNativeRegistry.size());
	}

	void WindowManager::Capture(Window* window)
	{
		BT_CORE_TRACE << "Capture / WindowPtr = " << m_capture.WindowPtr << ". window " << window << std::endl;
		if (m_capture.WindowPtr != window)
		{
			if (Exists(window))
			{
				API::CaptureWindow(window->RootHandle, true);

				if (m_capture.WindowPtr)
				{
					m_capture.PrevCaptured.emplace_back(m_capture.WindowPtr);
				}
				m_capture.WindowPtr = window;
			}
		}
	}

	void WindowManager::ReleaseCapture(Window* window)
	{
		if (m_capture.WindowPtr == window)
		{
			m_capture.WindowPtr = nullptr;
			if (!m_capture.PrevCaptured.empty())
			{
				auto lastCaptured = m_capture.PrevCaptured.back();
				m_capture.PrevCaptured.pop_back();

				if (Exists(lastCaptured))
				{
					m_capture.WindowPtr = lastCaptured;

					API::CaptureWindow(lastCaptured->RootHandle, true);
				}
			}

			if (window && m_capture.WindowPtr == nullptr)
			{
				API::CaptureWindow(window->RootHandle, false);
			}
		}
		else
		{
			auto it = std::find(m_capture.PrevCaptured.begin(), m_capture.PrevCaptured.end(), window);
			if (it != m_capture.PrevCaptured.end())
			{
				m_capture.PrevCaptured.erase(it);
			}
		}
	}

	Window* WindowManager::Find(Window* window, const Point& point)
	{
		if (window == nullptr || !window->Visible)
		{
			return nullptr;
		}

		if (!m_capture.WindowPtr)
		{
			if (window->Visible && IsPointOnWindow(window, point))
			{
				return FindInTree(window, point);
			}

			return nullptr;
		}

		if (window->Visible && IsPointOnWindow(window, point))
		{
			auto target = FindInTree(window, point);

			auto current = target;
			while (current)
			{
				if (current == m_capture.WindowPtr)
				{
					return target;
				}

				current = current->Parent;
			}
		}

		return m_capture.WindowPtr;
	}

	void WindowManager::UpdateTree(Window* window)
	{
		window->Renderer.Update(); //Update control's window.
		auto& rootGraphics = *(window->RootGraphics);

		Rectangle requestRectangle = window->Size.ToRectangle();
		auto absolutePosition = GetAbsolutePosition(window);
		requestRectangle.X = absolutePosition.X;
		requestRectangle.Y = absolutePosition.Y;

		rootGraphics.BitBlt(requestRectangle, window->Renderer.GetGraphics(), { 0,0 }); // Copy from control's graphics to root graphics.
		
		UpdateTreeInternal(window, rootGraphics);

		window->Renderer.Map(window, requestRectangle); // Copy from root graphics to native hwnd window.
	}

	void WindowManager::Show(Window* window, bool visible)
	{
		if (window->Visible != visible)
		{
			if (window->Type == WindowType::Form)
			{
				API::ShowNativeWindow(window->RootHandle, visible, window->Flags.MakeActive);
			}
			else
			{
				window->Visible = visible;

				ArgVisibility argVisibility;
				argVisibility.IsVisible = visible;
				window->Events->Visibility.Emit(argVisibility);

				auto windowToUpdate = window->Parent;
				while (windowToUpdate && windowToUpdate->Type == WindowType::Panel)
				{
					windowToUpdate = windowToUpdate->Parent;
				}
				if (windowToUpdate)
				{
					UpdateTree(windowToUpdate);
				}
			}
		}
	}

	void WindowManager::Resize(Window* window, const Size& newSize, bool updateTree)
	{
		if (window->Size != newSize)
		{
			window->Size = newSize;

			Graphics newGraphics;
			Graphics newRootGraphics;
			if (window->Type != WindowType::Panel)
			{
				newGraphics.Build(newSize);
				newGraphics.BuildFont(window->DPI);

				if (window->Type == WindowType::Form)
				{
					newRootGraphics.Build(newSize);
					newRootGraphics.BuildFont(window->DPI);
				}
			}			

			if (window->Type != WindowType::Panel)
			{
				window->Renderer.GetGraphics().Swap(newGraphics);

				if (window->Type == WindowType::Form)
				{
					window->RootGraphics->Swap(newRootGraphics);
				}
			}

			if (updateTree)
			{
				UpdateTree(window);
			}
		}
	}

	void WindowManager::Move(Window* window, const Rectangle& newRect)
	{
		Rectangle currentRect{ window->Position, window->Size };
		if (currentRect != newRect)
		{
			window->Position = newRect;
			if (window->Size != newRect)
			{
				Resize(window, newRect, false);
			}
		}
	}

	void WindowManager::Refresh(Window* window)
	{
		auto& rootGraphics = *(window->RootWindow->RootGraphics);

		auto absolutePosition = GetAbsolutePosition(window);
		Rectangle requestRectangle{ absolutePosition.X, absolutePosition.Y, window->Size.Width, window->Size.Height };

		rootGraphics.BitBlt(requestRectangle, window->Renderer.GetGraphics(), { 0,0 }); // Copy from control's graphics to root graphics.

		RefreshInternal(window, rootGraphics);

		window->RootWindow->Renderer.Map(window->RootWindow, requestRectangle); // Copy from root graphics to native hwnd window.
	}

	void WindowManager::UpdateDeferredRequests(Window* rootWindow)
	{
		if (rootWindow->DeferredRequests.size() == 0)
		{
			return;
		}

		auto& rootGraphics = *(rootWindow->RootGraphics);
		for (auto& request : rootWindow->DeferredRequests)
		{
			if (Exists(request))
			{
				//TODO: don't update a window twice. A child could be in the queue, add check.
				auto absolutePosition = GetAbsolutePosition(request);
				Rectangle requestRectangle{ absolutePosition.X, absolutePosition.Y, request->Size.Width, request->Size.Height };
				rootGraphics.BitBlt(requestRectangle, request->Renderer.GetGraphics(), { 0,0 }); // Copy from control's graphics to root graphics.

				UpdateDeferredRequestsInternal(request, rootGraphics);

				rootWindow->Renderer.Map(rootWindow, requestRectangle); // Copy from root graphics to native hwnd window.
			}
		}

		rootWindow->DeferredRequests.clear();
	}

	void WindowManager::ChangeDPI(Window* window, uint32_t newDPI)
	{
		if (window->DPI != newDPI)
		{
			auto oldDPI = window->DPI;
			window->DPI = newDPI;
			window->DPIScaleFactor = LayoutUtils::CalculateDPIScaleFactor(newDPI);

			float scalingFactor = (float)newDPI / oldDPI;
			window->Position.X = static_cast<int>(window->Position.X * scalingFactor);
			window->Position.Y = static_cast<int>(window->Position.Y * scalingFactor);
			window->Size.Width = static_cast<uint32_t>(window->Size.Width * scalingFactor);
			window->Size.Height = static_cast<uint32_t>(window->Size.Height * scalingFactor);

			auto& graphics = window->Renderer.GetGraphics();
			graphics.Release();
			graphics.Build(window->Size);
			graphics.BuildFont(newDPI);

			for (auto& child : window->Children)
			{
				ChangeDPI(child, newDPI);
			}

			ArgResize argResize;
			argResize.NewSize = window->Size;
			window->Renderer.Resize(argResize);
		}
	}

	void WindowManager::ChangeCursor(Window* window, Cursor newCursor)
	{
		auto& rootHandle = window->RootWindow->RootHandle;
		auto rootData = GetFormData(rootHandle);
		if (rootData)
		{
			if (rootData->CurrentCursor.CursorType != newCursor)
			{
				if (API::ChangeCursor(rootHandle, newCursor, rootData->CurrentCursor))
				{
					rootData->CurrentCursor.CursorType = newCursor;
				}
			}
		}
	}

	Point WindowManager::GetAbsolutePosition(Window* window)
	{
		Point position{ window->Position };
		window = window->Parent;
		while (window)
		{
			position += window->Position;
			window = window->Parent;
		}
		return position;
	}

	void WindowManager::SetMenu(MenuItemReactor* rootMenuItemWindow, MenuBarItemReactor* menuBarItemReactor)
	{
		m_rootMenuItemReactor = rootMenuItemWindow;
		m_menuBarItemReactor = menuBarItemReactor;
	}

	std::pair<MenuBarItemReactor*, MenuItemReactor*> WindowManager::GetMenu()
	{
		return std::make_pair(m_menuBarItemReactor, m_rootMenuItemReactor);
	}

	void WindowManager::DisposeMenu(bool disposeRoot)
	{
		if (!m_rootMenuItemReactor)
			return;

		std::stack<Window*> stack;
		auto current = m_rootMenuItemReactor->Next();
		while (current)
		{
			auto temp = current->Next();
			stack.push(current->Owner());
			
			current = temp;
		}

		while (!stack.empty())
		{
			auto& it = stack.top();
			stack.pop();

			Dispose(it);
		}

		if (disposeRoot)
		{
			m_rootMenuItemReactor->Clear();
			m_rootMenuItemReactor = nullptr;
			m_menuBarItemReactor = nullptr;
		}
	}

	void WindowManager::DisposeMenu(MenuItemReactor* rootReactor)
	{
		if (!rootReactor)
		{
			return;
		}

		std::stack<Window*> stack;
		auto current = rootReactor;
		while (current)
		{
			auto temp = current->Next();
			stack.push(current->Owner());

			current = temp;
		}

		while (!stack.empty())
		{
			auto& it = stack.top();
			stack.pop();

			Dispose(it);
		}
	}


	bool WindowManager::IsPointOnWindow(Window* window, const Point& point)
	{
		auto absolutePosition = GetAbsolutePosition(window);

		Rectangle rect
		{
			absolutePosition,
			window->Size
		};
		return rect.IsInside(point);
	}

	Window* WindowManager::FindInTree(Window* window, const Point& point)
	{
		if (!window->Visible)
		{
			return nullptr;
		}

		if (!window->Children.empty())
		{
			auto index = window->Children.size();

			do
			{
				auto child = window->Children[--index];
				if (child->Type != WindowType::Form && IsPointOnWindow(child, point))
				{
					child = FindInTree(child, point);
					if (child)
					{
						//BT_CORE_DEBUG << " -- find tree child = " << child->Name << std::endl;
						return child;
					}
				}
			} while (index != 0);
		}
		//BT_CORE_DEBUG << " -- find tree fallback = " << window->Name << std::endl;
		return window;
	}
}