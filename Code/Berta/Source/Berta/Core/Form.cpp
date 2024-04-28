/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Form.h"

#include "Berta/Native/API.h"

namespace Berta
{
	Form::Form(const Rectangle& rectangle)
	{
		Create(rectangle);
	}

	void Form::Create(const Rectangle& rectangle)
	{
		auto nativeHandle = API::Create_Window(rectangle);

		//TODO: HACK
		ShowWindow(nativeHandle.Handle, SW_SHOW);
		//g_hModuleInstance = hInstance;
		MSG msg = { 0 };

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}