/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "InputText.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/Caret.h"

namespace Berta
{
	namespace Internal::InputText
	{
		void Reactor::DoOnInit()
		{
			m_textEditor = std::make_unique<TextEditor>(*m_control, m_graphics);

			m_textEditor->SetEditorArea(GetEditorArea());
			m_textEditor->SetValueChangedCallback([this]()
			{
				ArgTextChanged args;
				//args.NewValue = m_textEditor->GetContent();
				reinterpret_cast<Events*>(m_control->Handle()->Events.get())->TextChanged.Emit(args);
			});
		}

		void Reactor::Update(Graphics& graphics)
		{
			m_textEditor->Render();
		}

		void Reactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
		{
			GUI::ChangeCursor(*m_control, Cursor::IBeam);
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			GUI::ChangeCursor(*m_control, Cursor::Default);
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			//GUI::Capture(*m_control); //moved into text editor

			m_textEditor->OnMouseDown(args);
			GUI::MarkAsNeedUpdate(m_control->Handle());
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			auto window = m_control->Handle();
		
			m_textEditor->OnMouseMove(args);
			GUI::MarkAsNeedUpdate(window);
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			//GUI::ReleaseCapture(*m_control); //moved into text editor
			m_textEditor->OnMouseUp(args);
		}

		void Reactor::Focus(Graphics& graphics, const ArgFocus& args)
		{
			auto window = m_control->Handle();
			if (m_textEditor->OnFocus(args))
			{
				GUI::MarkAsNeedUpdate(window);
			}
		}

		void Reactor::KeyChar(Graphics& graphics, const ArgKeyboard& args)
		{
			BT_CORE_DEBUG << "key char: " << (int)args.Key << ". " << std::endl;
			if (m_textEditor->OnKeyChar(args))
			{
				GUI::CaptionWindow(m_control->Handle(), m_textEditor->GetContent());
				GUI::MarkAsNeedUpdate(m_control->Handle());
			}
		}

		void Reactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
		{
			bool redraw = m_textEditor->OnKeyPressed(args);
			if (redraw)
			{
				auto window = m_control->Handle();
				GUI::MarkAsNeedUpdate(window);
			}
		}

		void Reactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
		{
			m_textEditor->OnKeyReleased(args);
		}

		void Reactor::DblClick(Graphics& graphics, const ArgMouse& args)
		{
			if (m_textEditor->OnDblClick(args))
			{
				auto window = m_control->Handle();
				GUI::MarkAsNeedUpdate(window);
			}
		}

		void Reactor::Resize(Graphics& graphics, const ArgResize& args)
		{
			m_textEditor->SetEditorArea(GetEditorArea());
			m_textEditor->OnResize(args);
		}

		void Reactor::DpiChanged(Graphics& graphics)
		{
			m_textEditor->OnDpiChanged();
		}

		TextEditor* Reactor::GetEditor() const
		{
			return m_textEditor.get();
		}

		Rectangle Reactor::GetEditorArea() const
		{
			auto area = m_control->GetSize().ToRectangle();
			if (!GUI::IsWindowBorderless(m_control->Handle()))
			{
				auto one = m_control->Handle()->ToScale(1);
				
				area.X = area.Y = one;
				if (area.Width > one * 2) area.Width -= one * 2;
				else area.Width = 0;
				
				if (area.Height > one * 2) area.Height -= one * 2;
				else area.Height = 0;
			}
			return area;
		}
	}

	InputText::InputText(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "InputText";
#endif
	}

	TextPosition InputText::GetCaretPosition() const
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			return editor->GetEndPosition();
		}
		return {};
	}

	void InputText::Deselect()
	{
		auto editor = GetReactor().GetEditor();
		if (editor && editor->Deselect())
		{
			GUI::UpdateWindow(m_handle);
		}
	}

	void InputText::SelectAll()
	{
		auto editor = GetReactor().GetEditor();
		if (editor && editor->SelectAll())
		{
			GUI::UpdateWindow(m_handle);
		}
	}

	bool InputText::IsEditable() const
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			return editor->IsEditable();
		}
		return false;
	}

	void InputText::SetEditable(bool isEditable)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetEditable(isEditable);
		}
	}

	void InputText::SetMultiLine(bool enabled)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetMultiline(enabled);
			GUI::UpdateWindow(m_handle);
		}
	}

	void InputText::SetWordWrap(bool enabled)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetWordWrap(enabled);
			GUI::UpdateWindow(m_handle);
		}
	}

	void InputText::SetCharFilter(std::function<bool(wchar_t)> predicate)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetCharFilter(std::move(predicate));
		}
	}

	std::wstring InputText::GetText() const
	{
		return DoOnCaption();
	}

	void InputText::SetText(const std::wstring& text)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetContent(text);
			GUI::UpdateWindow(m_handle);
		}
	}

	void InputText::SetText(const std::string& text)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetContent(text);
			GUI::UpdateWindow(m_handle);
		}
	}

	void InputText::SetFocusBehavior(TextFocusBehavior behavior)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetBehavior(behavior);
		}
	}

	void InputText::DoOnCaption(const std::wstring& caption)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetContent(caption);
		}
	}

	std::wstring InputText::DoOnCaption() const
	{
		return GetReactor().GetEditor()->GetContent();
	}
}