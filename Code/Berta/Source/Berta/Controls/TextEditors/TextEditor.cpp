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

		m_selectionTimer.SetOwner(m_owner);
		m_selectionTimer.Connect([this](const ArgTimer& args)
		{
			BT_CORE_DEBUG << "timer tick..." << std::endl;
			//if (m_selectionDirection)
			//{
			//	m_offsetView += 10;
			//}
			//else
			//	m_offsetView -= 10;
			//AdjustView();
		});
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
		m_selectionMousePosition = args.Position;
		isSelecting = true;
		if (m_shiftPressed)
		{
			m_selectionEndPosition = GetPositionUnderMouse(args.Position);
			if (m_selectionStartPosition == -1)
				m_selectionStartPosition = m_caretPosition;

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
		if (args.ButtonState.LeftButton && !m_selectionTimer.IsRunning() && (args.Position.X > (int)m_owner->Size.Width || args.Position.X < 0))
		{
			m_selectionTimer.SetInterval(300);
			m_selectionTimer.Start();
			m_selectionDirection = args.Position.X < 0;
		}
		else if (args.ButtonState.LeftButton && m_selectionTimer.IsRunning() && !(args.Position.X > (int)m_owner->Size.Width || args.Position.X < 0))
		{
			m_selectionTimer.Stop();
		}
	}

	void TextEditor::OnMouseUp(const ArgMouse& args)
	{
		m_selectionTimer.Stop();
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
		else if (args.Key == KeyboardKey::Delete && (m_caretPosition < contentSize || m_selectionStartPosition != m_selectionEndPosition))
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

		size_t start = GetPositionNextWord(m_caretPosition, -1);
		size_t end = GetPositionNextWord(m_caretPosition, 1);

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
			{
				m_caretPosition -= (end - start);
			}

			m_content.insert(m_caretPosition, 1, chr);
			++m_caretPosition;

			m_selectionEndPosition = m_selectionStartPosition = -1;
		}
		else
		{
			m_content.insert(m_caretPosition, 1, chr);
			++m_caretPosition;
		}
		AdjustView();
		EmitValueChanged();
	}

	void TextEditor::MoveCaretLeft()
	{
		size_t newCaretPosition = m_caretPosition;
		if (newCaretPosition > 0)
			--newCaretPosition;

		if (m_ctrlPressed)
		{
			size_t ctrlPos = GetPositionNextWord(m_caretPosition - 1, -1);
			newCaretPosition = ctrlPos;
		}
		if (m_shiftPressed)
		{
			if (m_selectionStartPosition == -1 && m_selectionEndPosition == -1)
			{
				m_selectionStartPosition = m_caretPosition;
			}
			m_selectionEndPosition = newCaretPosition;
		}
		else
		{
			m_selectionEndPosition = m_selectionStartPosition = -1;
		}
		m_caretPosition = newCaretPosition;
		AdjustView();
	}

	void TextEditor::MoveCaretHome()
	{
		size_t newCaretPosition = 0;

		if (m_shiftPressed)
		{
			if (m_selectionStartPosition == -1 && m_selectionEndPosition == -1)
			{
				m_selectionStartPosition = m_caretPosition;
			}
			m_selectionEndPosition = newCaretPosition;
		}
		else
		{
			m_selectionEndPosition = m_selectionStartPosition = -1;
		}
		m_caretPosition = newCaretPosition;
		AdjustView();
	}

	void TextEditor::MoveCaretRight()
	{
		size_t newCaretPosition = m_caretPosition;

		if (newCaretPosition < m_content.size())
			++newCaretPosition;

		if (m_ctrlPressed)
		{
			size_t ctrlPos = GetPositionNextWord(m_caretPosition, 1);
			newCaretPosition = ctrlPos;
		}
		if (m_shiftPressed)
		{
			if (m_selectionStartPosition == -1 && m_selectionEndPosition == -1)
			{
				m_selectionStartPosition = m_caretPosition;
			}
			m_selectionEndPosition = newCaretPosition;
		}
		else
		{
			m_selectionEndPosition = m_selectionStartPosition = -1;
		}
		m_caretPosition = newCaretPosition;
		AdjustView();
	}

	void TextEditor::MoveCaretEnd()
	{
		size_t newCaretPosition = m_content.size();

		if (m_shiftPressed)
		{
			if (m_selectionStartPosition == -1 && m_selectionEndPosition == -1)
			{
				m_selectionStartPosition = m_caretPosition;
			}
			m_selectionEndPosition = newCaretPosition;
		}
		else
		{
			m_selectionEndPosition = m_selectionStartPosition = -1;
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
			int64_t caretPosition = static_cast<int64_t>(m_caretPosition);
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

			m_selectionEndPosition = m_selectionStartPosition = -1;
		}
		else
		{
			m_content.erase(m_caretPosition, 1);
		}
		AdjustView();
		EmitValueChanged();
	}

	void TextEditor::DeleteBack()
	{
		if (m_caretPosition == 0 && m_selectionEndPosition == m_selectionStartPosition)
			return;

		if (m_selectionEndPosition != m_selectionStartPosition)
		{
			auto start = (std::min)(m_selectionStartPosition, m_selectionEndPosition);
			auto end = (std::max)(m_selectionStartPosition, m_selectionEndPosition);

			std::wstring stringToDelete{ m_content.data() + start, m_content.data() + end };
			m_content.erase(start, (end - start));
			int64_t caretPosition = m_caretPosition;
			if (m_caretPosition == end)
				caretPosition -= (end - start);

			if (caretPosition < 0)
			{
				m_caretPosition = static_cast<uint32_t>(m_content.size());
			}
			else
			{
				m_caretPosition = caretPosition;
			}
			if (m_offsetView < 0)
			{
				m_offsetView += m_graphics.GetTextExtent(stringToDelete).Width;
				if (m_offsetView > 0) m_offsetView = 0;
			}

			m_selectionEndPosition = m_selectionStartPosition = -1;
		}
		else
		{
			m_content.erase(m_caretPosition - 1, 1);
			--m_caretPosition;
		}
		AdjustView();
		EmitValueChanged();
	}

	void TextEditor::Render()
	{
		Size contentSize = GetContentTextExtent(m_caretPosition);

		bool enabled = m_owner->Flags.IsEnabled;
		
		if (m_selectionEndPosition != m_selectionStartPosition)
		{
			auto start = (std::min)(m_selectionStartPosition, m_selectionEndPosition);
			auto end = (std::max)(m_selectionStartPosition, m_selectionEndPosition);
			auto startTextExtent = m_graphics.GetTextExtent(m_content.substr(0, start));
			std::wstring selectionText{ m_content.data() + start, m_content.data() + end };
			auto endTextExtent = m_graphics.GetTextExtent(selectionText);

			m_graphics.DrawRectangle({ 2 + m_offsetView + (int)startTextExtent.Width , 2, endTextExtent.Width, m_owner->Size.Height - 4 }, m_owner->Appearance->HighlightColor, true);
			//m_graphics.DrawString({ 2 + m_offsetView + (int)startTextExtent.Width, (static_cast<int>(m_graphics.GetSize().Height - contentSize.Height) >> 1) + 1 }, selectionText, m_owner->Appearance->HighlightTextColor);
		}
		m_graphics.DrawString({ 2 + m_offsetView, (static_cast<int>(m_graphics.GetSize().Height - contentSize.Height) >> 1) + 1 }, m_content, enabled ? m_owner->Appearance->Foreground : m_owner->Appearance->BoxBorderDisabledColor);

		if (m_caret->IsVisible())
		{
			m_graphics.DrawLine({ 2 + m_offsetView + (int)contentSize.Width,3 }, { 2 + m_offsetView + (int)contentSize.Width, (int)m_owner->Size.Height - 2 }, { 0 });
		}
	}

	void TextEditor::AdjustView()
	{
		auto contentSizeAtCaret = GetContentTextExtent(m_caretPosition);
		auto contentSize = GetContentTextExtent(m_content.size());
		auto ownerSize = m_graphics.GetSize();

		constexpr int adjustment = 4;
		bool needAdjustment = m_offsetView + (int)contentSize.Width  < (int)ownerSize.Width - adjustment || m_offsetView + (int)contentSizeAtCaret.Width < 0 || m_offsetView + (int)contentSizeAtCaret.Width > (int)ownerSize.Width - adjustment;
		if (needAdjustment)
		{
			if (m_offsetView + (int)contentSize.Width < (int)ownerSize.Width - adjustment)
			{
				m_offsetView = static_cast<int>(ownerSize.Width - contentSizeAtCaret.Width) - adjustment;
			}
			else if (m_offsetView + (int)contentSizeAtCaret.Width < 0)
			{
				m_offsetView = -static_cast<int>(contentSizeAtCaret.Width) + adjustment;
			}
			else
			{
				m_offsetView = static_cast<int>(ownerSize.Width - contentSizeAtCaret.Width) - adjustment;
			}
			m_offsetView = std::clamp(m_offsetView, -(int)contentSize.Width, 0);
		}
	}

	Size TextEditor::GetContentTextExtent(size_t position) const
	{
		Size contentSize;
		if (position == 0)
		{
			contentSize = { 0, m_graphics.GetTextExtent().Height };
		}
		else
		{
			contentSize = m_graphics.GetTextExtent(m_content, position);
		}
		return contentSize;
	}

	size_t TextEditor::GetPositionUnderMouse(const Point& mousePosition) const
	{
		size_t index = 0;
		
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

	size_t TextEditor::GetPositionNextWord(int64_t currentPosition, int direction) const
	{
		if (m_content.size() == 0)
		{
			return currentPosition; //0
		}

		if (currentPosition < 0)
		{
			return 0;
		}

		if (static_cast<size_t>(currentPosition) >= m_content.size())
		{
			return m_content.size();
		}

		int64_t contentSize = static_cast<int64_t>(m_content.size());
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
		while (p >= 0 && p < contentSize);
		
		if (p < 0) p = 0;
		
		return p;
	}

	void TextEditor::EmitValueChanged()
	{
		if (m_valueChangedCallback)
		{
			m_valueChangedCallback();
		}
	}
}