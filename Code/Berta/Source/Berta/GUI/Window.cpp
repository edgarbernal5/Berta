/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Window.h"

namespace Berta
{
	Window::~Window()
	{
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
}
