/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Window.h"

#include "Berta/GUI/Control.h"

namespace Berta
{
	void Window::Init(ControlBase* control)
	{
		ControlWindowPtr = std::make_unique<ControlBase::ControlWindow>(*control);

		Flags.IsEnabled = true;
		Flags.IsDisposed = false;
		Flags.MakeActive = true;
		Flags.isUpdating = false;
		Flags.IgnoreMouseFocus = false;
	}

	Window::~Window()
	{
		if (RootWindow && RootWindow->HaveRequestedDeferred(this))
		{
			RootWindow->DeleteDeferredRequest(this);
		}
		DeferredRequests.clear();
	}

	Window* Window::FindFirstNonPanelAncestor() const
	{
		if (!Parent)
		{
			return const_cast<Window*>(this);
		}

		auto windowToUpdate = this->Parent;
		while (windowToUpdate && windowToUpdate->Type == WindowType::Panel)
		{
			windowToUpdate = windowToUpdate->Parent;
		}
		
		return windowToUpdate;
	}

	Window* Window::FindFirstPanelOrFormAncestor() const
	{
		if (!Parent)
		{
			return const_cast<Window*>(this);
		}

		auto windowToUpdate = const_cast<Window*>(this);
		while (windowToUpdate && windowToUpdate->Type != WindowType::Panel && windowToUpdate->Type != WindowType::Form)
		{
			windowToUpdate = windowToUpdate->Parent;
		}

		return windowToUpdate == nullptr ? this->RootWindow : windowToUpdate;
	}

	bool Window::AreParentsVisible() const
	{
		auto current = Parent;
		while (current)
		{
			if (!current->Visible)
			{
				return false;
			}
			current = current->Parent;
		}
		return true;
	}

	bool Window::IsVisible() const
	{
		return Visible && AreParentsVisible();
	}

	bool Window::IsAncestorOf(Window* window) const
	{
		auto current = window;
		while (current)
		{
			if (current == this)
				return true;

			current = current->Parent;
		}
		return false;
	}

	void Window::DeleteDeferredRequest(Window* window)
	{
		auto it = std::find(DeferredRequests.begin(), DeferredRequests.end(), window);
		DeferredRequests.erase(it);
	}

	bool Window::HaveRequestedDeferred(Window* window) const
	{
		return std::find(DeferredRequests.begin(), DeferredRequests.end(), window) != DeferredRequests.end();
	}
}
