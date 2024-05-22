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
#include "Berta/GUI/Control.h"

namespace Berta
{
	class ControlBase;

	class FormReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

	private:
		ControlBase* m_control{ nullptr };
	};

	class Form : public Control<FormReactor, RootEvents>
	{
	public:
		Form(const Rectangle& rectangle = { 0,0,800,600 }, const FormStyle& windowStyle = {true, true, true});

		void Exec();
	private:
	};
}

#endif