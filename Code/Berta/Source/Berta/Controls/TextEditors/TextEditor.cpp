/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "TextEditor.h"

#include <algorithm>
#include <cwctype>

#include "Berta/API/PlatformAPI.h"
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
			BT_CORE_DEBUG << "timer tick... isSelecting=" << m_selection.m_isSelecting << ". m_offsetView " << m_offsetView << ". m_selectionDirection " << m_selectionDirection << std::endl;
			if (!m_selection.m_isSelecting)
			{
				return;
			}
			constexpr int scrollSpeed = 20;
			auto savedOffsetView = m_offsetView;
			
			Size contentSize = GetContentTextExtent();
			const int maxScrollX = std::max<int>(0, static_cast<int>(contentSize.Width) - static_cast<int>(m_editorArea.Width));
			const int maxScrollY = std::max<int>(0, static_cast<int>(contentSize.Height) - static_cast<int>(m_editorArea.Height));
			
			if (m_selectionDirection.Y == -1)
			{
				m_offsetView.Y = std::min<int>(maxScrollY, m_offsetView.Y + scrollSpeed);
			}
			else if (m_selectionDirection.Y == 1)
			{
				m_offsetView.Y = std::max<int>(0, m_offsetView.Y - scrollSpeed);
			}
			
			if (!m_features.wordWrap)
			{
				if (m_selectionDirection.X == -1)
				{
					m_offsetView.X = std::min<int>(maxScrollX, m_offsetView.X + scrollSpeed);
				}
				else if (m_selectionDirection.X == 1)
				{
					m_offsetView.X = std::max<int>(0, m_offsetView.X - scrollSpeed);
				}
			}
			
			if (savedOffsetView != m_offsetView)
			{
				m_selection.m_endPosition = GetPositionUnderMouse(m_lastMousePosition);
				//AdjustView();
				GUI::UpdateWindow(m_owner);
			}
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
		if (m_lines.empty())
		{
			return;
		}
		m_lastMousePosition = args.Position;
		m_selectionMousePosition = args.Position;
		TextPosition clickedPos = GetPositionUnderMouse(args.Position);
		
		m_selection.m_isSelecting = args.ButtonState.LeftButton;
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
			
			bool selectionTimerRunning = m_selectionTimer.IsRunning();
			bool insideEditorArea = m_editorArea.IsInside(args.Position);
			if (!selectionTimerRunning && !insideEditorArea)
			{
				std::cout << "start timmeerr. mouse pos = " << args.Position <<". area="<<m_editorArea << std::endl;
				m_selectionDirection.X = args.Position.X < m_editorArea.X ? 1 : (args.Position.X >= static_cast<int>(m_editorArea.Width) + m_editorArea.X ? -1 : 0);
				m_selectionDirection.Y = args.Position.Y < m_editorArea.Y ? 1 : (args.Position.Y >= static_cast<int>(m_editorArea.Height) + m_editorArea.Y ? -1 : 0);
				m_selectionTimer.SetInterval(90);
				m_selectionTimer.Start();
			}
			else if (selectionTimerRunning)
			{
				if (insideEditorArea)
				{
					std::cout << "stop timmeerr" << std::endl;
					m_selectionDirection = {0, 0};
					m_selectionTimer.Stop();
				}
				else
				{
					m_selectionDirection.X = args.Position.X < m_editorArea.X ? 1 : (args.Position.X >= static_cast<int>(m_editorArea.Width) + m_editorArea.X ? -1 : 0);
					m_selectionDirection.Y = args.Position.Y < m_editorArea.Y ? 1 : (args.Position.Y >= static_cast<int>(m_editorArea.Height) + m_editorArea.Y ? -1 : 0);
				}
			}
		}
		
		m_lastMousePosition = args.Position;
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
		if (m_lines.empty())
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
			if (std::iswprint(args.Key))
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
		m_shiftPressed = m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_ctrlPressed = m_ctrlPressed || args.Key == KeyboardKey::Control;

		auto savedStartPosition = m_selection.m_startPosition;
		auto savedEndPosition = m_selection.m_endPosition;
		
		switch (args.Key)
		{
		case KeyboardKey::ArrowLeft:
			MoveCaretLeft(m_ctrlPressed, m_shiftPressed);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;

		case KeyboardKey::ArrowRight:
			MoveCaretRight(m_ctrlPressed, m_shiftPressed);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;
		
		case KeyboardKey::ArrowUp:
			MoveCaretUp(m_shiftPressed);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;
		
		case KeyboardKey::ArrowDown:
			MoveCaretDown(m_shiftPressed);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;
			
		case KeyboardKey::Enter:
			HandleEnter();
			break;
			
		//case KeyboardKey::A:
		//	break;

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
			redraw = true;
			break;

		case KeyboardKey::Backspace:
			HandleBackspace();
			redraw = true;
			break;
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
		if (m_lines.empty())
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

	void TextEditor::OnResize(ArgResize args)
	{
		RecomputeWordWrap();
		AdjustView();
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

	void TextEditor::InsertChar(const wchar_t chr)
	{
		if (!m_selection.IsEmpty())
		{
			DeleteRange(m_selection.m_startPosition, m_selection.m_endPosition);
			m_selection.Reset(m_selection.m_startPosition);
		}
		
		TextPosition& position = m_selection.m_endPosition;
		auto& currentLine = m_lines[m_selection.m_endPosition.line];
    
		if (chr == L'\n')
		{
			std::wstring remainder = currentLine.substr(position.column);
			currentLine = currentLine.substr(0, position.column);
        
			m_lines.insert(m_lines.begin() + position.line + 1, remainder);
			UpdateLinesIncremental(position.line, 1);
        
			position.line++;
			position.column = 0;
		}
		else
		{
			currentLine.insert(position.column, 1, chr);
			UpdateLinesIncremental(position.line, 0);
			position.column++;
		}
		//RecomputeWordWrap();
		m_selection.m_startPosition = position;
		
		AdjustView();
		EmitValueChanged();
	}

	void TextEditor::MoveCaretLeft(bool wordJump, bool select)
	{
		TextPosition& position = m_selection.m_endPosition;

		if (position.column > 0)
		{
			if (wordJump)
			{
				position = GetPositionNextWord(m_selection.m_endPosition, -1);
			}
			else
			{
				position.column--;
			}
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
		
		AdjustView();
	}

	void TextEditor::MoveCaretHome(bool select)
	{
		size_t vIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
		m_selection.m_endPosition.column = m_visualLines[vIdx].charStart;

		if (!select)
		{
			m_selection.m_startPosition = m_selection.m_endPosition;
		}
		AdjustView();
	}

	void TextEditor::MoveCaretRight(bool wordJump, bool select)
	{
		TextPosition& position = m_selection.m_endPosition;
		const std::wstring& currentLine = m_lines[position.line];

		if (position.column < currentLine.size())
		{
			if (wordJump)
			{
				position = GetPositionNextWord(position, 1);
			}
			else
			{
				position.column++;
			}
		}
		else if (position.line + 1 < m_lines.size())
		{
			position.line++;
			position.column = 0;
		}

		if (!select)
		{
			m_selection.m_startPosition = position;
		}
		AdjustView();
	}

	void TextEditor::MoveCaretEnd(bool select)
	{
		size_t vIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
		m_selection.m_endPosition.column = m_visualLines[vIdx].charStart + m_visualLines[vIdx].charLength;

		if (!select)
		{
			m_selection.m_startPosition = m_selection.m_endPosition;
		}
		AdjustView();
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
			m_selection.m_endPosition.column = targetV.charStart + std::min<size_t>(offset, targetV.charLength);
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
			RecomputeWordWrap();
			AdjustView();
			EmitValueChanged();
			return;
		}
		TextPosition& position = m_selection.m_endPosition;
		auto& currentLine = m_lines[position.line];
		bool changed = false;
		if (position.column < currentLine.size())
		{
			currentLine.erase(position.column, 1);
			changed = true;
		}
		else if (position.line + 1 < m_lines.size())
		{
			m_lines[position.line] += m_lines[position.line + 1];
			m_lines.erase(m_lines.begin() + position.line + 1);
			changed = true;
		}

		m_selection.m_startPosition = position;
		if (!changed)
		{
			return;
		}
		RecomputeWordWrap();
		AdjustView();
		EmitValueChanged();
	}

	void TextEditor::HandleBackspace()
	{
		if (!m_selection.IsEmpty())
		{
			DeleteRange(m_selection.Min(), m_selection.Max());
			RecomputeWordWrap();
			AdjustView();
			EmitValueChanged();
			return;
		}
		
		bool changed = false;
		TextPosition& pos = m_selection.m_endPosition;
		if (pos.column == 0 && pos.line > 0)
		{
			size_t targetLine = pos.line - 1;
			size_t newColumn = m_lines[targetLine].size();
        
			m_lines[targetLine] += m_lines[pos.line];
			m_lines.erase(m_lines.begin() + pos.line);
			UpdateLinesIncremental(targetLine, -1); // Incremental!
        
			pos.line = targetLine;
			pos.column = newColumn;
			changed = true;
		}
		else if (pos.column > 0)
		{
			m_lines[pos.line].erase(pos.column - 1, 1);
			UpdateLinesIncremental(pos.line, 0);
			pos.column--;
			changed = true;
		}
		else if (pos.line > 0)
		{
			size_t prevLine = pos.line - 1;
			pos.column = m_lines[prevLine].size();
			m_lines[prevLine] += m_lines[pos.line];
			m_lines.erase(m_lines.begin() + pos.line);
			pos.line = prevLine;
			changed = true;
		}
		m_selection.m_startPosition = pos;
		
		if (!changed)
		{
			return;
		}
		RecomputeWordWrap();
		AdjustView();
		EmitValueChanged();
	}

	void TextEditor::HandleEnter()
	{
		if (!m_features.isMultiLines)
		{
			return;
		}

		InsertChar(L'\n');
	}

	void TextEditor::DeleteRange(TextPosition start, TextPosition end)
	{
		if (start == end)
		{
			return;
		}

		if (end < start)
		{
			std::swap(start, end);
		}

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
		if (m_visualLines.empty())
		{
			return 0;
		}
		//binary search
		const auto it = std::lower_bound(m_visualLines.begin(), m_visualLines.end(), position.line,
			[](const VisualLine& vl, size_t lineIdx)
			{
				return vl.logicalLineIndex < lineIdx;
			});
		
		if (it == m_visualLines.end() || it->logicalLineIndex != position.line)
		{
			return m_visualLines.size() - 1;
		}

		size_t firstVisualOfLogical = std::distance(m_visualLines.begin(), it);
		for (size_t i = firstVisualOfLogical; i < m_visualLines.size(); ++i)
		{
			const auto& vl = m_visualLines[i];
			if (vl.logicalLineIndex != position.line)
			{
				break;
			}

			if (position.column >= vl.charStart && position.column <= vl.charStart + vl.charLength)
			{
				if (position.column == vl.charStart + vl.charLength && i + 1 < m_visualLines.size())
				{
					if (m_visualLines[i + 1].logicalLineIndex == position.line)
					{
						continue;
					}
				}
				return i;
			}
		}

		return firstVisualOfLogical;
	}

	uint32_t TextEditor::GetLineHeight() const
	{
		return m_graphics.GetTextExtent("Ay").Height;
	}

	std::wstring TextEditor::GetContent() const
	{
		if (m_lines.empty())
		{
			return L"";
		}
		std::wstring content = m_lines[0];
		for (size_t i = 1; i < m_lines.size(); ++i)
		{
			content += L"\r\n" + m_lines[i];
		}
		return content;
	}

	void TextEditor::SetContent(const std::wstring& newContent)
	{
		m_lines.clear();
		if (newContent.empty())
		{
			m_lines.emplace_back(L"");
		}
		else
		{
			std::size_t start = 0, end;
			while ((end = newContent.find(L'\n', start)) != std::wstring::npos)
			{
				std::wstring line = newContent.substr(start, end - start);
				
				if (!line.empty() && line.back() == L'\r')
				{
					line.pop_back();
				}
				m_lines.emplace_back(line);
				start = end + 1;
			}
			m_lines.emplace_back(newContent.substr(start));
		}

		m_selection.Reset({ 0, 0 });
		m_offsetView = {0, 0};
    
		RecomputeWordWrap();
	}

	void TextEditor::SetContent(const std::string& newContent)
	{
		SetContent(StringUtils::Convert(newContent));
	}

	std::wstring TextEditor::GetSelectedText() const
	{
		TextPosition start = m_selection.Min();
		TextPosition end = m_selection.Max();

		if (start == end)
		{
			return L"";
		}
		
		std::wstring result;
		for (size_t i = start.line; i <= end.line; ++i)
		{
			std::wstring_view lineView = m_lines[i];
			size_t startCol = (i == start.line) ? start.column : 0;
			size_t endCol = (i == end.line) ? end.column : lineView.size();

			result += lineView.substr(startCol, endCol - startCol);

			if (i < end.line)
			{
				result += L"\r\n";
			}
		}
		return result;
	}

	void TextEditor::Copy()
	{
		std::wstring selectedText = GetSelectedText();
		if (!selectedText.empty())
		{
			return;
		}
		Platform::SetClipboardText(selectedText, m_owner->RootHandle);
	}

	void TextEditor::Cut()
	{
		if (m_selection.m_startPosition == m_selection.m_endPosition)
		{
			return;
		}
		Copy();
		DeleteRange(m_selection.m_startPosition, m_selection.m_endPosition);
	}

	void TextEditor::Paste()
	{
		std::wstring clipboardText;
		if (!Platform::GetClipboardText(clipboardText) || clipboardText.empty())
		{
			return;
		}

		if (m_selection.m_startPosition != m_selection.m_endPosition)
		{
			DeleteRange(m_selection.m_startPosition, m_selection.m_endPosition);
		}

		TextPosition pos = m_selection.m_endPosition;
    
		std::vector<std::wstring> newLines;
		size_t start = 0, end;
		while ((end = clipboardText.find(L"\r\n", start)) != std::wstring::npos)
		{
			newLines.emplace_back(clipboardText.substr(start, end - start));
			start = end + 2;
		}
		newLines.emplace_back(clipboardText.substr(start));

		if (newLines.size() == 1)
		{
			m_lines[pos.line].insert(pos.column, newLines[0]);
			pos.column += newLines[0].size();
		}
		else
		{
			std::wstring currentLine = m_lines[pos.line];
			std::wstring prefix = currentLine.substr(0, pos.column);
			std::wstring suffix = currentLine.substr(pos.column);

			m_lines[pos.line] = prefix + newLines[0];
        
			m_lines.insert(m_lines.begin() + pos.line + 1, newLines.begin() + 1, newLines.end());
			
			pos.line += newLines.size() - 1;
			pos.column = newLines.back().size();
			m_lines[pos.line] += suffix;
		}

		RecomputeWordWrap();
		m_selection.Reset(pos);
		AdjustView();
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
		int viewportTop = m_offsetView.Y;
		int viewportBottom = m_offsetView.Y + static_cast<int>(m_editorArea.Height);

		size_t firstLineIndex = GetFirstVisibleVisualLine();
		
		auto one = m_owner->ToScale(1);
		auto two = m_owner->ToScale(2);
		
		TextPosition s = m_selection.Min();
		TextPosition e = m_selection.Max();
		
		for (size_t i = firstLineIndex; i < m_visualLines.size(); ++i)
		{
			const auto& vl = m_visualLines[i];
			const int vlPosY = static_cast<int>(vl.y);
			if (vlPosY > viewportBottom)
			{
				break; 
			}
			
			const int drawX = -m_offsetView.X;
			const int drawY = vlPosY - viewportTop + m_editorArea.Y;
			if (drawY + static_cast<int>(lineHeight) < m_editorArea.Y || drawY > static_cast<int>(m_editorArea.Height) + m_editorArea.Y)
			{
				continue;
			}
			
			std::wstring_view fragment = std::wstring_view(m_lines[vl.logicalLineIndex]).substr(vl.charStart, vl.charLength);
			
			if (s != e && vl.logicalLineIndex >= s.line && vl.logicalLineIndex <= e.line)
			{
				size_t selStartInV = (vl.logicalLineIndex == s.line) ? (std::max)(vl.charStart, s.column) : vl.charStart;
				size_t selEndInV = (vl.logicalLineIndex == e.line) ? (std::min)(vl.charStart + vl.charLength, e.column) : (vl.charStart + vl.charLength);

				if (selStartInV < selEndInV)
				{
					auto x1 = m_graphics.GetTextExtent(fragment.substr(0, selStartInV - vl.charStart)).Width;
					auto w = m_graphics.GetTextExtent(fragment.substr(selStartInV - vl.charStart, selEndInV - selStartInV)).Width;
					m_graphics.DrawRectangle(Rectangle{static_cast<int>(x1) + drawX + m_editorArea.X, drawY + m_editorArea.Y, (uint32_t)(w + one), lineHeight}, Color(0, 120, 215, 128), true);
				}
			}
			m_graphics.DrawString({ drawX + m_editorArea.X, drawY + m_editorArea.Y }, fragment, m_owner->Appearance->Foreground);
			
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
			m_graphics.DrawLine({ m_editorArea.X + m_caret->GetPosition().X, m_editorArea.Y + m_caret->GetPosition().Y }, { m_editorArea.X + m_caret->GetPosition().X, m_editorArea.Y + m_caret->GetPosition().Y + static_cast<int>(caretHeight) }, m_owner->Appearance->Foreground2nd);
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
		RecomputeWordWrap();
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

	TextPosition TextEditor::GetEndPosition() const
	{
		return m_selection.m_endPosition;
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
		if (m_lines.empty())
		{
			return false;
		}
		m_selection.m_startPosition = { 0, 0 };
		m_selection.m_endPosition = { m_lines.size() - 1, m_lines.back().size() };
    
		AdjustView();
		return true;
	}

	void TextEditor::AdjustView()
	{
		auto viewHeight = static_cast<int>(m_editorArea.Height);
		auto viewWidth = static_cast<int>(m_editorArea.Width);
		
		size_t vIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
		if (vIdx >= m_visualLines.size())
		{
			return;
		}
		
		const auto& vl = m_visualLines[vIdx];
		if (m_features.wordWrap || m_features.isMultiLines)
		{
			auto lineHeight = static_cast<int>(GetLineHeight());
			int caretY = static_cast<int>(vl.y);
			
			if (caretY + lineHeight - m_offsetView.Y > viewHeight)
			{
				m_offsetView.Y = caretY + lineHeight - viewHeight;
			}
			else if (caretY - m_offsetView.Y < 0)
			{
				m_offsetView.Y = caretY;
			}
		}
		else
		{
			m_offsetView.Y = 0;
		}
		
		if (!m_features.wordWrap)
		{
			std::wstring textToCaret = m_lines[vl.logicalLineIndex].substr(vl.charStart, m_selection.m_endPosition.column - vl.charStart);
			int caretX = static_cast<int>(m_graphics.GetTextExtent(textToCaret).Width);

			if (caretX > m_offsetView.X + viewWidth)
			{
				m_offsetView.X = caretX - viewWidth;
			}
			else if (caretX - m_offsetView.X < 0)
			{
				m_offsetView.X = caretX;
			}
		}
		else
		{
			m_offsetView.X = 0;
		}
	}

	TextPosition TextEditor::GetPositionUnderMouse(const Point& mousePosition) const
	{
		if (m_visualLines.empty())
		{
			return { 0, 0 };
		}

		int localX = mousePosition.X - static_cast<int>(m_editorArea.X) + m_offsetView.X;
		int localY = mousePosition.Y - static_cast<int>(m_editorArea.Y) + m_offsetView.Y;
		
		auto lineHeight = GetLineHeight();
		const VisualLine* targetVL = &m_visualLines.back();

		localY = std::max<int>(localY, 0);

		for (const auto& vl : m_visualLines)
		{
			if (localY >= static_cast<int>(vl.y) && localY < static_cast<int>(vl.y + lineHeight))
			{
				targetVL = &vl;
				break;
			}
		}
		
		const std::wstring& lineText = m_lines[targetVL->logicalLineIndex];
		if (lineText.empty())
		{
			return { targetVL->logicalLineIndex, 0 };
		}

		size_t low = 0;
		size_t high = targetVL->charLength;
		size_t foundOffset = 0;

		std::wstring_view visualPart = std::wstring_view(lineText).substr(targetVL->charStart, targetVL->charLength);

		while (low <= high)
		{
			size_t mid = low + (high - low) / 2;
			uint32_t width = m_graphics.GetTextExtent(visualPart.substr(0, mid)).Width;

			if (width <= static_cast<uint32_t>(localX))
			{
				foundOffset = mid;
				low = mid + 1;
			}
			else
			{
				high = mid - 1;
			}
		}

		if (foundOffset < targetVL->charLength)
		{
			uint32_t widthBefore = m_graphics.GetTextExtent(visualPart.substr(0, foundOffset)).Width;
			uint32_t widthAfter = m_graphics.GetTextExtent(visualPart.substr(0, foundOffset + 1)).Width;
			uint32_t halfChar = widthBefore + (widthAfter - widthBefore) / 2;

			if (static_cast<uint32_t>(localX) > halfChar)
			{
				foundOffset++;
			}
		}

		return { targetVL->logicalLineIndex, targetVL->charStart + foundOffset };
	}

	TextPosition TextEditor::GetPositionNextWord(TextPosition currentPosition, int direction) const
	{
		if (m_visualLines.empty())
		{
			return currentPosition;
		}

		size_t vIdx = GetVisualLineIndexFromPos(currentPosition);
		TextPosition pos = currentPosition;

		if (direction > 0)
		{
			while (vIdx < m_visualLines.size())
			{
				const auto& vl = m_visualLines[vIdx];
				const std::wstring& lineText = m_lines[vl.logicalLineIndex];
				size_t endLimit = vl.charStart + vl.charLength;

				if (pos.column >= endLimit)
				{
					vIdx++;
					if (vIdx < m_visualLines.size())
					{
						pos.line = m_visualLines[vIdx].logicalLineIndex;
						pos.column = m_visualLines[vIdx].charStart;

						if (m_visualLines[vIdx].charLength == 0)
						{
							continue;
						}
					} 
					else
					{
						break;
					}
				}

				size_t p = pos.column;

				while (p < endLimit && iswspace(lineText[p]))
				{
					p++;
				}

				if (p < endLimit)
				{
					while (p < endLimit && !iswspace(lineText[p]))
					{
						p++;
					}
					return { vl.logicalLineIndex, p };
				}
				pos.column = endLimit;
			}
		}
		else 
		{
			while (true)
			{
				const auto& vl = m_visualLines[vIdx];
				const std::wstring& lineText = m_lines[vl.logicalLineIndex];
				size_t startLimit = vl.charStart;

				if (pos.column <= startLimit)
				{
					if (vIdx > 0)
					{
						vIdx--;
						pos.line = m_visualLines[vIdx].logicalLineIndex;
						pos.column = m_visualLines[vIdx].charStart + m_visualLines[vIdx].charLength;
						if (m_visualLines[vIdx].charLength == 0)
						{
							continue;
						}
					}
					else
					{
						break;
					}
				}

				size_t p = pos.column;
				while (p > startLimit && iswspace(lineText[p - 1]))
				{
					p--;
				}
				if (p > startLimit)
				{
					while (p > startLimit && !iswspace(lineText[p - 1]))
					{
						p--;
					}
					return { vl.logicalLineIndex, p };
				}

				pos.column = startLimit;
			}
		}

		return pos;
	}

	Size TextEditor::GetContentTextExtent() const
	{
		uint32_t totalHeight = static_cast<uint32_t>(m_visualLines.size() * GetLineHeight());
    
		return { m_cachedMaxWidth, totalHeight };
	}

	void TextEditor::RecomputeWordWrap()
	{
		m_visualLines.clear();
		m_cachedMaxWidth = 0;
		uint32_t currentY = 0;

		for (size_t i = 0; i < m_lines.size(); ++i) {
			ComputeVisualLinesForLogicalLine(i, currentY, m_visualLines);
		}
    
		if (m_features.wordWrap) m_cachedMaxWidth = m_editorArea.Width;
	}

	void TextEditor::ComputeVisualLinesForLogicalLine(size_t logicalIndex, uint32_t& yOffset,
		std::vector<VisualLine>& outList)
	{
		const std::wstring& line = m_lines[logicalIndex];
		auto lineHeight = GetLineHeight();
		const uint32_t maxWidthLimit = (m_editorArea.Width > 15) ? m_editorArea.Width - 10 : 5;

		if (line.empty()) {
			outList.emplace_back(logicalIndex, 0, 0, yOffset);
			yOffset += lineHeight;
			return;
		}

		if (!m_features.wordWrap) {
			outList.emplace_back(logicalIndex, 0, line.size(), yOffset);
			yOffset += lineHeight;
		} else {
			size_t start = 0;
			while (start < line.size()) {
				size_t low = 1, high = line.size() - start, count = 1;
				while (low <= high) {
					size_t mid = low + (high - low) / 2;
					if (static_cast<uint32_t>( std::ceilf(GetStringWidth(line.substr(start, mid)))) <= maxWidthLimit)
					{
						count = mid;
						low = mid + 1;
					}
					else
					{
						high = mid - 1;
					}
				}
				outList.emplace_back(logicalIndex, start, count, yOffset);
				yOffset += lineHeight;
				start += count;
			}
		}
	}

	void TextEditor::UpdateLinesIncremental(size_t startLine, int lineCountDelta)
	{
		if (m_lines.empty())
		{
			return;
		}

		// (O(log N))
		auto itStart = std::lower_bound(m_visualLines.begin(), m_visualLines.end(), startLine,
			[](const VisualLine& vl, size_t idx)
			{
				return vl.logicalLineIndex < idx;
			});

		size_t firstVisualIdx = std::distance(m_visualLines.begin(), itStart);

		if (lineCountDelta != 0)
		{
			for (size_t i = firstVisualIdx; i < m_visualLines.size(); ++i)
			{
				m_visualLines[i].logicalLineIndex += lineCountDelta;
			}
		}

		uint32_t oldY = (itStart != m_visualLines.end()) ? itStart->y : 
					   (m_visualLines.empty() ? 0 : m_visualLines.back().y + GetLineHeight());

		size_t linesToUpdate = (lineCountDelta > 0) ? 1 + lineCountDelta : 1;
    
		auto itEnd = itStart;
		while (itEnd != m_visualLines.end() && itEnd->logicalLineIndex < (startLine + linesToUpdate))
		{
			itEnd++;
		}
		m_visualLines.erase(itStart, itEnd);

		std::vector<VisualLine> newVisuals;
		uint32_t currentY = oldY;
		for (size_t i = 0; i < linesToUpdate; ++i)
		{
			if (startLine + i < m_lines.size())
			{
				ComputeVisualLinesForLogicalLine(startLine + i, currentY, newVisuals);
			}
		}

		m_visualLines.insert(m_visualLines.begin() + firstVisualIdx, newVisuals.begin(), newVisuals.end());

		int yDelta = static_cast<int>(currentY) - static_cast<int>(oldY);
		if (yDelta != 0)
		{
			for (size_t i = firstVisualIdx + newVisuals.size(); i < m_visualLines.size(); ++i)
			{
				m_visualLines[i].y += yDelta;
			}
		}
	}

	size_t TextEditor::GetFirstVisibleVisualLine() const
	{
		if (m_visualLines.empty()) return 0;

		auto it = std::lower_bound(m_visualLines.begin(), m_visualLines.end(), m_offsetView.Y,
			[this](const VisualLine& vl, int scrollY)
			{
				return static_cast<int>(vl.y) + static_cast<int>(GetLineHeight()) < scrollY;
			});

		if (it == m_visualLines.end())
		{
			return m_visualLines.size() - 1;
		}
		
		return std::distance(m_visualLines.begin(), it);
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

	uint32_t TextEditor::GetCharWidthW(wchar_t c)
	{
		auto it = m_charWidthCache.find(c);
		if (it != m_charWidthCache.end())
		{
			return it->second;
		}

		uint32_t width = m_graphics.GetTextExtent(std::wstring(1, c)).Width;
		m_charWidthCache[c] = width;
		
		return width;
	}

	uint32_t TextEditor::GetStringWidth(std::wstring_view text)
	{
		//return m_graphics.GetTextExtent(text).Width;
		uint32_t totalWidth = 0;
		for (wchar_t c : text)
		{
			totalWidth += GetCharWidthW(c);
		}
		return totalWidth;
	}
}
