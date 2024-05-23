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
	}

	void TextEditor::OnMouseLeave(const ArgMouse& args)
	{
	}

	void TextEditor::OnMouseDown(const ArgMouse& args)
	{
		if (m_content.size() == 0)
		{
			return;
		}
		isSelecting = true;
		m_selectionStartPosition = GetPositionUnderMouse(args.Position);
		m_selectionEndPosition = m_selectionStartPosition;
		m_caretPosition = m_selectionStartPosition;
	}

	void TextEditor::OnMouseMove(const ArgMouse& args)
	{
		if (isSelecting)
		{
			m_selectionEndPosition = GetPositionUnderMouse(args.Position);
		}
	}

	void TextEditor::OnMouseUp(const ArgMouse& args)
	{
		if (m_content.size() == 0)
		{
			return;
		}
		m_selectionEndPosition = GetPositionUnderMouse(args.Position);
		if (m_selectionStartPosition != m_selectionEndPosition)
		{
			m_caretPosition = m_selectionEndPosition;
		}

		isSelecting = false;
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
		if (m_selectionEndPosition != m_selectionStartPosition)
		{
			auto start = (std::min)(m_selectionStartPosition, m_selectionEndPosition);
			auto end = (std::max)(m_selectionStartPosition, m_selectionEndPosition);

			m_content.erase(start, (end - start));
			m_content.insert(m_caretPosition, 1, chr);
			++m_caretPosition;

			m_selectionEndPosition = m_selectionStartPosition = 0;
		}
		else
		{
			m_content.insert(m_caretPosition, 1, chr);
			++m_caretPosition;
		}
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
		if (m_selectionEndPosition != m_selectionStartPosition)
		{
			auto start = (std::min)(m_selectionStartPosition, m_selectionEndPosition);
			auto end = (std::max)(m_selectionStartPosition, m_selectionEndPosition);

			m_content.erase(start, (end - start));
			int caretPosition = m_caretPosition;
			caretPosition -= (end - start);

			if (caretPosition < 0)
			{
				m_caretPosition = m_content.size();
			}
			else
			{
				m_caretPosition = caretPosition;
			}

			m_selectionEndPosition = m_selectionStartPosition = 0;
		}
		else
		{
			m_content.erase(m_caretPosition, 1);
		}
	}

	void TextEditor::DeleteBack()
	{
		if (m_selectionEndPosition != m_selectionStartPosition)
		{
			auto start = (std::min)(m_selectionStartPosition, m_selectionEndPosition);
			auto end = (std::max)(m_selectionStartPosition, m_selectionEndPosition);

			m_content.erase(start, (end - start));
			int caretPosition = m_caretPosition;
			caretPosition -= (end - start);

			if (caretPosition < 0)
			{
				m_caretPosition = m_content.size();
			}
			else
			{
				m_caretPosition = caretPosition;
			}

			m_selectionEndPosition = m_selectionStartPosition = 0;
		}
		else
		{
			m_content.erase(m_caretPosition - 1, 1);
			--m_caretPosition;
		}
		AdjustView();
	}

	void TextEditor::Render()
	{
		Size contentSize = GetContentSize();

		m_graphics.DrawString({ 2 + m_offsetView, (static_cast<int>(m_graphics.GetSize().Height - contentSize.Height) >> 1) + 1 }, m_content, m_owner->Appereance->Foreground);

		if (m_caret->IsVisible())
		{
			m_graphics.DrawLine({ 2 + m_offsetView + (int)contentSize.Width,3 }, { 2 + m_offsetView + (int)contentSize.Width, (int)m_owner->Size.Height - 2 }, { 0 });
		}
		if (m_selectionEndPosition != m_selectionStartPosition)
		{
			auto start = (std::min)(m_selectionStartPosition, m_selectionEndPosition);
			auto end = (std::max)(m_selectionStartPosition, m_selectionEndPosition);
			auto startTextExtent = m_graphics.GetTextExtent(m_content.substr(0, start));
			auto endTextExtent = m_graphics.GetTextExtent(m_content.substr(0, end));

			std::wostringstream builder;
			for (size_t i = start; i < (end); i++)
			{
				builder << m_content[i];
			}
			m_graphics.DrawRectangle({ 2 + m_offsetView + (int)startTextExtent.Width , 2, endTextExtent.Width - startTextExtent.Width, m_owner->Size.Height - 4 }, m_owner->Appereance->HighlightColor, true);
			m_graphics.DrawString({ 2 + m_offsetView + (int)startTextExtent.Width, (static_cast<int>(m_graphics.GetSize().Height - contentSize.Height) >> 1) + 1 }, builder.str(), m_owner->Appereance->HighlightTextColor);
		}
	}

	void TextEditor::AdjustView(bool scrollToLeft)
	{
		auto contentSize = GetContentSize();
		auto ownerSize = m_graphics.GetSize();

		int adjustment = 4;
		bool needAdjustment = m_offsetView + contentSize.Width < 0 || m_offsetView + contentSize.Width > ownerSize.Width - adjustment;

		if (needAdjustment)
		{
			if (scrollToLeft)
			{
				m_offsetView = -static_cast<int>(contentSize.Width) + adjustment;
			}
			else
			{
				m_offsetView = static_cast<int>(ownerSize.Width - contentSize.Width) - adjustment;
			}
			m_offsetView = (std::min)(m_offsetView, 0);
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

	uint32_t TextEditor::GetPositionUnderMouse(const Point& mousePosition)
	{
		uint32_t index = 0;
		
		int nearest = (std::numeric_limits<int>::max)();
		for (size_t i = 0; i < m_content.size() + 1; i++)
		{
			auto letterSize = m_graphics.GetTextExtent(m_content.substr(0, i));
			auto abs = std::abs((int)letterSize.Width - mousePosition.X);
			if (abs < nearest)
			{
				nearest = abs;
				index = i;
			}
		}
		return index;
	}
}