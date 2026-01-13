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
#include "Berta/Controls/TextEditors/TextEditorBase.h"
#include "Berta/Controls/TextEditors/TextEditor.h"

namespace Berta
{
	namespace ReactorCore::InputText
	{
		struct Events;

		class Reactor : public ControlReactor
		{
		public:
			void Init(ControlBase& control, Graphics* graphics) override;
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
			void DblClick(Graphics& graphics, const ArgMouse& args) override;
			void Resize(Graphics& graphics, const ArgResize& args) override;

			TextEditor* GetEditor() const;
		private:
			Rectangle GetEditorArea() const;
			std::unique_ptr<TextEditor> m_textEditor{ nullptr };
		};
	}
		
	struct ArgTextChanged
	{
		//ArgTextChanged(std::wstring& value):NewValue(value){}
		std::wstring NewValue;
	};
	
	namespace ReactorCore::InputText
	{
		struct Events : public ControlEvents
		{
			Event<ArgTextChanged> TextChanged;
		};
	}
	
	class InputText : public Control<ReactorCore::InputText::Reactor, ReactorCore::InputText::Events>
	{
	public:
		InputText() = default;
		InputText(Window* parent, const Rectangle& rectangle = {});

		void Deselect();
		void SelectAll();

		bool IsEditable() const;
		void SetEditable(bool isEditable);

		void SetMultiLine(bool enabled);
		void SetWordWrap(bool enabled);
		
		void SetCharFilter(std::function<bool(wchar_t)> predicate);

		std::wstring GetText() const;
		void SetText(const std::wstring& text);
		void SetText(const std::string& text);

		void SetFocusBehavior(TextFocusBehavior behavior);
		
	protected:
		void DoOnCaption(const std::wstring& caption) override;
		std::wstring DoOnCaption() const override;
	};
}

#endif