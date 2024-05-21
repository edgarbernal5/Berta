/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "TextEditor.h"

#include "Berta/GUI/Caret.h"
#include "Berta/GUI/Window.h"
#include "Berta/GUI/ControlAppearance.h"
#include "Berta/GUI/Interface.h"

namespace Berta
{
	TextEditor::TextEditor(Window* owner) :
		m_graphics(owner->Renderer.GetGraphics()),
		m_owner(owner)
	{
		m_caret = new Caret(owner, {1,0});
	}

	TextEditor::~TextEditor()
	{
		
		delete m_caret;
		m_caret = nullptr;
	}

	void TextEditor::OnMouseEnter(const ArgMouse& args)
	{
		GUI::ChangeCursor(m_owner, Cursor::IBeam);
	}

	void TextEditor::OnMouseLeave(const ArgMouse& args)
	{
	}

	void TextEditor::OnFocus(const ArgFocus& args)
	{
		if (args.Focused)
		{
			ActivateCaret();
		}
		else
		{
			DeactivateCaret();
		}
	}

	bool TextEditor::OnKeyChar(const ArgKeyboard& args)
	{
		if (std::isprint(static_cast<int>(args.Key)))
		{
			Insert(args.Key);
			return true;
		}
		return false;
	}

	bool TextEditor::OnKeyPressed(const ArgKeyboard& args)
	{
		bool redraw = false;
		auto contentSize = m_content.size();
		if (args.Key == VK_LEFT && m_caretPosition > 0)
		{
			MoveCaretLeft();
			redraw = true;
		}
		else if (args.Key == VK_RIGHT && m_caretPosition < contentSize)
		{
			MoveCaretRight();
			redraw = true;
		}
		else if (args.Key == VK_BACK && m_caretPosition > 0)
		{
			DeleteBack();
			redraw = true;
		}
		else if (args.Key == VK_DELETE && m_caretPosition < contentSize)
		{
			Delete();
			redraw = true;
		}
		return redraw;
	}

	void TextEditor::ActivateCaret()
	{
		m_caret->Activate();
	}

	void TextEditor::DeactivateCaret()
	{
		m_caret->Deactivate();
	}

	void TextEditor::Insert(wchar_t chr)
	{
		m_content.insert(m_caretPosition, 1, chr);
		++m_caretPosition;
		AdjustView();
	}

	void TextEditor::MoveCaretLeft()
	{
		--m_caretPosition;
		AdjustView(true);
	}

	void TextEditor::MoveCaretRight()
	{
		++m_caretPosition;
		AdjustView();
	}

	void TextEditor::Delete()
	{
		m_content.erase(m_caretPosition, 1);
	}

	void TextEditor::DeleteBack()
	{
		m_content.erase(m_caretPosition - 1, 1);
		--m_caretPosition;
		AdjustView();
	}

	void TextEditor::Render()
	{
		Size contentSize = GetContentSize();

		m_graphics.DrawString({ 2 + m_offset, (static_cast<int>(m_graphics.GetSize().Height - contentSize.Height) >> 1) + 1 }, m_content, m_owner->Appereance->Foreground);

		if (m_caret->IsVisible())
		{
			m_graphics.DrawLine({ 2 + m_offset + (int)contentSize.Width,3 }, { 2 + m_offset + (int)contentSize.Width, (int)m_owner->Size.Height - 2 }, { 0 });
		}
	}

	void TextEditor::AdjustView(bool scrollToLeft)
	{
		auto contentSize = GetContentSize();
		auto ownerSize = m_graphics.GetSize();

		int adjustment = 4;
		bool needAdjustment = m_offset + contentSize.Width < 0 || m_offset + contentSize.Width > ownerSize.Width - adjustment;

		if (needAdjustment)
		{
			if (scrollToLeft)
			{
				m_offset = -static_cast<int>(contentSize.Width) + adjustment;
			}
			else
			{
				m_offset = static_cast<int>(ownerSize.Width - contentSize.Width) - adjustment;
			}
			m_offset = (std::min)(m_offset, 0);
		}
	}

	Size TextEditor::GetContentSize()
	{
		Size contentSize;
		if (m_caretPosition == 0)
		{
			contentSize = { 0,m_graphics.GetTextExtent().Height };
		}
		else
		{
			contentSize = m_graphics.GetTextExtent(m_content, m_caretPosition);
		}
		return contentSize;
	}
}