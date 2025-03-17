/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "WindowManager.h"

#include "Berta/GUI/Window.h"
#include "Berta/Controls/MenuBar.h"
#include "Berta/Core/Foundation.h"

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
			for (size_t i = 0; i < window->Parent->Children.size(); i++)
			{
				if (window->Parent->Children[i] == window)
				{
					window->Parent->Children.erase(window->Parent->Children.begin() + i);
					break;
				}
			}
		}

		DestroyInternal(window);
	}

	void WindowManager::DestroyInternal(Window* window)
	{
		if (window->Flags.IsDisposed)
		{
			return;
		}

		auto& foundation = Foundation::GetInstance();
		window->Flags.IsDisposed = true;

		ArgDestroy argDestroy;
		foundation.ProcessEvents(window, static_cast<void(Renderer::*)(const ArgDestroy&)>(nullptr), &ControlEvents::Destroy, argDestroy);

		while (!window->Children.empty())
		{
			auto child = window->Children.back();
			if (child->Type == WindowType::Form)
			{
				API::DestroyNativeWindow(child->RootHandle); //TODO: don't know if this makes sense here.
				continue;
			}
			//BT_CORE_DEBUG << "    - DestroyInternal. Child Window " << child << std::endl;
			DestroyInternal(child);
			window->Children.pop_back();
			//delete child; //TODO: make it shared ptr (Window*) or maybe move this deallocation to Remove method (when WM_NCDESTROY is sent)
		}

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

	void WindowManager::UpdateTreeInternal(Window* window, Graphics& rootGraphics, Point parentPosition)
	{
		if (window == nullptr)
		{
			return;
		}

		Rectangle parentRectangle{ parentPosition.X, parentPosition.Y, window->Size.Width, window->Size.Height };
		for (auto& child : window->Children)
		{
			if (!child->Visible)
			{
				continue;
			}

			auto absolutePositionChild = GetLocalPosition(child);
			absolutePositionChild += parentPosition;
			if (child->Type != WindowType::Panel)
			{
				if (child->Type == WindowType::Form)
				{
					Paint(child, true);
					continue;
				}

				child->Renderer.Update();

				Rectangle childRectangle{ absolutePositionChild.X, absolutePositionChild.Y, child->Size.Width, child->Size.Height };
				if (GetIntersectionClipRect(parentRectangle, childRectangle, childRectangle))
				{
					rootGraphics.BitBlt(childRectangle, child->Renderer.GetGraphics(), { 0,0 });
				}
			}
			UpdateTreeInternal(child, rootGraphics, absolutePositionChild);
		}
	}

	void WindowManager::PaintInternal(Window* window, Graphics& rootGraphics, bool doUpdate, Point parentPosition)
	{
		if (window == nullptr)
		{
			return;
		}

		Rectangle parentRectangle{ parentPosition.X, parentPosition.Y, window->Size.Width, window->Size.Height };
		for (auto& child : window->Children)
		{
			if (!child->Visible)
			{
				continue;
			}

			auto childAbsolutePosition = GetLocalPosition(child);
			childAbsolutePosition += parentPosition;
			if (child->Type != WindowType::Panel)
			{
				if (doUpdate && !child->Flags.isUpdating)
				{
					child->Flags.isUpdating = true;
					child->Renderer.Update();
					child->Flags.isUpdating = false;
				}

				Rectangle childRectangle{ childAbsolutePosition.X, childAbsolutePosition.Y, child->Size.Width, child->Size.Height };
				if (GetIntersectionClipRect(parentRectangle, childRectangle, childRectangle))
				{
					rootGraphics.BitBlt(childRectangle, child->Renderer.GetGraphics(), { 0,0 });
				}
			}
			PaintInternal(child, rootGraphics, doUpdate, childAbsolutePosition);
		}
	}

	bool WindowManager::GetIntersectionClipRect(const Rectangle& parentRectangle, const Rectangle& childRectangle, Rectangle& result)
	{
		if (parentRectangle.X + (int)parentRectangle.Width <= childRectangle.X || childRectangle.X + (int)childRectangle.Width <= parentRectangle.X ||
			parentRectangle.Y + (int)parentRectangle.Height <= childRectangle.Y || childRectangle.Y + (int)childRectangle.Height <= parentRectangle.Y) {
			return false;
		}

		// Calculate the intersection rectangle
		int interLeft = (std::max)(parentRectangle.X, childRectangle.X);
		int interTop = (std::max)(parentRectangle.Y, childRectangle.Y);
		int interRight = (std::min)(parentRectangle.X + parentRectangle.Width, childRectangle.X + childRectangle.Width);
		int interBottom = (std::min)(parentRectangle.Y + parentRectangle.Height, childRectangle.Y + childRectangle.Height);

		// Set the intersection rectangle's position and size
		result.X = interLeft;
		result.Y = interTop;
		result.Width = interRight - interLeft;
		result.Height = interBottom - interTop;

		return true;
	}

	void WindowManager::Paint(Window* window, bool doUpdate)
	{
		if (doUpdate && !window->Flags.isUpdating)
		{
			window->Flags.isUpdating = true;
			window->Renderer.Update();
			window->Flags.isUpdating = false;
		}

		auto& rootGraphics = *(window->RootWindow->RootGraphics);
		auto absolutePosition = GetAbsoluteRootPosition(window);
		Rectangle requestRectangle{ absolutePosition.X, absolutePosition.Y, window->Size.Width, window->Size.Height };

		rootGraphics.BitBlt(requestRectangle, window->Renderer.GetGraphics(), { 0,0 }); // Copy from control's graphics to root graphics.
		PaintInternal(window, rootGraphics, doUpdate, absolutePosition);
	}

	void WindowManager::Dispose(Window* window)
	{
		if (window->Flags.IsDisposed)
		{
			return;
		}

		if (window->Type == WindowType::Form)
		{
			ArgDisposing argDisposing{ false };
			auto events = dynamic_cast<FormEvents*>(window->Events.get());
			events->Disposing.Emit(argDisposing);

			if (!argDisposing.Cancel)
			{
				if (!window->Flags.IsDisposed)
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

	void WindowManager::Capture(Window* window, bool redirectToChildren)
	{
		//BT_CORE_TRACE << "Capture / WindowPtr = " << m_capture.WindowPtr << ". window " << window << std::endl;
		if (m_capture.WindowPtr != window)
		{
			if (Exists(window))
			{
				API::CaptureWindow(window->RootHandle, true);

				if (m_capture.WindowPtr)
				{
					m_capture.PrevCaptured.emplace_back(m_capture.WindowPtr, m_capture.RedirectToChildren);
				}
				m_capture.WindowPtr = window;
				m_capture.RedirectToChildren = redirectToChildren;
			}
		}
	}

	void WindowManager::ReleaseCapture(Window* window)
	{
		//BT_CORE_TRACE << "ReleaseCapture / WindowPtr = " << m_capture.WindowPtr << ". window " << window << std::endl;
		if (m_capture.WindowPtr == window)
		{
			m_capture.WindowPtr = nullptr;
			if (!m_capture.PrevCaptured.empty())
			{
				auto& lastCaptured = m_capture.PrevCaptured.back();
				m_capture.PrevCaptured.pop_back();

				if (Exists(lastCaptured.WindowPtr))
				{
					m_capture.WindowPtr = lastCaptured.WindowPtr;
					m_capture.RedirectToChildren = lastCaptured.RedirectToChildren;

					//BT_CORE_TRACE << "   ReleaseCapture / true" << std::endl;
					API::CaptureWindow(lastCaptured.WindowPtr->RootHandle, true);
				}
			}

			if (window && m_capture.WindowPtr == nullptr)
			{
				//BT_CORE_TRACE << "   ReleaseCapture / false" << std::endl;
				API::CaptureWindow(window->RootHandle, false);
			}
		}
		else
		{
			auto it = m_capture.PrevCaptured.begin();
			while (it != m_capture.PrevCaptured.end())
			{
				if (it->WindowPtr == window)
				{
					m_capture.PrevCaptured.erase(it);
					break;
				}
				++it;
			}
		}
		//BT_CORE_TRACE << "ReleaseCapture / m_capture.PrevCaptured size = " << m_capture.PrevCaptured.size() << std::endl;
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

		if (m_capture.RedirectToChildren && window->Visible && IsPointOnWindow(window, point))
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
		if (!window->IsVisible())
		{
			return;
		}

		window->Renderer.Update(); //Update control's window.
		auto& rootGraphics = *(window->RootGraphics);

		Rectangle requestRectangle = window->Size.ToRectangle();
		auto absolutePosition = GetAbsoluteRootPosition(window);
		requestRectangle.X = absolutePosition.X;
		requestRectangle.Y = absolutePosition.Y;

		rootGraphics.BitBlt(requestRectangle, window->Renderer.GetGraphics(), { 0,0 }); // Copy from control's graphics to root graphics.
		
		UpdateTreeInternal(window, rootGraphics, absolutePosition);
	}

	void WindowManager::Map(Window* window, const Rectangle* areaToUpdate)
	{
		if (areaToUpdate == nullptr)
		{
			Rectangle requestRectangle = window->Size.ToRectangle();
			auto absolutePosition = GetAbsoluteRootPosition(window);
			requestRectangle.X = absolutePosition.X;
			requestRectangle.Y = absolutePosition.Y;

			window->Renderer.Map(window, requestRectangle); // Copy from root graphics to native hwnd window.
			return;
		}

		window->Renderer.Map(window, *areaToUpdate); // Copy from root graphics to native hwnd window.
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

				auto windowToUpdate = window->FindFirstNonPanelAncestor();
				if (windowToUpdate)
				{
					UpdateTree(windowToUpdate);
					auto position = GetAbsoluteRootPosition(windowToUpdate);
					Rectangle areaToUpdate{ position.X,position.Y, windowToUpdate->Size.Width, windowToUpdate->Size.Height };
					Map(windowToUpdate, &areaToUpdate);
				}
			}
		}
	}

	void WindowManager::Resize(Window* window, const Size& newSize, bool resizeForm)
	{
		auto& foundation = Foundation::GetInstance();
		if (window->Size != newSize)
		{
			window->Size = newSize;

			Graphics newGraphics;
			Graphics newRootGraphics;
			if (window->Type != WindowType::Panel)
			{
				newGraphics.Build(newSize);
				newGraphics.BuildFont(window->DPI);
				//newGraphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->Background, true);

				if (window->Type == WindowType::Form)
				{
					newRootGraphics.Build(newSize);
					newRootGraphics.BuildFont(window->DPI);
					newRootGraphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->Background, true); //TODO: not sure if we have to call this here.
				}
			}

			if (window->Type != WindowType::Panel)
			{
				window->Renderer.GetGraphics().Swap(newGraphics);

				if (window->Type == WindowType::Form)
				{
					window->RootGraphics->Swap(newRootGraphics);

					if (resizeForm)
					{
						API::ResizeChildWindow(window->RootHandle, window->Position, window->Size);
					}
				}
			}

			ArgResize argResize;
			argResize.NewSize = newSize;
#if BT_DEBUG
			//BT_CORE_TRACE << "* Resize() - window = " << window->Name << ". newSize = " << newSize << std::endl;
#else
			//BT_CORE_TRACE << "* Resize(). newSize = "<< newSize<< std::endl;
#endif
			foundation.ProcessEvents(window, &Renderer::Resize, &ControlEvents::Resize, argResize);
		}
	}

	void WindowManager::Move(Window* window, const Rectangle& newRect)
	{
		auto& foundation = Foundation::GetInstance();

		Rectangle currentRect{ window->Position, window->Size };
		if (currentRect != newRect)
		{
			bool sizeChanged = window->Size != newRect;
			bool positionChanged = window->Position != newRect;

			if (positionChanged)
			{
				window->Position = newRect;
			}

			if (sizeChanged)
			{
				Resize(window, newRect);
			}

			if (window->Type == WindowType::Form && positionChanged && !sizeChanged)
			{
				API::ResizeChildWindow(window->RootHandle, window->Position, window->Size);

				ArgMove argMove;
				argMove.NewPosition = newRect;
				foundation.ProcessEvents(window, &Renderer::Move, &ControlEvents::Move, argMove);
			}
		}
	}

	void WindowManager::Move(Window* window, const Point& newPosition)
	{
		auto& foundation = Foundation::GetInstance();
		if (window->Position != newPosition)
		{
			window->Position = newPosition;
			if (window->Type == WindowType::Form)
			{
				API::ResizeChildWindow(window->RootHandle, window->Position, window->Size);
			}

			ArgMove argMove;
			argMove.NewPosition = newPosition;
			foundation.ProcessEvents(window, &Renderer::Move, &ControlEvents::Move, argMove);
		}
	}

	void WindowManager::Update(Window* window)
	{		
		if (window->Flags.isUpdating)
		{
			BT_CORE_WARN << " - WindowManager.Update() / ALREADY updating..." << std::endl;
		}

		if (!window->Flags.isUpdating)
		{
			window->Flags.isUpdating = true;
			window->Renderer.Update();
			window->Flags.isUpdating = false;
		}

		if (!TryDeferredUpdate(window))
		{
#if BT_DEBUG
			BT_CORE_TRACE << " - WindowManager.Update() / paint and map..." << window->Name << std::endl;
#else
			BT_CORE_TRACE << " - WindowManager.Update() / paint and map..." << std::endl;
#endif
			Paint(window, false);
			Map(window, nullptr);
		}
	}

	bool WindowManager::TryDeferredUpdate(Window* window)
	{
		if (!window->Visible || !window->AreParentsVisible())
		{
			return true;
		}

		if (window->RootWindow->Flags.IsDeferredCount == 0)
		{
			//TODO: paint and map
#if BT_DEBUG
			BT_CORE_TRACE << " -- TryDeferredUpdate / Paint and Map. window = " << window->Name << ". hwnd = " << window->RootHandle.Handle << std::endl;
#else
			BT_CORE_TRACE << " -- TryDeferredUpdate / Paint and Map. hwnd = " << window->RootHandle.Handle << std::endl;
#endif
			return false;
		}

		if (std::find(
			window->RootWindow->DeferredRequests.begin(),
			window->RootWindow->DeferredRequests.end(),
			window) == window->RootWindow->DeferredRequests.end()
			)
		{
			auto requestIt = window->RootWindow->DeferredRequests.begin();
			while (requestIt != window->RootWindow->DeferredRequests.end())
			{
				if ((*requestIt)->IsAncestorOf(window))
				{
					return true;
				}
				if (window->IsAncestorOf(*requestIt))
				{
					requestIt = window->RootWindow->DeferredRequests.erase(requestIt);
				}
				else
				{
					++requestIt;
				}
			}

			window->RootWindow->DeferredRequests.emplace_back(window);
		}
		return true;
	}

	void WindowManager::UpdateDeferredRequests(Window* rootWindow)
	{
		if (rootWindow->DeferredRequests.size() == 0)
		{
			return;
		}

		for (auto& request : rootWindow->DeferredRequests)
		{
			if (Exists(request) && !request->Flags.IsDisposed)
			{
				//TODO: don't update a window twice. A child could be in the queue, add check.
				Paint(request, false);
				Map(request, nullptr);

				//request->Status = WindowStatus::None;
			}
		}
	}

	void WindowManager::ChangeDPI(Window* window, uint32_t newDPI, const API::NativeWindowHandle& nativeWindowHandle)
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

			if (window->Type == WindowType::Form && window->RootHandle != nativeWindowHandle)
			{
				API::ResizeChildWindow(window->RootHandle, window->Position, window->Size);
				API::RefreshWindow(window->RootHandle);
			}

			for (auto& child : window->Children)
			{
				ChangeDPI(child, newDPI, nativeWindowHandle);
			}
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

	Cursor WindowManager::GetCursor(Window* window)
	{
		auto& rootHandle = window->RootWindow->RootHandle;
		auto rootData = GetFormData(rootHandle);
		if (rootData)
		{
			return rootData->CurrentCursor.CursorType;
		}

		return Cursor::Default;
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

	Point WindowManager::GetAbsoluteRootPosition(Window* window)
	{
		Point position{};
		while (window && window->Type != WindowType::Form)
		{
			position += window->Position;
			window = window->Parent;
		}
		return position;
	}

	Point WindowManager::GetLocalPosition(Window* window)
	{
		return window->Position;
	}

	void WindowManager::SetMenu(MenuItemReactor* rootMenuItemWindow)
	{
		m_rootMenuItemReactor = rootMenuItemWindow;
	}

	MenuItemReactor* WindowManager::GetMenu()
	{
		return m_rootMenuItemReactor;
	}

	void WindowManager::DisposeMenu()
	{
		DisposeMenu(m_rootMenuItemReactor);
		m_rootMenuItemReactor = nullptr;
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
			if (!current->IsMenuBar())
			{
				stack.push(current->Owner());
			}

			current = current->Next();
		}

		while (!stack.empty())
		{
			auto& it = stack.top();
			stack.pop();

			Dispose(it);
		}

		if (rootReactor == m_rootMenuItemReactor)
		{
			m_rootMenuItemReactor = nullptr;
		}
	}

	bool WindowManager::IsPointOnWindow(Window* window, const Point& point)
	{
		auto absolutePosition = GetAbsoluteRootPosition(window);

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
						return child;
					}
				}
			} while (index != 0);
		}
		return window;
	}
}