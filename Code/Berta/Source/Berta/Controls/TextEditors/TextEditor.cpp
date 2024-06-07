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
#include "Berta/GUI/EnumTypes.h"

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
		if (m_shiftPressed)
		{
			m_selectionEndPosition = GetPositionUnderMouse(args.Position);
			m_caretPosition = m_selectionEndPosition;
		}
		else
		{
			m_selectionStartPosition = GetPositionUnderMouse(args.Position);
			m_selectionEndPosition = m_selectionStartPosition;
			m_caretPosition = m_selectionStartPosition;
		}
	}

	void TextEditor::OnMouseMove(const ArgMouse& args)
	{
		if (isSelecting)
		{
			m_selectionEndPosition = GetPositionUnderMouse(args.Position);
			m_caretPosition = m_selectionEndPosition;
		}
	}

	void TextEditor::OnMouseUp(const ArgMouse& args)
	{
		if (m_wasDblClick)
		{
			m_wasDblClick = false;
			return;
		}
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
			isSelecting = false;
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
		m_shiftPressed = m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_ctrlPressed = m_ctrlPressed || args.Key == KeyboardKey::Control;
		if (args.Key == KeyboardKey::ArrowLeft && (m_caretPosition > 0 || m_selectionStartPosition != m_selectionEndPosition))
		{
			MoveCaretLeft();
			redraw = true;
		}
		else if (args.Key == KeyboardKey::ArrowRight && (m_caretPosition < contentSize || m_selectionStartPosition != m_selectionEndPosition))
		{
			MoveCaretRight();
			redraw = true;
		}
		else if (args.Key == KeyboardKey::Home)
		{
			MoveCaretHome();
			redraw = true;
		}
		else if (args.Key == KeyboardKey::End)
		{
			MoveCaretEnd();
			redraw = true;
		}
		else if (args.Key == KeyboardKey::Backspace && contentSize > 0)
		{
			DeleteBack();
			redraw = true;
		}
		else if (args.Key == KeyboardKey::Delete && m_caretPosition < contentSize)
		{
			Delete();
			redraw = true;
		}
		return redraw;
	}

	bool TextEditor::OnKeyReleased(const ArgKeyboard& args)
	{
		if (args.Key == KeyboardKey::Shift) m_shiftPressed = false;
		if (args.Key == KeyboardKey::Control) m_ctrlPressed = false;
		return false;
	}

	bool TextEditor::OnDblClick(const ArgClick& args)
	{
		m_wasDblClick = true;
		if (m_content.empty())
			return false;

		int start = GetPositionNextWord(m_caretPosition, -1);
		int end = GetPositionNextWord(m_caretPosition, 1);

		m_selectionStartPosition = start;
		m_selectionEndPosition = end;
		m_caretPosition = end;
		return true;
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
			
			if (m_caretPosition == end)
				m_caretPosition -= (end - start);

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
		uint32_t newCaretPosition = m_caretPosition;
		if (newCaretPosition > 0)
			--newCaretPosition;

		if (m_ctrlPressed)
		{
			int ctrlPos = GetPositionNextWord(m_caretPosition - 1, -1);
			newCaretPosition = ctrlPos;
		}
		if (m_shiftPressed)
		{
			if (m_selectionStartPosition == 0 && m_selectionEndPosition == 0)
			{
				m_selectionStartPosition = m_caretPosition;
			}
			m_selectionEndPosition = newCaretPosition;
		}
		else
		{
			m_selectionEndPosition = m_selectionStartPosition = 0;
		}
		m_caretPosition = newCaretPosition;
		AdjustView(true);
	}

	void TextEditor::MoveCaretHome()
	{
		uint32_t newCaretPosition = 0;

		if (m_shiftPressed)
		{
			if (m_selectionStartPosition == 0 && m_selectionEndPosition == 0)
			{
				m_selectionStartPosition = m_caretPosition;
			}
			m_selectionEndPosition = newCaretPosition;
		}
		else
		{
			m_selectionEndPosition = m_selectionStartPosition = 0;
		}
		m_caretPosition = newCaretPosition;
		AdjustView(true);
	}

	void TextEditor::MoveCaretRight()
	{
		uint32_t newCaretPosition = m_caretPosition;

		if (newCaretPosition < m_content.size())
			++newCaretPosition;

		if (m_ctrlPressed)
		{
			int ctrlPos = GetPositionNextWord(m_caretPosition, 1);
			newCaretPosition = ctrlPos;
		}
		if (m_shiftPressed)
		{
			if (m_selectionStartPosition == 0 && m_selectionEndPosition == 0)
			{
				m_selectionStartPosition = m_caretPosition;
			}
			m_selectionEndPosition = newCaretPosition;
		}
		else
		{
			m_selectionEndPosition = m_selectionStartPosition = 0;
		}
		m_caretPosition = newCaretPosition;
		AdjustView();
	}

	void TextEditor::MoveCaretEnd()
	{
		uint32_t newCaretPosition = m_content.size();

		if (m_shiftPressed)
		{
			if (m_selectionStartPosition == 0 && m_selectionEndPosition == 0)
			{
				m_selectionStartPosition = m_caretPosition;
			}
			m_selectionEndPosition = newCaretPosition;
		}
		else
		{
			m_selectionEndPosition = m_selectionStartPosition = 0;
		}
		m_caretPosition = newCaretPosition;
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
			if (m_caretPosition == end)
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

			std::wstring stringToDelete{ m_content.data() + start, m_content.data() + end };
			m_content.erase(start, (end - start));
			int caretPosition = m_caretPosition;
			if (m_caretPosition == end)
				caretPosition -= (end - start);

			if (caretPosition < 0)
			{
				m_caretPosition = m_content.size();
			}
			else
			{
				m_caretPosition = caretPosition;
			}
			if (m_offsetView < 0)
			{
				m_offsetView += m_graphics.GetTextExtent(stringToDelete).Width;
				if (m_offsetView > 0)m_offsetView = 0;
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
		Size contentSize = GetContentTextExtent();

		m_graphics.DrawString({ 2 + m_offsetView, (static_cast<int>(m_graphics.GetSize().Height - contentSize.Height) >> 1) + 1 }, m_content, m_owner->Appereance->Foreground);

		if (m_selectionEndPosition != m_selectionStartPosition)
		{
			auto start = (std::min)(m_selectionStartPosition, m_selectionEndPosition);
			auto end = (std::max)(m_selectionStartPosition, m_selectionEndPosition);
			auto startTextExtent = m_graphics.GetTextExtent(m_content.substr(0, start));
			std::wstring selectionText{ m_content.data() + start, m_content.data() + end };
			auto endTextExtent = m_graphics.GetTextExtent(selectionText);

			m_graphics.DrawRectangle({ 2 + m_offsetView + (int)startTextExtent.Width , 2, endTextExtent.Width, m_owner->Size.Height - 4 }, m_owner->Appereance->HighlightColor, true);
			m_graphics.DrawString({ 2 + m_offsetView + (int)startTextExtent.Width, (static_cast<int>(m_graphics.GetSize().Height - contentSize.Height) >> 1) + 1 }, selectionText, m_owner->Appereance->HighlightTextColor);
		}
		if (m_caret->IsVisible())
		{
			m_graphics.DrawLine({ 2 + m_offsetView + (int)contentSize.Width,3 }, { 2 + m_offsetView + (int)contentSize.Width, (int)m_owner->Size.Height - 2 }, { 0 });
		}
	}

	void TextEditor::AdjustView(bool scrollToLeft)
	{
		auto contentSize = GetContentTextExtent();
		auto ownerSize = m_graphics.GetSize();

		constexpr int adjustment = 4;
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

	Size TextEditor::GetContentTextExtent() const
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

	uint32_t TextEditor::GetPositionUnderMouse(const Point& mousePosition) const
	{
		uint32_t index = 0;
		
		int nearest = (std::numeric_limits<int>::max)();
		for (size_t i = 0; i <= m_content.size(); i++)
		{
			auto letterSize = m_graphics.GetTextExtent(m_content.substr(0, i));
			auto abs = std::abs((int)letterSize.Width - mousePosition.X + m_offsetView);
			if (abs < nearest)
			{
				nearest = abs;
				index = i;
			}
		}
		return index;
	}

	uint32_t TextEditor::GetPositionNextWord(int currentPosition, int direction) const
	{
		if (m_content.size() == 0)
		{
			return currentPosition;
		}
		if (currentPosition < 0)
			return 0;
		if (currentPosition >= m_content.size())
			return m_content.size();

		auto p = currentPosition;
		bool ignore = m_content[p] == ' ';
		do
		{
			if (ignore)
			{
				ignore = false;
			}
			else if (m_content[p] == ' ')
			{
				if (direction == -1)
					++p;
				break;
			}
			p += direction;
		}
		while (p >= 0 && p < m_content.size());
		
		if (p < 0) p = 0;
		
		return p;
	}
}