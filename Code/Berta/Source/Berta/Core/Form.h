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
#include "Berta/Core/BasicTypes.h"
#include "Berta/Core/Widget.h"

namespace Berta
{
	class FormRenderer : public WidgetRenderer
	{
	public:
		void Update() override {}
	};

	class Form : public Widget<FormRenderer>
	{
	public:
		Form(const Rectangle& rectangle = { 0,0,800,600 }, const WindowStyle& windowStyle = {true, true, true});

		void Exec();
	private:
	};
}

#endif