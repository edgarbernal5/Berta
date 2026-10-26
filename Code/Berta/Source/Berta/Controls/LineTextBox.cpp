#include "btpch.h"
#include "LineTextBox.h"

#include "Berta/Controls/TextEditors/SingleLineEditor.h"

namespace Berta
{
    namespace Internal::LineTextBox
	{
		void Reactor::DoOnInit()
		{
			m_textEditor = std::make_unique<SingleLineEditor>(*m_control);

			m_textEditor->SetEditorArea(GetEditorArea(), m_control->Handle()->ClientPadding);
			m_textEditor->SetValueChangedCallback([this]()
			{
				ArgTextChanged args;
				args.Text = m_textEditor->GetContent();
				reinterpret_cast<Events*>(m_control->Handle()->Events.get())->TextChanged.Emit(args);
			});
			
			GUI::SetWindowBorderless(*m_control, false);
		}

		Reactor::Reactor() = default;

    	Reactor::~Reactor() = default;

		void Reactor::Update(Graphics& graphics)
		{
			m_textEditor->Render(graphics);
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
			if (m_textEditor->OnKeyPressed(args))
			{
				auto window = m_control->Handle();
				GUI::MarkAsNeedUpdate(window);
			}
		}

		void Reactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
		{
			//TODO
			//m_textEditor->OnKeyReleased(args);
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
			m_textEditor->SetEditorArea(GetEditorArea(), m_control->Handle()->ClientPadding);
			m_textEditor->OnResize(args);
		}

		void Reactor::DpiChanged(Graphics& graphics)
		{
			m_textEditor->SetEditorArea(GetEditorArea(), m_control->Handle()->ClientPadding);
			m_textEditor->OnDpiChanged();
		}

		SingleLineEditor* Reactor::GetEditor() const
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

	LineTextBox::LineTextBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "TextAreaBox";
#endif
	}

	size_t LineTextBox::GetCaretPosition() const
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			return editor->GetEndPosition().column;
		}
		return {};
	}

	void LineTextBox::Deselect()
	{
		auto editor = GetReactor().GetEditor();
		if (editor && editor->Deselect())
		{
			GUI::UpdateWindow(m_handle);
		}
	}

	void LineTextBox::SelectAll()
	{
		auto editor = GetReactor().GetEditor();
		if (editor && editor->SelectAll())
		{
			GUI::UpdateWindow(m_handle);
		}
	}

	bool LineTextBox::IsEditable() const
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			return editor->IsEditable();
		}
		return false;
	}

	void LineTextBox::SetEditable(bool isEditable)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetEditable(isEditable);
		}
	}

	void LineTextBox::SetCharFilter(std::function<bool(wchar_t)> predicate)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetCharFilter(std::move(predicate));
		}
	}

	std::wstring LineTextBox::GetText() const
	{
		return DoOnCaption();
	}

	void LineTextBox::SetText(const std::wstring& text)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetContent(text);
			GUI::UpdateWindow(m_handle);
		}
	}

	void LineTextBox::SetText(const std::string& text)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetContent(StringUtils::UTF8ToWide(text));
			GUI::UpdateWindow(m_handle);
		}
	}

	void LineTextBox::SetFocusBehavior(TextFocusBehavior behavior)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetBehavior(behavior);
		}
	}

	void LineTextBox::DoOnCaption(const std::wstring& caption)
	{
		auto editor = GetReactor().GetEditor();
		if (editor)
		{
			editor->SetContent(caption);
		}
	}

	std::wstring LineTextBox::DoOnCaption() const
	{
		return GetReactor().GetEditor()->GetContent();
	}
}