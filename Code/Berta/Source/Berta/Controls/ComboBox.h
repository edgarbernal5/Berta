/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_COMBO_BOX_HEADER
#define BT_COMBO_BOX_HEADER

#include <string>
#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"

namespace Berta
{
	class TextEditor;
	class FloatBox;

	class ComboBoxReactor : public ControlReactor
	{
	public:
		~ComboBoxReactor();

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

		TextEditor* GetEditor() const { return m_textEditor; }

		friend class ComboBox;
	private:
		ControlBase* m_control{ nullptr };
		TextEditor* m_textEditor{ nullptr };

		std::vector<std::wstring> m_items;
		FloatBox* m_floatBox{ nullptr };
	};

	class ComboBox : public Control<ComboBoxReactor>
	{
	public:
		ComboBox(Window* parent, const Rectangle& rectangle);

		void PushItem(const std::wstring& text);

	protected:
		void DoOnCaption(const std::wstring& caption) override;
		std::wstring DoOnCaption() override;

	private:
		bool m_isEditable{ false };
	};
}

#endif