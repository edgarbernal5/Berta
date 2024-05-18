/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_INPUT_TEXT_HEADER
#define BT_INPUT_TEXT_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include <string>

namespace Berta
{
	class InputTextRenderer : public ControlRenderer
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

	private:
		ControlBase* m_control;
	};

	class InputText : public Control<InputTextRenderer>
	{
	public:
		InputText(Window* parent, const Rectangle& rectangle);
	};
}

#endif