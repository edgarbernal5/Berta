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

	uint32_t Window::ToScale(uint32_t units) const
	{
		return static_cast<uint32_t>(units * DPIScaleFactor);
	}

	int Window::ToScale(int units) const
	{
		return static_cast<int>(units * DPIScaleFactor);
	}
}
