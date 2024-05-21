/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "WindowManager.h"

#include "Berta/GUI/Window.h"

namespace Berta
{
	WindowManager::RootData::RootData(RootData&& other) noexcept :
		WindowPtr(other.WindowPtr),
		RootGraphics(std::move(other.RootGraphics))
	{
	}

	WindowManager::RootData::RootData(Window* window, const Size& size) :
		WindowPtr(window),
		RootGraphics(size)
	{
	}

	void WindowManager::Add(Window* window)
	{
		m_windowRegistry.emplace(window);
	}

	void WindowManager::AddNative(API::NativeWindowHandle nativeWindowHandle, RootData&& append)
	{
		m_windowNativeRegistry.emplace(nativeWindowHandle, std::move(append));
	}

	void WindowManager::Caption(Window* window, const std::wstring& caption)
	{
		window->Title = caption;
		if (window->Type == WindowType::Native)
		{
			API::CaptionNativeWindow(window->RootHandle, caption);
		}
	}

	void WindowManager::Dispose(Window* window)
	{
		if (window->Type == WindowType::Native)
		{
			API::DestroyNativeWindow(window->RootHandle);
			m_windowNativeRegistry.erase(window->RootHandle);
		}
		m_windowRegistry.erase(window);
	}

	Window* WindowManager::Get(API::NativeWindowHandle nativeWindowHandle)
	{
		auto it = m_windowNativeRegistry.find(nativeWindowHandle);
		if (it != m_windowNativeRegistry.end())
		{
			return it->second.WindowPtr;
		}

		return nullptr;
	}

	Berta::WindowManager::RootData* WindowManager::GetWindowData(API::NativeWindowHandle nativeWindowHandle)
	{
		auto it = m_windowNativeRegistry.find(nativeWindowHandle);
		if (it != m_windowNativeRegistry.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	bool WindowManager::Exists(Window* window)
	{
		return m_windowRegistry.find(window) != m_windowRegistry.end();
	}

	Window* WindowManager::Find(Window* window, const Point& point)
	{
		if (window == nullptr || !window->Visible)
		{
			return nullptr;
		}

		if (IsPointOnWindow(window, point))
		{
			return FindInTree(window, point);
		}
		return nullptr;
	}

	void WindowManager::UpdateTree(Window* window)
	{
		window->Renderer.Update(); //Update control's window.
		auto& rootGraphics = *(window->RootGraphics);
		rootGraphics.BitBlt(window->Size.ToRectangle(), window->Renderer.GetGraphics(), { 0,0 }); // Copy from root graphics to control's graphics.
		
		//TODO: traverse the entire tree.
		for (auto& child : window->Children)
		{
			if (!child->Visible)
				continue;

			child->Renderer.Update();
			Rectangle childRectangle{ child->Position.X, child->Position.Y, child->Size.Width, child->Size.Height };
			rootGraphics.BitBlt(childRectangle, child->Renderer.GetGraphics(), { 0,0 });
		}

		window->Renderer.Map(window, window->Size.ToRectangle()); // Copy from root graphics to native hwnd window.
	}

	void WindowManager::Show(Window* window, bool visible)
	{
		if (window->Visible != visible)
		{
			if (window->Type == WindowType::Native)
			{
				API::ShowNativeWindow(window->RootHandle, visible);
			}

			window->Visible = visible;
		}
	}

	void WindowManager::Resize(Window* window, const Size& newSize)
	{
		if (window->Size != newSize)
		{
			window->Size = newSize;

			Graphics newGraphics;
			Graphics newRootGraphics;
			newGraphics.Build(newSize);
			if (window->Type == WindowType::Native)
			{
				newRootGraphics.Build(newSize);
			}

			window->Renderer.GetGraphics().Swap(newGraphics);

			if (window->Type == WindowType::Native)
			{
				window->RootGraphics->Swap(newRootGraphics);
				UpdateTree(window);
			}
		}
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
				Rectangle requestRectangle{ request->Position.X, request->Position.Y, request->Size.Width, request->Size.Height };

				rootGraphics.BitBlt(requestRectangle, request->Renderer.GetGraphics(), { 0,0 }); // Copy from root graphics to control's graphics.

				rootWindow->Renderer.Map(rootWindow, requestRectangle); // Copy from root graphics to native hwnd window.
			}
		}

		rootWindow->DeferredRequests.clear();
	}

	void WindowManager::ChangeDPI(Window* window, uint32_t newDPI)
	{
		if (window->DPI != newDPI)
		{
			window->DPI = newDPI;
			window->Renderer.GetGraphics().BuildFont(newDPI);

			for (auto& child : window->Children)
			{
				ChangeDPI(child, newDPI);
			}
		}
	}

	void WindowManager::ChangeCursor(Window* window, Cursor newCursor)
	{
	}

	bool WindowManager::IsPointOnWindow(Window* window, const Point& point)
	{
		return Rectangle{ window->Position.X, window->Position.Y, window->Size.Width, window->Size.Height}.IsInside(point);
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
				if (child->Type != WindowType::Native && IsPointOnWindow(child, point))
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