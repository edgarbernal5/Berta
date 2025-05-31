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

#ifdef BT_PLATFORM_WINDOWS
#include "Berta/Platform/Windows/D2D.h"
#include <comdef.h>
#endif

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
		RootGraphics(size, window->DPI, window->RootBufferHandle)
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
		if (window->IsNative())
		{
			API::CaptionNativeWindow(window->RootHandle, caption);
		}

		return true;
	}

	Window* WindowManager::CreateForm(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, const FormStyle& formStyle, bool isNested, ControlBase* control, bool isRenderForm)
	{
		API::NativeWindowHandle parentHandle{};
		if (parent)
		{
			parentHandle = parent->RootWindow->RootHandle;
		}

		Rectangle finalRect{ rectangle };
		if (isUnscaleRect && parent && parent->DPI != BT_APPLICATION_DPI)
		{
			float scalingFactor = LayoutUtils::CalculateDPIScaleFactor(parent->DPI);
			finalRect.X = static_cast<int>(finalRect.X * scalingFactor);
			finalRect.Y = static_cast<int>(finalRect.Y * scalingFactor);
			finalRect.Width = static_cast<uint32_t>(finalRect.Width * scalingFactor);
			finalRect.Height = static_cast<uint32_t>(finalRect.Height * scalingFactor);
		}

		auto windowResult = API::CreateNativeWindow(parentHandle, finalRect, formStyle, isNested);
		if (windowResult.WindowHandle)
		{
			Window* window = new Window(isRenderForm ? WindowType::RenderForm : WindowType::Form);
			window->Init(control);

			window->RootHandle = windowResult.WindowHandle;
			window->ClientSize = windowResult.ClientSize;
			window->BorderSize = windowResult.BorderSize;
			window->RootWindow = window;
			window->DPI = windowResult.DPI;
			window->DPIScaleFactor = LayoutUtils::CalculateDPIScaleFactor(windowResult.DPI);

#ifdef BT_PLATFORM_WINDOWS
			auto rtProps = D2D1::RenderTargetProperties();
			rtProps.dpiX = BT_APPLICATION_DPI;
			rtProps.dpiY = BT_APPLICATION_DPI;

			D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat
			(
				DXGI_FORMAT_B8G8R8A8_UNORM,
				D2D1_ALPHA_MODE_PREMULTIPLIED
			);
			rtProps.pixelFormat = pixelFormat;

			auto hr = DirectX::D2DModule::GetInstance().GetFactory()->CreateHwndRenderTarget
			(
				rtProps,
				D2D1::HwndRenderTargetProperties(windowResult.WindowHandle.Handle,
					D2D1::SizeU(windowResult.ClientSize.Width, windowResult.ClientSize.Height)),
				&window->RootBufferHandle.m_renderTarget
			);

			if (FAILED(hr))
			{
				_com_error err(hr);
				BT_CORE_ERROR << "Error creating render target hwnd. err.ErrorMessage() = " << StringUtils::Convert(err.ErrorMessage()) << std::endl;
			}
#endif
			if (isNested)
			{
				window->Parent = parent;
				window->Position = finalRect;
				window->Owner = nullptr;

				if (parent)
				{
					parent->Children.emplace_back(window);
				}
			}
			else
			{
				window->Owner = parent;
				window->Parent = nullptr;
			}

			AddNative(windowResult.WindowHandle, WindowManager::FormData(window, window->ClientSize));
			Add(window);

			auto& rootGraphics = GetFormData(windowResult.WindowHandle)->RootGraphics;
			window->RootGraphics = &rootGraphics;

			return window;
		}

		return nullptr;
	}

	Window* WindowManager::CreateControl(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, ControlBase* control, bool isPanel)
	{
		Window* window = new Window(isPanel ? WindowType::Panel : WindowType::Control);
		window->Init(control);

		Rectangle finalRect{ rectangle };
		if (isUnscaleRect && parent && parent->DPI != BT_APPLICATION_DPI)
		{
			float scalingFactor = LayoutUtils::CalculateDPIScaleFactor(parent->DPI);
			finalRect.X = static_cast<int>(finalRect.X * scalingFactor);
			finalRect.Y = static_cast<int>(finalRect.Y * scalingFactor);
			finalRect.Width = static_cast<uint32_t>(finalRect.Width * scalingFactor);
			finalRect.Height = static_cast<uint32_t>(finalRect.Height * scalingFactor);
		}
		window->ClientSize = finalRect;
		window->Parent = parent;
		window->Position = finalRect;

		if (parent)
		{
			window->DPI = parent->DPI;
			window->DPIScaleFactor = parent->DPIScaleFactor;
			window->RootWindow = parent->RootWindow;
			window->RootHandle = parent->RootHandle;
			window->RootBufferHandle = parent->RootBufferHandle;
			window->RootGraphics = parent->RootGraphics;

			parent->Children.emplace_back(window);
		}

		Add(window);
		return window;
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

		if (!window->IsNative())
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
			if (child->IsNative())
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
		if (!window->IsNative())
		{
			m_windowRegistry.erase(window);
		}

		window->Renderer.GetGraphics().Release();
		if (window->IsNative())
		{
			API::Dispose(window->RootBufferHandle);
		}
	}

	void WindowManager::UpdateTreeInternal(Window* window, Graphics& rootGraphics, bool now, const Point& parentPosition, const Rectangle& containerRectangle)
	{
		if (window == nullptr)
		{
			return;
		}

		for (auto& child : window->Children)
		{
			if (!child->Visible)
			{
				continue;
			}

			Rectangle newContainerRectangle = containerRectangle;
			auto absolutePositionChild = GetLocalPosition(child);
			absolutePositionChild += parentPosition;

			Rectangle childRectangle{ absolutePositionChild.X, absolutePositionChild.Y, child->ClientSize.Width, child->ClientSize.Height };
			
			if (child->Type != WindowType::Panel)
			{
				if (child->Type == WindowType::Form)
				{
					Paint(child, true);
					continue;
				}

				if (now || !child->IsBatchActive())
				{
					child->Renderer.Update();
					child->DrawStatus = DrawWindowStatus::Updated;

					if (LayoutUtils::GetIntersectionClipRect(containerRectangle, childRectangle, childRectangle))
					{
						rootGraphics.BitBlt(childRectangle, child->Renderer.GetGraphics(), { 0,0 });
					}
				}
				else
				{
					if (LayoutUtils::GetIntersectionClipRect(containerRectangle, childRectangle, childRectangle))
					{
						AddWindowToBatch(child, childRectangle, DrawOperation::NeedUpdate | DrawOperation::NeedMap);
					}
				}
			}
			else
			{
				newContainerRectangle = childRectangle;
			}

			UpdateTreeInternal(child, rootGraphics, now, absolutePositionChild, newContainerRectangle);
		}
	}

	void WindowManager::PaintInternal(Window* window, Graphics& rootGraphics, bool doUpdate, const Point& parentPosition, const Rectangle& containerRectangle)
	{
		if (window == nullptr)
		{
			return;
		}

		for (auto& child : window->Children)
		{
			if (!child->Visible)
			{
				continue;
			}

			Rectangle newContainerRectangle = containerRectangle;
			auto childAbsolutePosition = GetLocalPosition(child);
			childAbsolutePosition += parentPosition;

			Rectangle childRectangle{ childAbsolutePosition.X, childAbsolutePosition.Y, child->ClientSize.Width, child->ClientSize.Height };
			if (child->Type != WindowType::Panel)
			{
				if (doUpdate && !child->Flags.isUpdating)
				{
					child->Flags.isUpdating = true;
					child->Renderer.Update();
					child->Flags.isUpdating = false;
				}

				if (LayoutUtils::GetIntersectionClipRect(containerRectangle, childRectangle, childRectangle))
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

	void WindowManager::SetParentInternal(Window* window, Window* newParent, const Point& deltaPosition)
	{
		for (size_t i = 0; i < window->Children.size(); i++)
		{
			auto child = window->Children[i];

			if (child->IsNative())
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

				if (child->RootBufferHandle != window->RootBufferHandle)
				{
					child->RootBufferHandle = window->RootBufferHandle;
					auto& graphics = child->Renderer.GetGraphics();
					graphics.Rebuild(child->ClientSize, window->RootBufferHandle);
					graphics.BuildFont(child->DPI);
				}
			}

			SetParentInternal(child, newParent, deltaPosition);
		}
	}

	void WindowManager::MoveInternal(Window* window, const Point& delta, bool forceRepaint)
	{
		if (window->IsNative())
		{
			auto nativePosition = API::GetWindowPosition(window->RootHandle);
			API::MoveWindow(window->RootHandle, nativePosition + delta, forceRepaint);
			if (!forceRepaint)
			{
				auto batch = window->IsBatchActive() ? window->Batcher : window->Parent->RootWindow->Batcher;

				if (batch)
				{
					AddWindowToBatch(batch, window, {}, DrawOperation::Refresh);
				}
			}
		}

		for (size_t i = 0; i < window->Children.size(); i++)
		{
			MoveInternal(window->Children[i], delta, forceRepaint);
		}
	}

	void WindowManager::ShowInternal(Window* window, bool visible)
	{
		if (window->IsNative())
		{
			if (visible != window->Visible)
			{
				API::ShowNativeWindow(window->RootHandle, visible, window->Flags.MakeActive);
			}
		}

		for (size_t i = 0; i < window->Children.size(); i++)
		{
			ShowInternal(window->Children[i], visible);
		}
	}

	void WindowManager::AddWindowToBatch(Window* window, const Rectangle& areaToUpdate, const DrawOperation& operation)
	{
		if (!window->RootWindow->Batcher)
			return;

		AddWindowToBatch(window->RootWindow->Batcher, window, areaToUpdate, operation);
	}

	void WindowManager::AddWindowToBatch(DrawBatch* batch, Window* window, const Rectangle& areaToUpdate, const DrawOperation& operation)
	{
		if (batch->Exists(window, areaToUpdate, operation))
			return;

		batch->AddWindow(window, areaToUpdate, operation);
	}

	void WindowManager::TryAddWindowToBatchInternal(Window* window, const Rectangle& containerRectangle, const Point& parentPosition, const DrawOperation& operation)
	{
		if (window == nullptr)
		{
			return;
		}

		for (auto& child : window->Children)
		{
			if (!child->Visible)
			{
				continue;
			}

			Rectangle newContainerRectangle = containerRectangle;
			auto childAbsolutePosition = GetLocalPosition(child);
			childAbsolutePosition += parentPosition;

			Rectangle childRectangle{ childAbsolutePosition.X, childAbsolutePosition.Y, child->ClientSize.Width, child->ClientSize.Height };
			if (child->Type != WindowType::Panel)
			{
				if (HasFlag(operation, DrawOperation::NeedMap) && LayoutUtils::GetIntersectionClipRect(containerRectangle, childRectangle, childRectangle))
				{
					AddWindowToBatch(child, childRectangle, DrawOperation::NeedMap);
				}
			}
			else
			{
				newContainerRectangle = childRectangle;
			}
			TryAddWindowToBatchInternal(child, newContainerRectangle, childAbsolutePosition, operation);
		}
	}

	void WindowManager::TryAddWindowToBatch(Window* window, const DrawOperation& operation)
	{
		Rectangle requestRectangle = window->ClientSize.ToRectangle();
		auto absolutePosition = GetAbsoluteRootPosition(window);
		requestRectangle.X = absolutePosition.X;
		requestRectangle.Y = absolutePosition.Y;

		auto container = window->FindFirstPanelOrFormAncestor();
		auto containerPosition = GetAbsoluteRootPosition(container);
		Rectangle containerRectangle{ containerPosition.X, containerPosition.Y, container->ClientSize.Width, container->ClientSize.Height };
		if (LayoutUtils::GetIntersectionClipRect(containerRectangle, requestRectangle, requestRectangle))
		{
			AddWindowToBatch(window, requestRectangle, operation);
			if (!window->Children.empty())
			{
				TryAddWindowToBatchInternal(window, requestRectangle, requestRectangle, operation);
			}
		}
	}

	void WindowManager::GetNativeWindows(std::vector<API::NativeWindowHandle>& windowHandles)
	{
		windowHandles.clear();
		for (auto& item : m_windowNativeRegistry)
		{
			windowHandles.push_back(item.first);
		}
	}

	bool WindowManager::GetIntersectionClipRect(Window* window, Rectangle& result)
	{
		Rectangle requestRectangle = window->ClientSize.ToRectangle();
		auto absolutePosition = GetAbsoluteRootPosition(window);
		requestRectangle.X = absolutePosition.X;
		requestRectangle.Y = absolutePosition.Y;

		auto container = window->FindFirstPanelOrFormAncestor();
		auto containerPosition = GetAbsoluteRootPosition(container);
		Rectangle containerRectangle{ containerPosition.X, containerPosition.Y, container->ClientSize.Width, container->ClientSize.Height };
		
		return LayoutUtils::GetIntersectionClipRect(containerRectangle, requestRectangle, result);
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
		Rectangle requestRectangle{ absolutePosition.X, absolutePosition.Y, window->ClientSize.Width, window->ClientSize.Height };

		auto container = window->FindFirstPanelOrFormAncestor();
		auto containerPosition = GetAbsoluteRootPosition(container);
		Rectangle containerRectangle{ containerPosition.X, containerPosition.Y, container->ClientSize.Width, container->ClientSize.Height };
		rootGraphics.Begin();
		if (LayoutUtils::GetIntersectionClipRect(containerRectangle, requestRectangle, requestRectangle))
		{
			rootGraphics.BitBlt(requestRectangle, window->Renderer.GetGraphics(), { 0,0 });
		}

		PaintInternal(window, rootGraphics, doUpdate, absolutePosition, containerRectangle);
		rootGraphics.Flush();
	}

	void WindowManager::Dispose(Window* window)
	{
		if (window->Flags.IsDisposed)
		{
			return;
		}

		if (window->IsNative())
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
		if (!window->IsNative())
		{
			return;
		}

		m_windowNativeRegistry.erase(window->RootHandle);
		m_windowRegistry.erase(window);
#if BT_DEBUG
		//BT_CORE_DEBUG << "    - Remove. Window =" << window->Name << std::endl;
#else
		//BT_CORE_DEBUG << "    - Remove." << std::endl;
#endif
		//delete window; //TODO: place this deallocation in a safe place!
		
	}

	void WindowManager::Refresh(Window* window)
	{
		API::RefreshWindow(window->RootHandle);
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

	void WindowManager::UpdateTree(Window* window, bool now)
	{
		if (!window->IsVisible())
		{
			return;
		}

		auto& rootGraphics = *(window->RootGraphics);

		Rectangle requestRectangle = window->ClientSize.ToRectangle();
		auto absolutePosition = GetAbsoluteRootPosition(window);
		requestRectangle.X = absolutePosition.X;
		requestRectangle.Y = absolutePosition.Y;

		auto container = window->FindFirstPanelOrFormAncestor();
		auto containerPosition = GetAbsoluteRootPosition(container);
		Rectangle containerRectangle{ containerPosition.X, containerPosition.Y, container->ClientSize.Width, container->ClientSize.Height };

		if (now || !window->IsBatchActive())
		{
			window->Renderer.Update();
			window->DrawStatus = DrawWindowStatus::Updated;
			rootGraphics.Begin();
			if (LayoutUtils::GetIntersectionClipRect(containerRectangle, requestRectangle, requestRectangle))
			{
				rootGraphics.BitBlt(requestRectangle, window->Renderer.GetGraphics(), { 0,0 });
			}
		}
		else
		{
			if (LayoutUtils::GetIntersectionClipRect(containerRectangle, requestRectangle, requestRectangle))
			{
				AddWindowToBatch(window, requestRectangle, DrawOperation::NeedUpdate | DrawOperation::NeedMap);
			}
		}

		UpdateTreeInternal(window, rootGraphics, now, absolutePosition, containerRectangle);

		if (now || !window->IsBatchActive())
		{
			rootGraphics.Flush();
		}
	}

	void WindowManager::Map(Window* window, const Rectangle* areaToUpdate)
	{
		if (areaToUpdate == nullptr)
		{
			Rectangle requestRectangle = window->ClientSize.ToRectangle();
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

		if (window->IsNative())
		{
			API::ShowNativeWindow(window->RootHandle, visible, window->Flags.MakeActive);
		}
		else
		{
			window->Visible = visible;

			ShowInternal(window, visible);

			ArgVisibility argVisibility;
			argVisibility.IsVisible = visible;
			window->Events->Visibility.Emit(argVisibility);

			auto windowToUpdate = window->FindFirstNonPanelAncestor();
			if (windowToUpdate)
			{
				UpdateTree(windowToUpdate);
				if (!windowToUpdate->IsBatchActive())
				{
					auto position = GetAbsoluteRootPosition(windowToUpdate);
					Rectangle areaToUpdate{ position.X, position.Y, windowToUpdate->ClientSize.Width, windowToUpdate->ClientSize.Height };
					Map(windowToUpdate, &areaToUpdate);
				}
			}
		}
	}

	bool WindowManager::Resize(Window* window, const Size& newSize, bool resizeForm)
	{
		if (window->ClientSize == newSize)
		{
			return false;
		}
		auto& foundation = Foundation::GetInstance();

		window->ClientSize = newSize;

#ifdef BT_PLATFORM_WINDOWS
		if (window->Type == WindowType::Form)
		{
			auto hr = window->RootBufferHandle.m_renderTarget->Resize(D2D1::SizeU(newSize.Width, newSize.Height));
			if (FAILED(hr))
			{
				BT_CORE_ERROR << "error> resize hwnd render target." << std::endl;
			}
		}
#endif

		Graphics newGraphics;
		Graphics newRootGraphics;
		if (window->Type != WindowType::Panel && window->Type != WindowType::RenderForm)
		{
			newGraphics.Build(newSize, window->RootBufferHandle);
			newGraphics.BuildFont(window->DPI);
			//newGraphics.DrawRectangle(window->ClientSize.ToRectangle(), window->Appearance->Background, true);

			if (window->Type == WindowType::Form)
			{
				newRootGraphics.Build(newSize, window->RootBufferHandle);
				newRootGraphics.BuildFont(window->DPI);
				newRootGraphics.Begin();
				newRootGraphics.DrawRectangle(window->ClientSize.ToRectangle(), window->Appearance->Background, true); //TODO: not sure if we have to call this here.
				newRootGraphics.Flush();
			}
		}

		if (window->Type != WindowType::Panel && window->Type != WindowType::RenderForm)
		{
			window->Renderer.GetGraphics().Swap(newGraphics);

			if (window->Type == WindowType::Form)
			{
				window->RootGraphics->Swap(newRootGraphics);

				if (resizeForm)
				{
					auto nativePosition = API::GetWindowPosition(window->RootHandle);
					Rectangle newArea{ nativePosition.X, nativePosition.Y, window->ClientSize.Width, window->ClientSize.Height };
					API::MoveWindow(window->RootHandle, newArea);
				}
			}
		}

		ArgResize argResize;
		argResize.NewSize = newSize;
		foundation.ProcessEvents(window, &Renderer::Resize, &ControlEvents::Resize, argResize);

		return true;
	}

	bool WindowManager::Move(Window* window, const Rectangle& newRect, bool forceRepaint)
	{
		auto& foundation = Foundation::GetInstance();

		bool positionChanged = false;
		bool sizeChanged = window->ClientSize != newRect;

		if (window->IsNative())
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
				window->ClientSize = newRect;

#ifdef BT_PLATFORM_WINDOWS
				auto hr = window->RootBufferHandle.m_renderTarget->Resize(D2D1::SizeU(window->ClientSize.Width, window->ClientSize.Height));
				if (FAILED(hr))
				{
					BT_CORE_ERROR << "error> resize hwnd render target." << std::endl;
				}
#endif

				window->Renderer.GetGraphics().Rebuild(window->ClientSize, window->RootBufferHandle);
				window->RootGraphics->Rebuild(window->ClientSize, window->RootBufferHandle);

				API::MoveWindow(window->RootHandle, rootRect, forceRepaint);

				ArgResize argResize;
				argResize.NewSize = window->ClientSize;
				foundation.ProcessEvents(window, &Renderer::Resize, &ControlEvents::Resize, argResize);
			}
			else
			{
				auto nativePosition = API::GetWindowPosition(window->RootHandle);
				Point newNativePosition = { rootRect.X, rootRect.Y };
				API::MoveWindow(window->RootHandle, newNativePosition, forceRepaint);
				positionChanged = newNativePosition != nativePosition;
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
		
		if (window->IsNative())
		{
			auto nativePosition = API::GetWindowPosition(window->RootHandle);
			API::MoveWindow(window->RootHandle, newPosition, forceRepaint);
			bool positionChanged = newPosition != nativePosition;

			if (!forceRepaint)
			{
				//API::RefreshWindow(window->RootHandle);
			}
			return positionChanged;
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
		if (!window->IsVisible())
			return;

		if (window->IsBatchActive())
		{
			TryAddWindowToBatch(window);
		}
		else
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

			Paint(window, false);
			Map(window, nullptr);
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

		if (window->IsNative())
		{
			window->BorderSize.Width = static_cast<uint32_t>(window->BorderSize.Width * scalingFactor);
			window->BorderSize.Height = static_cast<uint32_t>(window->BorderSize.Height * scalingFactor);
		}
		else
		{
			window->Position.X = static_cast<int>(window->Position.X * scalingFactor);
			window->Position.Y = static_cast<int>(window->Position.Y * scalingFactor);
		}
		window->ClientSize.Width = static_cast<uint32_t>(window->ClientSize.Width * scalingFactor);
		window->ClientSize.Height = static_cast<uint32_t>(window->ClientSize.Height * scalingFactor);

		if (window->Type != WindowType::RenderForm)
		{
			auto& graphics = window->Renderer.GetGraphics();
			graphics.Release();
			graphics.Build(window->ClientSize, window->RootBufferHandle);
			graphics.BuildFont(newDPI);
		}

		if (window->IsNative() && window->RootHandle != nativeWindowHandle)
		{
			auto nativePosition = API::GetWindowPosition(window->RootHandle);
			nativePosition.X = static_cast<int>(nativePosition.X * scalingFactor);
			nativePosition.Y = static_cast<int>(nativePosition.Y * scalingFactor);

			Rectangle newArea{ nativePosition.X, nativePosition.Y, window->ClientSize.Width, window->ClientSize.Height };
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
		while (window && !window->IsNative())
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
		if (!window->IsNative())
		{
			window->RootHandle = newParent->RootHandle;
			window->RootWindow = newParent->RootWindow;
			window->RootGraphics = newParent->RootGraphics;

			if (window->RootBufferHandle != newParent->RootBufferHandle)
			{
				window->RootBufferHandle = newParent->RootBufferHandle;
				auto& graphics = window->Renderer.GetGraphics();
				graphics.Rebuild(window->ClientSize, newParent->RootBufferHandle);
				graphics.BuildFont(window->DPI);
			}
		}
		window->Position = { 0,0 };

		newParent->Children.emplace_back(window);

		if (window->IsNative())
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
			window->ClientSize
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
				if (!child->IsNative() && IsPointOnWindow(child, point))
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