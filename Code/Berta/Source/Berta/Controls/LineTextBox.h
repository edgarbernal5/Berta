/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LINE_TEXT_BOX_HEADER
#define BT_LINE_TEXT_BOX_HEADER

#include <string>
#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/TextEditors/EnumTypes.h"

namespace Berta
{
	class SingleLineEditor;

	namespace Internal::LineTextBox
	{
		struct Events;

		class Reactor : public ControlReactor
		{
		public:
			Reactor();
			~Reactor() override;
			
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
			void DpiChanged(Graphics& graphics) override;

			SingleLineEditor* GetEditor() const;
			
		protected:
			void DoOnInit() override;
			
		private:
			Rectangle GetEditorArea() const;
			std::unique_ptr<SingleLineEditor> m_textEditor{ nullptr };
		};
	}
	
	namespace Internal::LineTextBox
	{
		struct Events : public ControlEvents
		{
			Event<ArgTextChanged> TextChanged;
			Event<ArgSubmit> Submit;
		};
	}
	
	class LineTextBox : public Control<Category::ControlTag, Internal::LineTextBox::Reactor, Internal::LineTextBox::Events>
	{
	public:
		LineTextBox() = default;
		LineTextBox(Window* parent, const Rectangle& rectangle = {});

		size_t GetCaretPosition() const;
		
		void Deselect();
		void SelectAll();

		bool IsEditable() const;
		void SetEditable(bool isEditable);
		
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