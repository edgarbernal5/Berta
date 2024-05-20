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

namespace Berta
{
	TextEditor::TextEditor(Window* owner) :
		m_graphics(owner->Renderer.GetGraphics()),
		m_owner(owner)
	{
		m_caret = new Caret(owner, {1,0});
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
	}

	void TextEditor::MoveCaretLeft()
	{
		--m_caretPosition;
	}

	void TextEditor::MoveCaretRight()
	{
		++m_caretPosition;
	}

	void TextEditor::Delete()
	{
		m_content.erase(m_caretPosition, 1);
	}

	void TextEditor::DeleteBack()
	{
		m_content.erase(m_caretPosition - 1, 1);
		--m_caretPosition;
	}

	void TextEditor::Render()
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

		m_graphics.DrawString({ 2, (static_cast<int>(m_graphics.GetSize().Height - contentSize.Height) >> 1) + 1 }, m_content, m_owner->Appereance->Foreground);

		if (m_caret->IsVisible())
		{
			BT_CORE_TRACE << "draw caret. contentSize = " << contentSize << "." << std::endl;
			//graphics.DrawLine({ 2,3 }, { 2, (int)window->Size.Height - 2 }, { 0 });
			m_graphics.DrawLine({ 2 + (int)contentSize.Width,3 }, { 2 + (int)contentSize.Width, (int)m_owner->Size.Height - 2 }, { 0 });
		}
		else
		{
			BT_CORE_TRACE << "hide caret" << std::endl;
		}
	}
}