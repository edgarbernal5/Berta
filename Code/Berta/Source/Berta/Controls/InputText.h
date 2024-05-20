/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_INPUT_TEXT_HEADER
#define BT_INPUT_TEXT_HEADER

#include <string>
#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/TextEditors/TextEditor.h"

namespace Berta
{
	class InputTextReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void Focus(Graphics& graphics, const ArgFocus& args) override;

	private:
		ControlBase* m_control;
		TextEditor m_textEditor;
	};

	class InputText : public Control<InputTextReactor>
	{
	public:
		InputText(Window* parent, const Rectangle& rectangle);
	};
}

#endif