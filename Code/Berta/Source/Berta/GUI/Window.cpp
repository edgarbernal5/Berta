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
}
