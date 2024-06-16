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

namespace Berta
{
	class TextEditor;

	class InputTextReactor : public ControlReactor
	{
	public:
		~InputTextReactor();

		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;
		
		void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void Focus(Graphics& graphics, const ArgFocus& args) override;
		void KeyChar(Graphics& graphics, const ArgKeyboard& args) override;
		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;
		void KeyReleased(Graphics& graphics, const ArgKeyboard& args) override;
		void DblClick(Graphics& graphics, const ArgClick& args);

		TextEditor* GetEditor() const { return m_textEditor; }
	private:
		ControlBase* m_control{ nullptr };
		TextEditor* m_textEditor{ nullptr };
	};

	class InputText : public Control<InputTextReactor, InputTextEvents>
	{
	public:
		InputText(Window* parent, const Rectangle& rectangle);

	protected:
		void DoOnCaption(const std::wstring& caption) override;
		std::wstring DoOnCaption() override;
	};
}

#endif