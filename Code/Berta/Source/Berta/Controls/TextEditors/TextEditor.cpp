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
		m_caret = new Caret(owner, Size{1,0});

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
		if (m_caret)
		{
			delete m_caret;
			m_caret = nullptr;
		}
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
		if (m_content.empty())
		{
			return;
		}
		m_selectionMousePosition = args.Position;
		TextPosition clickedPos = GetPositionUnderMouse(args.Position);
		
		m_selection.m_isSelecting = true;
		if (m_shiftPressed)
		{
			m_selection.m_endPosition = clickedPos;
		}
		else
		{
			m_selection.Reset(clickedPos);
		}
		
		GUI::Capture(m_owner);
	}

	void TextEditor::OnMouseMove(const ArgMouse& args)
	{
		if (m_selection.m_isSelecting)
		{
			TextPosition newPosition = GetPositionUnderMouse(args.Position);
			m_selection.m_endPosition = newPosition;
			
			if (args.ButtonState.LeftButton && !m_selectionTimer.IsRunning() && (args.Position.X > static_cast<int>(m_editorArea.Width) || args.Position.X < 0))
			{
				m_selectionTimer.SetInterval(300);
				m_selectionTimer.Start();
				m_selectionDirection = args.Position.X < 0;
			}
			else if (args.ButtonState.LeftButton && m_selectionTimer.IsRunning() && !(args.Position.X > static_cast<int>(m_editorArea.Width) || args.Position.X < 0))
			{
				m_selectionTimer.Stop();
			}
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
		GUI::ReleaseCapture(m_owner);
		
		m_selection.m_endPosition = GetPositionUnderMouse(args.Position);
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
				InsertChar(args.Key);
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

		switch (args.Key)
		{
		case KeyboardKey::ArrowLeft:
			if (m_ctrlPressed)
			{
				auto nextWordPosition = GetPositionNextWord(m_selection.m_endPosition, -1);
				m_selection.m_startPosition = m_selection.m_endPosition = nextWordPosition;
			}
			else
				MoveCaretLeft(m_shiftPressed);
			break;

		case KeyboardKey::ArrowRight:
			if (m_ctrlPressed)
			{
				auto nextWordPosition = GetPositionNextWord(m_selection.m_endPosition, 1);
				m_selection.m_startPosition = m_selection.m_endPosition = nextWordPosition;
			}
			else
				MoveCaretRight(m_shiftPressed);
			break;
		
		case KeyboardKey::ArrowUp:
			MoveCaretUp(m_shiftPressed);
			break;
		
		case KeyboardKey::ArrowDown:
			MoveCaretDown(m_shiftPressed);
			break;
			
		case KeyboardKey::Enter:
			HandleEnter();
			break;

		//case KeyboardKey::A:
		//	if (m_ctrlPressed) return SelectAll();
		//	break;

		case KeyboardKey::Home:
			MoveCaretHome(m_shiftPressed);
			break;

		case KeyboardKey::End:
			MoveCaretEnd(m_shiftPressed);
			break;

		case KeyboardKey::Delete:
			HandleDelete();
			break;

		case KeyboardKey::Backspace:
			HandleBackspace();
			break;
		}
		return true;
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
		
		TextPosition clickPos = GetPositionUnderMouse(args.Position);
		const std::wstring& line = m_lines[clickPos.line];
		if (line.empty())
		{
			return true;
		}

		size_t start = clickPos.column;
		size_t end = clickPos.column;

		while (start > 0 && iswalnum(line[start - 1]))
		{
			start--;
		}
		
		while (end < line.size() && iswalnum(line[end]))
		{
			end++;
		}

		m_selection.m_startPosition = { clickPos.line, start };
		m_selection.m_endPosition = { clickPos.line, end };
    
		ActivateCaret();
		return true;
	}

	void TextEditor::ActivateCaret()
	{
		auto& currentLine = m_lines[m_selection.m_endPosition.line];
		auto extent = m_graphics.GetTextExtent(currentLine.substr(0, m_selection.m_endPosition.column));
    
		Point caretPos;
		caretPos.X = static_cast<int>(extent.Width) + m_offsetView.X;
		caretPos.Y = static_cast<int>(m_selection.m_endPosition.line * GetLineHeight()) + m_offsetView.Y;
		
		m_caret->SetPosition(caretPos);
		m_caret->Activate();
	}

	void TextEditor::DeactivateCaret()
	{
		m_caret->Deactivate();
	}

	void TextEditor::InsertChar(wchar_t chr)
	{
		if (!m_selection.IsEmpty())
		{
			DeleteRange(m_selection.m_startPosition, m_selection.m_endPosition);
		}
		TextPosition& position = m_selection.m_endPosition;
		auto& currentLine = m_lines[m_selection.m_endPosition.line];
    
		if (chr == L'\n')
		{
			std::wstring remainder = currentLine.substr(position.column);
			currentLine = currentLine.substr(0, position.column);
        
			m_lines.insert(m_lines.begin() + position.line + 1, remainder);
        
			position.line++;
			position.column = 0;
		}
		else
		{
			currentLine.insert(position.column, 1, chr);
			position.column++;
		}
		RecomputeWordWrap();
		m_selection.m_startPosition = position;
		
		//AdjustView();
		EmitValueChanged();
	}

	void TextEditor::MoveCaretLeft(bool select)
	{
		TextPosition& position = m_selection.m_endPosition;

		if (position.column > 0)
		{
			position.column--;
		}
		else if (position.line > 0)
		{
			position.line--;
			position.column = m_lines[position.line].size();
		}
		
		if (!select)
		{
			m_selection.m_startPosition = position;
		}
	}

	void TextEditor::MoveCaretHome(bool select)
	{
		size_t vIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
		m_selection.m_endPosition.column = m_visualLines[vIdx].charStart;

		if (!select)
		{
			m_selection.m_startPosition = m_selection.m_endPosition;
		}
	}

	void TextEditor::MoveCaretRight(bool select)
	{
		TextPosition& pos = m_selection.m_endPosition;
		const std::wstring& currentLine = m_lines[pos.line];

		if (pos.column < currentLine.size())
		{
			pos.column++;
		}
		else if (pos.line + 1 < m_lines.size())
		{
			pos.line++;
			pos.column = 0;
		}

		if (!select)
		{
			m_selection.m_startPosition = pos;
		}
	}

	void TextEditor::MoveCaretEnd(bool select)
	{
		size_t vIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
		m_selection.m_endPosition.column = m_visualLines[vIdx].charStart + m_visualLines[vIdx].charLength;

		if (!select)
		{
			m_selection.m_startPosition = m_selection.m_endPosition;
		}
	}

	void TextEditor::MoveCaretUp(bool select)
	{
		size_t vIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
		if (vIdx > 0)
		{
			const auto& currentV = m_visualLines[vIdx];
			const auto& targetV = m_visualLines[vIdx - 1];
			size_t offset = m_selection.m_endPosition.column - currentV.charStart;
        
			m_selection.m_endPosition.line = targetV.logicalLineIndex;
			m_selection.m_endPosition.column = targetV.charStart + (std::min)(offset, targetV.charLength);
		}
		if (!select)
		{
			m_selection.m_startPosition = m_selection.m_endPosition;
		}
		AdjustView();
	}

	void TextEditor::MoveCaretDown(bool select)
	{
		size_t vIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
		if (vIdx + 1 < m_visualLines.size())
		{
			const auto& currentV = m_visualLines[vIdx];
			const auto& targetV = m_visualLines[vIdx + 1];
			size_t offset = m_selection.m_endPosition.column - currentV.charStart;

			m_selection.m_endPosition.line = targetV.logicalLineIndex;
			m_selection.m_endPosition.column = targetV.charStart + (std::min)(offset, targetV.charLength);
		}
		if (!select)
		{
			m_selection.m_startPosition = m_selection.m_endPosition;
		}
		AdjustView();
	}

	void TextEditor::HandleDelete()
	{
		if (!m_selection.IsEmpty())
		{
			DeleteRange(m_selection.m_startPosition, m_selection.m_endPosition);
			return;
		}
		TextPosition& position = m_selection.m_endPosition;
		auto& currentLine = m_lines[position.line];

		if (position.column < currentLine.size())
		{
			currentLine.erase(position.column, 1);
		}
		else if (position.line + 1 < m_lines.size())
		{
			m_lines[position.line] += m_lines[position.line + 1];
			m_lines.erase(m_lines.begin() + position.line + 1);
		}

		m_selection.m_startPosition = position;
		//AdjustView();
		//EmitValueChanged();
	}

	void TextEditor::HandleBackspace()
	{
		if (!m_selection.IsEmpty())
		{
			DeleteRange(m_selection.Min(), m_selection.Max());
			return;
		}
		
		TextPosition& pos = m_selection.m_endPosition;
		if (pos.column == 0 && pos.line > 0)
		{
			size_t targetLine = pos.line - 1;
			size_t newCol = m_lines[targetLine].size();
        
			m_lines[targetLine] += m_lines[pos.line];
			m_lines.erase(m_lines.begin() + pos.line);
        
			pos.line = targetLine;
			pos.column = newCol;
        
			RecomputeWordWrap();
		}
		else if (pos.column > 0)
		{
			m_lines[pos.line].erase(pos.column - 1, 1);
			pos.column--;
		}
		else if (pos.line > 0)
		{
			size_t prevLine = pos.line - 1;
			pos.column = m_lines[prevLine].size();
			m_lines[prevLine] += m_lines[pos.line];
			m_lines.erase(m_lines.begin() + pos.line);
			pos.line = prevLine;
		}
		m_selection.m_startPosition = pos;
	}

	void TextEditor::HandleEnter()
	{
		if (!m_features.isMultiLines)
			return;

		InsertChar(L'\n');
	}

	void TextEditor::DeleteRange(TextPosition start, TextPosition end)
	{
		if (start == end)
			return;

		if (end < start)
			std::swap(start, end);

		if (start.line == end.line)
		{
			m_lines[start.line].erase(start.column, end.column - start.column);
		} 
		else
		{
			std::wstring head = m_lines[start.line].substr(0, start.column);
			std::wstring tail = m_lines[end.line].substr(end.column);

			m_lines[start.line] = head + tail;
			m_lines.erase(m_lines.begin() + start.line + 1, m_lines.begin() + end.line + 1);
		}

		m_selection.Reset(start);
	}

	size_t TextEditor::GetVisualLineIndexFromPos(TextPosition position) const
	{
		for (size_t i = 0; i < m_visualLines.size(); ++i)
		{
			const auto& vl = m_visualLines[i];
			if (vl.logicalLineIndex == position.line && position.column >= vl.charStart && position.column <= vl.charStart + vl.charLength)
			{
				if (position.column == vl.charStart + vl.charLength && i + 1 < m_visualLines.size())
				{
					if (m_visualLines[i+1].logicalLineIndex == position.line) 
						continue;
				}
				return i;
			}
		}
		return 0;
	}

	uint32_t TextEditor::GetLineHeight() const
	{
		return m_graphics.GetTextExtent("Ay").Height;
	}

	void TextEditor::SetContent(const std::wstring& newContent)
	{
		m_content = newContent;
		m_lines.clear();
    
		if (m_content.empty())
		{
			m_lines.push_back(L"");
		}
		else
		{
			std::size_t start = 0, end;
			while ((end = m_content.find(L'\n', start)) != std::wstring::npos)
			{
				std::wstring line = m_content.substr(start, end - start);
				
				if (!line.empty() && line.back() == L'\r')
				{
					line.pop_back();
				}
				m_lines.push_back(line);
				start = end + 1;
			}
			m_lines.push_back(m_content.substr(start));
		}

		m_selection.Reset({ 0, 0 });
		m_offsetView = {0, 0};
    
		RecomputeWordWrap();
	}

	void TextEditor::SetContent(const std::string& newContent)
	{
		SetContent(StringUtils::Convert(newContent));
	}
	
	void TextEditor::SetEditorArea(const Rectangle& area)
	{
		m_editorArea = area;
	}

	void TextEditor::Render()
	{
		bool enabled = GUI::IsWindowEnabled(m_owner);
		m_graphics.DrawRectangle(m_owner->ClientSize.ToRectangle(), GetBackgroundColor(), true);
		
		auto lineHeight = GetLineHeight();
		auto one = m_owner->ToScale(1);
		auto two = m_owner->ToScale(2);
		
		TextPosition s = m_selection.Min();
		TextPosition e = m_selection.Max();
		
		for (const auto& vl : m_visualLines)
		{
			const int drawY = static_cast<int>(vl.y) - m_offsetView.Y;
			if (drawY + lineHeight < 0 || drawY > static_cast<int>(m_editorArea.Height))
			{
				continue;
			}
			
			std::wstring fragment = m_lines[vl.logicalLineIndex].substr(vl.charStart, vl.charLength);
			int drawX = -m_offsetView.X;
			if (s != e && vl.logicalLineIndex >= s.line && vl.logicalLineIndex <= e.line)
			{
				size_t selStartInV = (vl.logicalLineIndex == s.line) ? (std::max)(vl.charStart, s.column) : vl.charStart;
				size_t selEndInV = (vl.logicalLineIndex == e.line) ? (std::min)(vl.charStart + vl.charLength, e.column) : (vl.charStart + vl.charLength);

				if (selStartInV < selEndInV)
				{
					auto x1 = m_graphics.GetTextExtent(fragment.substr(0, selStartInV - vl.charStart)).Width;
					auto w = m_graphics.GetTextExtent(fragment.substr(selStartInV - vl.charStart, selEndInV - selStartInV)).Width;
					m_graphics.DrawRectangle(Rectangle{static_cast<int>(x1), drawY, (uint32_t)(w + one), lineHeight}, Color(0, 120, 215, 128), true);
				}
			}
			m_graphics.DrawString({ m_editorArea.X, drawY + m_editorArea.Y }, fragment, m_owner->Appearance->Foreground);
			
			//Caret
			if (m_selection.m_endPosition.line == vl.logicalLineIndex && 
				m_selection.m_endPosition.column >= vl.charStart && 
				m_selection.m_endPosition.column <= vl.charStart + vl.charLength)
			{
				auto caretX = m_graphics.GetTextExtent(fragment.substr(0, m_selection.m_endPosition.column - vl.charStart)).Width;
				m_caret->SetPosition({ drawX + static_cast<int>(caretX), drawY });
			}
		}
		
		if (m_caret->IsVisible())
		{
			auto caretHeight= m_graphics.GetCaretHeight();
			m_graphics.DrawLine({ m_editorArea.X + m_caret->GetPosition().X, m_editorArea.Y +  m_caret->GetPosition().Y }, { m_editorArea.X + m_caret->GetPosition().X, m_editorArea.Y + m_caret->GetPosition().Y + static_cast<int>(caretHeight) }, m_owner->Appearance->Foreground2nd);
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

	void TextEditor::SetMultiline(bool enable)
	{
		if (m_features.isMultiLines == enable)
			return;
		
		m_features.isMultiLines = enable;
	}

	void TextEditor::SetWordWrap(bool enable)
	{
		if (m_features.wordWrap == enable)
			return;
		
		m_features.wordWrap = enable;
		RecomputeWordWrap();
	}

	void TextEditor::SetCharFilter(std::function<bool(wchar_t)> predicate)
	{
		m_predicate = std::move(predicate);
	}

	bool TextEditor::Deselect()
	{
		if (m_selection.IsEmpty())
		{
			return false;
		}
		m_selection.Reset(m_selection.m_endPosition);
		
		return true;
	}

	bool TextEditor::SelectAll()
	{
		if (m_content.empty())
		{
			return false;
		}
		/*m_selection.m_startPosition = 0;
		m_selection.m_endPosition = m_content.size();
		m_caretPosition = m_selection.m_endPosition;

		AdjustView();*/
		return true;
	}

	void TextEditor::AdjustView()
	{
		auto lineHeight = GetLineHeight();
		auto viewHeight = m_editorArea.Height;
    
		size_t vIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
		auto caretY = m_visualLines[vIdx].y;

		if (caretY + lineHeight - m_offsetView.Y > viewHeight)
		{
			m_offsetView.Y = static_cast<int>(caretY + lineHeight - viewHeight);
		}
		else if (caretY - m_offsetView.Y < 0)
		{
			m_offsetView.Y = static_cast<int>(caretY);
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

	TextPosition TextEditor::GetPositionUnderMouse(const Point& mousePosition) const
	{
		auto lineHeight = GetLineHeight();
    
		auto worldX = mousePosition.X + m_offsetView.X;
		auto worldY = mousePosition.Y + m_offsetView.Y;

		for (const auto& vl : m_visualLines)
		{
			if (worldY >= static_cast<int>(vl.y) && worldY < static_cast<int>(vl.y + lineHeight))
			{
				const std::wstring& line = m_lines[vl.logicalLineIndex];
				std::wstring fragment = line.substr(vl.charStart, vl.charLength);
            
				size_t bestCol = 0;
				size_t minDistance = 999999;
            
				for (size_t i = 0; i <= fragment.size(); ++i)
				{
					auto width = m_graphics.GetTextExtent(fragment.substr(0, i)).Width;
					size_t dist = std::abs(static_cast<int>(width) - worldX);
					if (dist < minDistance)
					{
						minDistance = dist;
						bestCol = i;
					}
				}
				return { vl.logicalLineIndex, vl.charStart + bestCol };
			}
		}
		return { m_lines.size() - 1, m_lines.back().size() };
	}

	TextPosition TextEditor::GetPositionNextWord(TextPosition currentPosition, int direction) const
	{
		if (m_content.empty())
		{
			return currentPosition; //0
		}

		const std::wstring& line = m_lines[currentPosition.line];
		if (direction > 0)
		{
			if (currentPosition.column >= line.size())
				{
				if (currentPosition.line + 1 < m_lines.size())
				{
					return { currentPosition.line + 1, 0 };
				}
				
				return currentPosition;
			}
        
			size_t p = currentPosition.column;
			while (p < line.size() && iswspace(line[p])) 
				p++;

			while (p < line.size() && !iswspace(line[p])) 
				p++;
        
			return { currentPosition.line, p };
		}
		
		if (currentPosition.column == 0)
		{
			if (currentPosition.line > 0)
			{
				return { currentPosition.line - 1, m_lines[currentPosition.line - 1].size() };
			}
			
			return currentPosition;
		}

		size_t p = currentPosition.column;
		while (p > 0 && iswspace(line[p - 1]))
			p--;
		
		while (p > 0 && !iswspace(line[p - 1]))
			p--;
    
		return { currentPosition.line, p };
		
	}

	void TextEditor::RecomputeWordWrap()
	{
		m_visualLines.clear();
		if (m_lines.empty())
		{
			return;
		}

		auto maxWidth = m_editorArea.Width - 10u;
		auto lineHeight = GetLineHeight();
		uint32_t currentY = 0;

		for (size_t i = 0; i < m_lines.size(); ++i)
			{
			const std::wstring& lineText = m_lines[i];
        
			if (lineText.empty())
			{
				m_visualLines.push_back({ i, 0, 0, currentY });
				currentY += lineHeight;
				continue;
			}

			if (!m_features.wordWrap)
				{
				m_visualLines.push_back({ i, 0, lineText.size(), currentY });
				currentY += lineHeight;
			}
			else
			{
				size_t start = 0;
				while (start < lineText.size())
				{
					size_t count = 0;
					// TODO: Binary search
					while (start + count < lineText.size())
					{
						auto w = m_graphics.GetTextExtent(lineText.substr(start, count + 1)).Width;
						if (w > maxWidth && count > 0)
						{
							break;
						}
						count++;
					}

					m_visualLines.push_back({ i, start, count, currentY });
					currentY += lineHeight;
					start += count;
				}
			}
		}
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
