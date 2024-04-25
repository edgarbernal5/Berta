/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FORM_HEADER
#define BT_FORM_HEADER

#include <Windows.h>
#include <string>

#include "Berta/EntryPoint.h"
#include "BasicTypes.h"
#include "Widget.h"

namespace Berta
{
	class Form : public Widget
	{
	public:
		Form(const Rectangle& rectangle = { 0,0,800,600 });

	private:
		const wchar_t* ApplicationClassName = L"BertaInternalClass";

		void Create(const Rectangle& rectangle);
		static LRESULT CALLBACK WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);
		
		HWND m_hwnd;
		HINSTANCE m_hModuleInstance;
	};
}

#endif