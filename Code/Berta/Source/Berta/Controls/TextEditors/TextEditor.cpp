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
	TextEditor::TextEditor(Window* owner, Graphics* graphics) :
		m_graphics(*graphics),
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
		if (m_selection.m_ignoreMouseDown)
		{
			return;
		}
		if (m_content.size() == 0)
		{
			return;
		}
		m_selectionMousePosition = args.Position;
		m_selection.m_isSelecting = true;
		if (m_shiftPressed)
		{
			m_selection.m_endPosition = GetPositionUnderMouse(args.Position);
			if (m_selection.m_startPosition == -1)
			{
				m_selection.m_startPosition = m_caretPosition;
			}

			m_caretPosition = m_selection.m_endPosition;
		}
		else
		{
			m_selection.m_startPosition = GetPositionUnderMouse(args.Position);
			m_selection.m_endPosition = m_selection.m_startPosition;
			m_caretPosition = m_selection.m_startPosition;
		}
	}

	void TextEditor::OnMouseMove(const ArgMouse& args)
	{
		if (m_selection.m_isSelecting)
		{
			m_selection.m_endPosition = static_cast<int64_t>(GetPositionUnderMouse(args.Position));
			m_caretPosition = m_selection.m_endPosition;
		}
		if (args.ButtonState.LeftButton && !m_selectionTimer.IsRunning() && (args.Position.X > static_cast<int>(m_owner->ClientSize.Width) || args.Position.X < 0))
		{
			m_selectionTimer.SetInterval(300);
			m_selectionTimer.Start();
			m_selectionDirection = args.Position.X < 0;
		}
		else if (args.ButtonState.LeftButton && m_selectionTimer.IsRunning() && !(args.Position.X > static_cast<int>(m_owner->ClientSize.Width) || args.Position.X < 0))
		{
			m_selectionTimer.Stop();
		}
	}

	void TextEditor::OnMouseUp(const ArgMouse& args)
	{
		if (m_selection.m_ignoreMouseDown)
		{
			m_selection.m_ignoreMouseDown = false;
			return;
		}
		m_selectionTimer.Stop();
		if (m_wasDblClick)
		{
			m_wasDblClick = false;
			return;
		}
		if (m_content.empty())
		{
			return;
		}
		m_selection.m_endPosition = static_cast<int64_t>(GetPositionUnderMouse(args.Position));
		if (m_selection.m_startPosition != m_selection.m_endPosition)
		{
			m_caretPosition = m_selection.m_endPosition;
		}

		m_selection.m_isSelecting = false;
	}

	bool TextEditor::OnFocus(const ArgFocus& args)
	{
		bool needUpdate = false;
		if (args.Focused)
		{
			bool selectAll = false;
			switch (m_selection.Behavior)
			{
			case TextFocusBehavior::Select:
				selectAll = true;
				break;
			case TextFocusBehavior::SelectOnClick:
				selectAll = args.FocusReason == ArgFocus::Reason::MousePress;
				break;
			default:
				break;
			}

			if (selectAll)
			{
				needUpdate = true;
				SelectAll();

				m_selection.m_ignoreMouseDown = args.FocusReason == ArgFocus::Reason::MousePress;
			}

			ActivateCaret();
			return needUpdate;
		}

		if (m_selection.Behavior != TextFocusBehavior::None)
		{
			Deselect();
		}
		DeactivateCaret();
		m_selection.m_isSelecting = false;
		return needUpdate;
	}

	bool TextEditor::OnKeyChar(const ArgKeyboard& args)
	{
		if (!m_features.isEditable)
		{
			return false;
		}

		if (!m_predicate || m_predicate(args.Key))
		{
			if (std::isprint(static_cast<int>(args.Key)))
			{
				Insert(args.Key);
				return true;
			}
		}
		return false;
	}

	bool TextEditor::OnKeyPressed(const ArgKeyboard& args)
	{
		bool redraw = false;
		auto contentSize = m_content.size();
		m_shiftPressed = m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_ctrlPressed = m_ctrlPressed || args.Key == KeyboardKey::Control;

		if (args.Key == KeyboardKey::ArrowLeft && (m_caretPosition > 0 || m_selection.m_startPosition != m_selection.m_endPosition))
		{
			MoveCaretLeft();
			redraw = true;
		}
		else if (args.Key == KeyboardKey::ArrowRight && (m_caretPosition < contentSize || m_selection.m_startPosition != m_selection.m_endPosition))
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
		else if (args.Key == KeyboardKey::Backspace && m_features.isEditable && contentSize > 0)
		{
			DeleteBack();
			redraw = true;
		}
		else if (args.Key == KeyboardKey::Delete && m_features.isEditable && (m_caretPosition < contentSize || m_selection.m_startPosition != m_selection.m_endPosition))
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

	bool TextEditor::OnDblClick(const ArgMouse& args)
	{
		m_wasDblClick = true;
		if (m_content.empty())
		{
			return false;
		}

		size_t start = GetPositionNextWord(m_caretPosition, -1);
		size_t end = GetPositionNextWord(m_caretPosition, 1);

		m_selection.m_startPosition = start;
		m_selection.m_endPosition = end;
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
		if (m_selection.m_endPosition != m_selection.m_startPosition)
		{
			auto start = (std::min)(m_selection.m_startPosition, m_selection.m_endPosition);
			auto end = (std::max)(m_selection.m_startPosition, m_selection.m_endPosition);

			m_content.erase(start, (end - start));
			
			if (m_caretPosition == end)
			{
				m_caretPosition -= (end - start);
			}

			m_content.insert(m_caretPosition, 1, chr);
			++m_caretPosition;

			m_selection.m_endPosition = m_selection.m_startPosition = -1;
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
		{
			--newCaretPosition;
		}

		if (m_ctrlPressed)
		{
			size_t ctrlPos = GetPositionNextWord(m_caretPosition - 1, -1);
			newCaretPosition = ctrlPos;
		}
		if (m_shiftPressed)
		{
			if (m_selection.m_startPosition == -1 && m_selection.m_endPosition == -1)
			{
				m_selection.m_startPosition = m_caretPosition;
			}
			m_selection.m_endPosition = newCaretPosition;
		}
		else
		{
			m_selection.m_endPosition = m_selection.m_startPosition = -1;
		}
		m_caretPosition = newCaretPosition;
		AdjustView();
	}

	void TextEditor::MoveCaretHome()
	{
		size_t newCaretPosition = 0;

		if (m_shiftPressed)
		{
			if (m_selection.m_startPosition == -1 && m_selection.m_endPosition == -1)
			{
				m_selection.m_startPosition = m_caretPosition;
			}
			m_selection.m_endPosition = newCaretPosition;
		}
		else
		{
			m_selection.m_endPosition = m_selection.m_startPosition = -1;
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
			if (m_selection.m_startPosition == -1 && m_selection.m_endPosition == -1)
			{
				m_selection.m_startPosition = m_caretPosition;
			}
			m_selection.m_endPosition = newCaretPosition;
		}
		else
		{
			m_selection.m_endPosition = m_selection.m_startPosition = -1;
		}
		m_caretPosition = newCaretPosition;
		AdjustView();
	}

	void TextEditor::MoveCaretEnd()
	{
		size_t newCaretPosition = m_content.size();

		if (m_shiftPressed)
		{
			if (m_selection.m_startPosition == -1 && m_selection.m_endPosition == -1)
			{
				m_selection.m_startPosition = m_caretPosition;
			}
			m_selection.m_endPosition = newCaretPosition;
		}
		else
		{
			m_selection.m_endPosition = m_selection.m_startPosition = -1;
		}
		m_caretPosition = newCaretPosition;
		AdjustView();
	}

	void TextEditor::Delete()
	{
		if (m_selection.m_endPosition != m_selection.m_startPosition)
		{
			auto start = (std::min)(m_selection.m_startPosition, m_selection.m_endPosition);
			auto end = (std::max)(m_selection.m_startPosition, m_selection.m_endPosition);

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

			m_selection.m_endPosition = m_selection.m_startPosition = -1;
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
		if (m_caretPosition == 0 && m_selection.m_endPosition == m_selection.m_startPosition)
			return;

		if (m_selection.m_endPosition != m_selection.m_startPosition)
		{
			auto start = (std::min)(m_selection.m_startPosition, m_selection.m_endPosition);
			auto end = (std::max)(m_selection.m_startPosition, m_selection.m_endPosition);

			std::wstring stringToDelete{ m_content.data() + start, m_content.data() + end };
			m_content.erase(start, (end - start));
			int64_t caretPosition = m_caretPosition;
			if (m_caretPosition == end)
			{
				caretPosition -= (end - start);
			}

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

			m_selection.m_endPosition = m_selection.m_startPosition = -1;
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
		/*
		*
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->ClientSize.ToRectangle(), window->Appearance->BoxBackground, true);

		 */
		m_graphics.DrawRectangle(m_owner->ClientSize.ToRectangle(), GetBackgroundColor(), true);
		
		Size contentSize = GetContentTextExtent(m_caretPosition);
		auto one = m_owner->ToScale(1);
		auto two = m_owner->ToScale(2);
		auto three = m_owner->ToScale(3);

		bool enabled = GUI::IsWindowEnabled(m_owner);
		auto caretHeight = m_graphics.GetCaretHeight();
		int textOffset = (static_cast<int>(m_graphics.GetSize().Height - contentSize.Height) >> 1);
		if (m_selection.m_endPosition != m_selection.m_startPosition)
		{
			auto start = (std::min)(m_selection.m_startPosition, m_selection.m_endPosition);
			auto end = (std::max)(m_selection.m_startPosition, m_selection.m_endPosition);
			auto startTextExtent = m_graphics.GetTextExtent(m_content.substr(0, start));
			std::wstring selectionText{ m_content.data() + start, m_content.data() + end };
			auto endTextExtent = m_graphics.GetTextExtent(selectionText);
			
			m_graphics.DrawRectangle({ two + m_offsetView + static_cast<int>(startTextExtent.Width) , one + textOffset, endTextExtent.Width, caretHeight }, m_owner->Appearance->HighlightColor, true);
		}
		m_graphics.DrawString({ two + m_offsetView, one + textOffset }, m_content, enabled ? m_owner->Appearance->Foreground : m_owner->Appearance->BoxBorderDisabledColor);

		if (m_caret->IsVisible())
		{
			m_graphics.DrawLine({ two + m_offsetView + static_cast<int>(contentSize.Width), one + textOffset }, { two + m_offsetView + static_cast<int>(contentSize.Width), one + textOffset + static_cast<int>(caretHeight) }, m_owner->Appearance->Foreground2nd);
		}
		m_graphics.DrawRectangle(m_owner->ClientSize.ToRectangle(), enabled ? m_owner->Appearance->BoxBorderColor : m_owner->Appearance->BoxBorderDisabledColor, false);
	}

	bool TextEditor::IsEditable() const
	{
		return m_features.isEditable;
	}

	void TextEditor::SetEditable(bool isEditable)
	{
		m_features.isEditable = isEditable;
	}

	void TextEditor::SetCharFilter(std::function<bool(wchar_t)> predicate)
	{
		m_predicate = std::move(predicate);
	}

	bool TextEditor::Deselect()
	{
		m_selection.m_startPosition = m_selection.m_endPosition = -1;
		m_caretPosition = 0;

		AdjustView();
		return true;
	}

	bool TextEditor::SelectAll()
	{
		if (m_content.empty())
		{
			return false;
		}
		m_selection.m_startPosition = 0;
		m_selection.m_endPosition = m_content.size();
		m_caretPosition = m_selection.m_endPosition;

		AdjustView();
		return true;
	}

	void TextEditor::AdjustView()
	{
		auto contentSizeAtCaret = GetContentTextExtent(m_caretPosition);
		auto contentSize = GetContentTextExtent(m_content.size());
		const auto& ownerSize = m_owner->ClientSize;

		constexpr int adjustment = 4;
		bool needAdjustment = m_offsetView + static_cast<int>(contentSize.Width)  < static_cast<int>(ownerSize.Width) - adjustment || m_offsetView + (int)contentSizeAtCaret.Width < 0 || m_offsetView + (int)contentSizeAtCaret.Width > (int)ownerSize.Width - adjustment;
		if (needAdjustment)
		{
			if (m_offsetView + static_cast<int>(contentSize.Width) < static_cast<int>(ownerSize.Width) - adjustment)
			{
				m_offsetView = static_cast<int>(ownerSize.Width - contentSizeAtCaret.Width) - adjustment;
			}
			else if (m_offsetView + static_cast<int>(contentSizeAtCaret.Width) < 0)
			{
				m_offsetView = -static_cast<int>(contentSizeAtCaret.Width) + adjustment;
			}
			else
			{
				m_offsetView = static_cast<int>(ownerSize.Width - contentSizeAtCaret.Width) - adjustment;
			}
			m_offsetView = std::clamp(m_offsetView, -static_cast<int>(contentSize.Width), 0);
		}
	}

	Size TextEditor::GetContentTextExtent(size_t position) const
	{
		if (position == 0)
		{
			return { 0, m_graphics.GetTextExtent().Height };
		}
		
		return  m_graphics.GetTextExtent(m_content, position);
	}

	size_t TextEditor::GetPositionUnderMouse(const Point& mousePosition) const
	{
		size_t index = 0;
		
		int nearest = (std::numeric_limits<int>::max)();
		for (size_t i = 0; i <= m_content.size(); i++)
		{
			auto letterSize = m_graphics.GetTextExtent(m_content.substr(0, i));
			auto abs = std::abs(static_cast<int>(letterSize.Width) - mousePosition.X + m_offsetView);
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
		if (m_content.empty())
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

		p = std::max<int64_t>(p, 0);

		return p;
	}

	void TextEditor::EmitValueChanged() const
	{
		if (m_valueChangedCallback)
		{
			m_valueChangedCallback();
		}
	}

	Color TextEditor::GetBackgroundColor() const
	{
		return GUI::IsWindowEnabled(m_owner) ? m_owner->Appearance->BoxBackground :m_owner->Appearance->BoxPressedBackground;
	}
}
