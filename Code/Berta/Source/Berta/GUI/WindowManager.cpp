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
		RootGraphics(size, window->DPI)
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

	bool WindowManager::Caption(Window* window, const std::wstring& caption)
	{
		if (window->Title == caption)
			return false;

		window->Title = caption;
		if (window->Type == WindowType::Form)
		{
			API::CaptionNativeWindow(window->RootHandle, caption);
		}
		return true;
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

		if (window->Type != WindowType::Form)
		{
#if BT_DEBUG
			BT_CORE_DEBUG << "    - Destroy. Window =" << window->Name << std::endl;
#else
			BT_CORE_DEBUG << "    - Destroy." << std::endl;
#endif
			//delete window;  //TODO: place this deallocation in a safe place!
		}
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
				API::DestroyNativeWindow(child->RootHandle); //child will be removed from parent's children.
				continue;
			}
			DestroyInternal(child);
			window->Children.pop_back();

#if BT_DEBUG
			//BT_CORE_DEBUG << "    - DestroyInternal. Child Window =" << child->Name << std::endl;
#else
			//BT_CORE_DEBUG << "    - DestroyInternal." << std::endl;
#endif
			//delete child; //TODO: place this deallocation in a safe place!
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

	void WindowManager::UpdateTreeInternal(Window* window, Graphics& rootGraphics, const Point& parentPosition, const Rectangle& containerRectangle)
	{
		if (window == nullptr)
		{
			return;
		}

		Rectangle newContainerRectangle = containerRectangle;
		for (auto& child : window->Children)
		{
			if (!child->Visible)
			{
				continue;
			}

			auto absolutePositionChild = GetLocalPosition(child);
			absolutePositionChild += parentPosition;
			Rectangle childRectangle{ absolutePositionChild.X, absolutePositionChild.Y, child->Size.Width, child->Size.Height };
			if (child->Type != WindowType::Panel)
			{
				if (child->Type == WindowType::Form)
				{
					Paint(child, true);
					continue;
				}

				child->Renderer.Update();

				if (GetIntersectionClipRect(containerRectangle, childRectangle, childRectangle))
				{
					rootGraphics.BitBlt(childRectangle, child->Renderer.GetGraphics(), { 0,0 });
				}
			}
			else
			{
				newContainerRectangle = childRectangle;
			}
			UpdateTreeInternal(child, rootGraphics, absolutePositionChild, newContainerRectangle);
		}
	}

	void WindowManager::PaintInternal(Window* window, Graphics& rootGraphics, bool doUpdate, const Point& parentPosition, const Rectangle& containerRectangle)
	{
		if (window == nullptr)
		{
			return;
		}

		Rectangle newContainerRectangle = containerRectangle;
		for (auto& child : window->Children)
		{
			if (!child->Visible)
			{
				continue;
			}

			auto childAbsolutePosition = GetLocalPosition(child);
			childAbsolutePosition += parentPosition;
			Rectangle childRectangle{ childAbsolutePosition.X, childAbsolutePosition.Y, child->Size.Width, child->Size.Height };
			if (child->Type != WindowType::Panel)
			{
				if (doUpdate && !child->Flags.isUpdating)
				{
					child->Flags.isUpdating = true;
					child->Renderer.Update();
					child->Flags.isUpdating = false;
				}

				if (GetIntersectionClipRect(containerRectangle, childRectangle, childRectangle))
				{
					rootGraphics.BitBlt(childRectangle, child->Renderer.GetGraphics(), { 0,0 });
				}
			}
			else
			{
				newContainerRectangle = childRectangle;
			}
			PaintInternal(child, rootGraphics, doUpdate, childAbsolutePosition, newContainerRectangle);
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

	void WindowManager::SetParentInternal(Window* window, Window* newParent, const Point& deltaPosition)
	{
		for (size_t i = 0; i < window->Children.size(); i++)
		{
			auto child = window->Children[i];

			if (child->Type == WindowType::Form)
			{
				auto nativePosition = API::GetWindowPosition(child->RootHandle);
				auto currentParent = API::GetParentWindow(child->RootHandle);
				if (currentParent != newParent->RootHandle)
				{
					API::SetParentWindow(child->RootHandle, newParent->RootHandle);
				}

				nativePosition -= deltaPosition;
				API::MoveWindow(child->RootHandle, nativePosition);
			}
			else
			{
				child->RootHandle = window->RootHandle;
				child->RootWindow = window->RootWindow;
				child->RootGraphics = window->RootGraphics;
			}

			SetParentInternal(child, newParent, deltaPosition);
		}
	}

	void WindowManager::MoveInternal(Window* window, const Point& delta, bool forceRepaint)
	{
		if (window->Type == WindowType::Form)
		{
			auto position = API::GetWindowPosition(window->RootHandle);
			API::MoveWindow(window->RootHandle, position + delta, forceRepaint);
		}

		for (size_t i = 0; i < window->Children.size(); i++)
		{
			MoveInternal(window->Children[i], delta, forceRepaint);
		}
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

		auto container = window->FindFirstPanelOrFormAncestor();
		auto containerPosition = GetAbsoluteRootPosition(container);
		Rectangle containerRectangle{ containerPosition.X, containerPosition.Y, container->Size.Width, container->Size.Height };
		if (GetIntersectionClipRect(containerRectangle, requestRectangle, requestRectangle))
		{
			rootGraphics.BitBlt(requestRectangle, window->Renderer.GetGraphics(), { 0,0 }); // Copy from control's graphics to root graphics.
		}

		PaintInternal(window, rootGraphics, doUpdate, absolutePosition, containerRectangle);
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

#if BT_DEBUG
			//BT_CORE_DEBUG << "    - Remove. Window =" << window->Name << std::endl;
#else
			//BT_CORE_DEBUG << "    - Remove." << std::endl;
#endif
			//delete window; //TODO: place this deallocation in a safe place!
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
		BT_CORE_TRACE << " - Capture / WindowPtr = " << m_capture.WindowPtr << ". window " << window << std::endl;
		if (m_capture.WindowPtr == window)
		{
			return;
		}

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

	void WindowManager::ReleaseCapture(Window* window)
	{
		BT_CORE_TRACE << " - ReleaseCapture / WindowPtr = " << m_capture.WindowPtr << ". window " << window << std::endl;
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

		auto container = window->FindFirstPanelOrFormAncestor();
		auto containerPosition = GetAbsoluteRootPosition(container);
		Rectangle containerRectangle{ containerPosition.X, containerPosition.Y, container->Size.Width, container->Size.Height };
		if (GetIntersectionClipRect(containerRectangle, requestRectangle, requestRectangle))
		{
			rootGraphics.BitBlt(requestRectangle, window->Renderer.GetGraphics(), { 0,0 }); // Copy from control's graphics to root graphics.
		}
		
		UpdateTreeInternal(window, rootGraphics, absolutePosition, containerRectangle);
	}

	void WindowManager::DoDeferredUpdate(Window* window)
	{
		if (!TryDeferredUpdate(window))
		{
#if BT_DEBUG
			BT_CORE_TRACE << " - WindowManager.DoDeferredUpdate() / paint and map... " << window->Name << ". Hwnd = " << window->RootHandle.Handle << std::endl;
#else
			BT_CORE_TRACE << " - WindowManager.DoDeferredUpdate() / paint and map... Hwnd = " << window->RootHandle.Handle << std::endl;
#endif
			Paint(window, false);
			Map(window, nullptr);
		}
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
		if (window->Visible == visible)
		{
			return;
		}

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
				Rectangle areaToUpdate{ position.X, position.Y, windowToUpdate->Size.Width, windowToUpdate->Size.Height };
				Map(windowToUpdate, &areaToUpdate);
			}
		}
	}

	void WindowManager::Resize(Window* window, const Size& newSize, bool resizeForm)
	{
		if (window->Size == newSize)
		{
			return;
		}
		auto& foundation = Foundation::GetInstance();

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
					auto nativePosition = API::GetWindowPosition(window->RootHandle);
					Rectangle newArea{ nativePosition.X, nativePosition.Y, window->Size.Width, window->Size.Height };
					API::MoveWindow(window->RootHandle, newArea);
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

	bool WindowManager::Move(Window* window, const Rectangle& newRect, bool forceRepaint)
	{
		auto& foundation = Foundation::GetInstance();

		bool positionChanged = false;
		bool sizeChanged = window->Size != newRect;

		if (window->Type == WindowType::Form)
		{
			Rectangle rootRect = newRect;
			if (window->Owner)
			{
				auto ownerPosition = GetAbsoluteRootPosition(window->Owner);
				rootRect.X += ownerPosition.X;
				rootRect.Y += ownerPosition.Y;
			}
			else if (window->Parent)
			{
				auto parentPosition = GetAbsoluteRootPosition(window->Parent);
				rootRect.X += parentPosition.X;
				rootRect.Y += parentPosition.Y;
			}

			if (sizeChanged)
			{
				window->Size = newRect;
				window->Renderer.GetGraphics().Rebuild(window->Size);
				window->RootGraphics->Rebuild(window->Size);

				API::MoveWindow(window->RootHandle, rootRect, forceRepaint);

				ArgResize argResize;
				argResize.NewSize = window->Size;
#if BT_DEBUG
				//BT_CORE_TRACE << "* Resize() - window = " << window->Name << ". newSize = " << newSize << std::endl;
#else
				//BT_CORE_TRACE << "* Resize(). newSize = "<< newSize<< std::endl;
#endif
				foundation.ProcessEvents(window, &Renderer::Resize, &ControlEvents::Resize, argResize);
			}
			else
			{
				API::MoveWindow(window->RootHandle, { rootRect.X, rootRect.Y }, forceRepaint);
			}
		}
		else
		{
			positionChanged = window->Position != newRect;
			Point delta{ newRect.X - window->Position.X, newRect.Y - window->Position.Y };

			if (positionChanged)
			{
				window->Position = newRect;
				MoveInternal(window, delta, forceRepaint);

				ArgMove argMove;
				argMove.NewPosition = newRect;
				foundation.ProcessEvents(window, &Renderer::Move, &ControlEvents::Move, argMove);
			}

			if (sizeChanged)
			{
				Resize(window, newRect);
			}
		}

		return sizeChanged || positionChanged;
	}

	bool WindowManager::Move(Window* window, const Point& newPosition, bool forceRepaint)
	{
		auto& foundation = Foundation::GetInstance();
		
		if (window->Type == WindowType::Form)
		{
			API::MoveWindow(window->RootHandle, newPosition, forceRepaint);
			return true;
		}
		else if (window->Position != newPosition)
		{
			Point delta{ newPosition.X - window->Position.X, newPosition.Y - window->Position.Y };
			window->Position = newPosition;

			MoveInternal(window, delta, forceRepaint);

			ArgMove argMove;
			argMove.NewPosition = newPosition;
			foundation.ProcessEvents(window, &Renderer::Move, &ControlEvents::Move, argMove);

			return true;
		}
		return false;
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

		DoDeferredUpdate(window);
	}

	bool WindowManager::TryDeferredUpdate(Window* window)
	{
		if (!window->Visible || !window->AreParentsVisible())
		{
			return true;
		}

		if (window->RootWindow->Flags.IsDeferredCount == 0)
		{
#if BT_DEBUG
			BT_CORE_TRACE << " -- TryDeferredUpdate / Paint and Map. window = " << window->Name << ". hwnd = " << window->RootHandle.Handle << std::endl;
#else
			BT_CORE_TRACE << " -- TryDeferredUpdate / Paint and Map. hwnd = " << window->RootHandle.Handle << std::endl;
#endif
			return false;
		}

		if (!window->RootWindow->HaveRequestedDeferred(window))
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
				Paint(request, false);
				Map(request, nullptr);

				//request->Status = WindowStatus::None;
			}
		}
	}

	void WindowManager::ChangeDPI(Window* window, uint32_t newDPI, const API::NativeWindowHandle& nativeWindowHandle)
	{
		if (window->DPI == newDPI)
		{
			return;
		}

		auto oldDPI = window->DPI;
		float scalingFactor = (float)newDPI / oldDPI;

		window->DPI = newDPI;
		window->DPIScaleFactor = LayoutUtils::CalculateDPIScaleFactor(newDPI);

		if (window->Type != WindowType::Form)
		{
			window->Position.X = static_cast<int>(window->Position.X * scalingFactor);
			window->Position.Y = static_cast<int>(window->Position.Y * scalingFactor);
		}
		window->Size.Width = static_cast<uint32_t>(window->Size.Width * scalingFactor);
		window->Size.Height = static_cast<uint32_t>(window->Size.Height * scalingFactor);

		auto& graphics = window->Renderer.GetGraphics();
		graphics.Release();
		graphics.Build(window->Size);
		graphics.BuildFont(newDPI);

		if (window->Type == WindowType::Form && window->RootHandle != nativeWindowHandle)
		{
			auto nativePosition = API::GetWindowPosition(window->RootHandle);
			nativePosition.X = static_cast<int>(nativePosition.X * scalingFactor);
			nativePosition.Y = static_cast<int>(nativePosition.Y * scalingFactor);

			Rectangle newArea{ nativePosition.X, nativePosition.Y, window->Size.Width, window->Size.Height };
			API::MoveWindow(window->RootHandle, newArea);
		}

		for (auto& child : window->Children)
		{
			ChangeDPI(child, newDPI, nativeWindowHandle);
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

	void WindowManager::SetParent(Window* window, Window* newParent)
	{
		if (window->Parent == newParent)
			return;

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

		auto deltaPosition = GUI::GetAbsoluteRootPosition(window) - GUI::GetAbsoluteRootPosition(newParent);

		window->Parent = newParent;
		if (window->Type != WindowType::Form)
		{
			window->RootHandle = newParent->RootHandle;
			window->RootWindow = newParent->RootWindow;
			window->RootGraphics = newParent->RootGraphics;
		}
		window->Position = { 0,0 };

		newParent->Children.emplace_back(window);

		if (window->Type == WindowType::Form)
		{
			auto nativePosition = API::GetWindowPosition(window->RootHandle); 
			auto currentParent = API::GetParentWindow(window->RootHandle);
			if (currentParent != newParent->RootHandle)
			{
				API::SetParentWindow(window->RootHandle, newParent->RootHandle);
			}

			nativePosition -= deltaPosition;
			API::MoveWindow(window->RootHandle, nativePosition);
		}
		SetParentInternal(window, newParent, deltaPosition);
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
					auto innerChild = FindInTree(child, point);
					if (innerChild)
					{
						return innerChild;
					}
				}
			} while (index != 0);
		}
		return window;
	}
}