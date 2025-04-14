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

		BorderSize = { 0,0 };
	}

	Window::~Window()
	{
	}

	bool Window::IsBatchActive() const
	{
		if (!RootWindow)
			return false;

		return RootWindow->Batcher != nullptr;
	}

	Window* Window::FindFirstNonPanelAncestor() const
	{
		if (!Parent)
		{
			return const_cast<Window*>(this);
		}

		auto window = this->Parent;
		while (window && window->Type == WindowType::Panel)
		{
			window = window->Parent;
		}
		
		return window;
	}

	Window* Window::FindFirstPanelOrFormAncestor() const
	{
		auto window = const_cast<Window*>(this);
		if (!Parent)
		{
			return window;
		}

		while (window && window->Type != WindowType::Panel && window->Type != WindowType::Form)
		{
			window = window->Parent;
		}

		return window == nullptr ? this->RootWindow : window;
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

	int Window::GetHierarchyIndex() const
	{
		bool found = false;
		return GetHierarchyIndexInternal(RootWindow, const_cast<Window*>(this), found);
	}

	int Window::GetHierarchyIndexInternal(Window* current, Window* target, bool& found) const
	{
		if (!current)
			return 0;

		if (current == target)
		{
			found = true;
			return 0;
		}

		int index = 0;
		for (size_t i = 0; i < current->Children.size(); i++)
		{
			auto child = current->Children[i];

			index += 1 + GetHierarchyIndexInternal(child, target, found);

			if (found)
				return index;
		}

		return index;
	}
}
