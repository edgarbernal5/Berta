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

#ifdef BT_PLATFORM_WINDOWS
#include "Berta/Platform/Windows/D2D.h"
#endif

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
			MoveCaretHorizontal(-1,m_ctrlPressed, m_shiftPressed);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;

		case KeyboardKey::ArrowRight:
			MoveCaretHorizontal(1,m_ctrlPressed, m_shiftPressed);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;
		
		case KeyboardKey::ArrowUp:
			MoveCaretVertically(-1, m_shiftPressed);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;
		
		case KeyboardKey::ArrowDown:
			MoveCaretVertically(1, m_shiftPressed);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;
			
		case KeyboardKey::Enter:
			HandleEnter();
			redraw = true;
			break;
			
		//case KeyboardKey::A:
		//	break;

		//case KeyboardKey::A:
		//	if (m_ctrlPressed) return SelectAll();
		//	break;

		case KeyboardKey::Home:
			MoveCaretHome(m_shiftPressed);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;

		case KeyboardKey::End:
			MoveCaretEnd(m_shiftPressed);
			redraw = savedEndPosition != m_selection.m_endPosition;
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

		auto isWordChar = [](wchar_t ch)
		{ 
			return std::iswalnum(ch) || ch == L'_'; 
		};
		bool clickingOnWord = (clickPos.column < line.size()) ? isWordChar(line[clickPos.column]) : false;
		while (start > 0 && isWordChar(line[start - 1]) == clickingOnWord)
		{
			start--;
		}

		while (end < line.size() && isWordChar(line[end]) == clickingOnWord)
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
			InvalidateLayoutsForLogicalLine(position.line);
			UpdateLinesIncremental(position.line, 1);
			
			position.line++;
			position.column = 0;
		}
		else
		{
			currentLine.insert(position.column, 1, chr);
			InvalidateLayoutsForLogicalLine(position.line);
			UpdateLinesIncremental(position.line, 0);
			position.column++;
		}
		m_selection.m_startPosition = position;
		
		AdjustView();
		EmitValueChanged();
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

	void TextEditor::MoveCaretVertically(int direction, bool select)
	{
		size_t currentVlIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
    
		if (direction < 0 && currentVlIdx == 0) return;
		if (direction > 0 && currentVlIdx + 1 >= m_visualLines.size())
			return;

		size_t targetVlIdx = currentVlIdx + direction;
		const auto& targetVl = m_visualLines[targetVlIdx];

		Point currentPt = GetPointFromPosition(m_selection.m_endPosition);
		float targetX = static_cast<float>(currentPt.X - m_editorArea.X + m_offsetView.X);

		EnsureLayout(targetVl);
    
		BOOL isTrailingHit, isInside;
		DWRITE_HIT_TEST_METRICS metrics;

#ifdef BT_PLATFORM_WINDOWS
		if (targetVl.m_textHandle.IsValid())
		{
			targetVl.m_textHandle.m_textLayout->HitTestPoint
			(
				targetX, 0, &isTrailingHit, &isInside, &metrics
			);

			m_selection.m_endPosition.line = targetVl.logicalLineIndex;
			m_selection.m_endPosition.column = targetVl.charStart + metrics.textPosition;
			if (isTrailingHit)
			{
				m_selection.m_endPosition.column++;
			}
		}
#endif
		
		if (!select)
		{
			m_selection.m_startPosition = m_selection.m_endPosition;
		}
		AdjustView();
	}

	void TextEditor::MoveCaretHorizontal(int direction, bool wordJump, bool select)
	{
		TextPosition& position = m_selection.m_endPosition;
		const std::wstring& line = m_lines[position.line];

		if (direction > 0)
		{
			if (position.column < line.size())
			{
				if (IS_HIGH_SURROGATE(line[position.column]) && 
					position.column + 1 < line.size() && 
					IS_LOW_SURROGATE(line[position.column + 1]))
				{
					position.column += 2;
				}
				else if (wordJump)
				{
					position = GetPositionNextWord(m_selection.m_endPosition, direction);
				}
				else
				{
					position.column += 1;
				}
			}
			else if (position.line + 1 < m_lines.size())
			{
				position.line++;
				position.column = 0;
			}
		}
		else
		{
			if (position.column > 0)
			{
				if (IS_LOW_SURROGATE(line[position.column - 1]) && 
					position.column > 1 && 
					IS_HIGH_SURROGATE(line[position.column - 2]))
				{
					position.column -= 2;
				}
				else if (wordJump)
				{
					position = GetPositionNextWord(m_selection.m_endPosition, direction);
				}
				else
				{
					position.column -= 1;
				}
			}
			else if (position.line > 0)
			{
				position.line--;
				position.column = m_lines[position.line].size();
			}
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
			if (position.column >= m_visualLines[i].charStart && 
						position.column < m_visualLines[i].charStart + m_visualLines[i].charLength)
			{
				return i;
			}
        
			if (position.column == m_visualLines[i].charStart + m_visualLines[i].charLength)
			{
				if (i + 1 == m_visualLines.size() || m_visualLines[i + 1].logicalLineIndex != position.line)
				{
					return i;
				}
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
		SetContent(StringUtils::UTF8ToWide(newContent));
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
		
		TextPosition startPosition = m_selection.Min();
		TextPosition endPosition = m_selection.Max();
		
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
			
			if (startPosition != endPosition && vl.logicalLineIndex >= startPosition.line && vl.logicalLineIndex <= endPosition.line)
			{
				size_t selStartInV = (vl.logicalLineIndex == startPosition.line) ? (std::max)(vl.charStart, startPosition.column) : vl.charStart;
				size_t selEndInV = (vl.logicalLineIndex == endPosition.line) ? (std::min)(vl.charStart + vl.charLength, endPosition.column) : (vl.charStart + vl.charLength);

				if (selStartInV < selEndInV)
				{
					auto x1 = m_graphics.GetTextExtent(fragment.substr(0, selStartInV - vl.charStart)).Width;
					auto w = m_graphics.GetTextExtent(fragment.substr(selStartInV - vl.charStart, selEndInV - selStartInV)).Width;
					m_graphics.DrawRectangle(Rectangle{static_cast<int>(x1) + drawX + m_editorArea.X, drawY + m_editorArea.Y, (uint32_t)(w + one), lineHeight}, Color(0, 120, 215, 128), true);
				}
			}
			EnsureLayout(vl);
			if (vl.m_textHandle)
			{
				m_graphics.DrawTextLayout(vl.m_textHandle, { drawX + m_editorArea.X, drawY + m_editorArea.Y }, m_owner->Appearance->Foreground);
			}
			//m_graphics.DrawString({ drawX + m_editorArea.X, drawY + m_editorArea.Y }, fragment, m_owner->Appearance->Foreground);
			
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
		{
			return;
		}
		
		m_features.isMultiLines = enable;
		for(auto& vl : m_visualLines)
		{
			vl.m_textHandle.Release();
		}
		RecomputeWordWrap();
	}

	void TextEditor::SetWordWrap(bool enabled)
	{
		if (m_features.wordWrap == enabled)
		{
			return;
		}
		
		m_features.wordWrap = enabled;
		for(auto& vl : m_visualLines)
		{
			vl.m_textHandle.Release();
		}
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

	void TextEditor::LoadFile(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			return;
		}

		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
		std::string_view view(content);
		if (view.size() >= 3 && static_cast<unsigned char>(view[0]) == 0xEF && static_cast<unsigned char>(view[1]) == 0xBB && static_cast<unsigned char>(view[2]) == 0xBF)
		{
			view.remove_prefix(3);
		}

		std::wstring wContent = StringUtils::UTF8ToWide(std::string(view));

		m_lines.clear();
		std::size_t start = 0, end;
		while ((end = wContent.find_first_of(L"\r\n", start)) != std::wstring::npos)
		{
			m_lines.push_back(wContent.substr(start, end - start));
			if (wContent[end] == L'\r' && end + 1 < wContent.size() && wContent[end + 1] == L'\n')
			{
				start = end + 2;
			}
			else
			{
				start = end + 1;
			}
		}
		m_lines.push_back(wContent.substr(start));

		RecomputeWordWrap();
		m_selection.Reset({ 0, 0 });
	}

	void TextEditor::SaveFile(const std::string& path) const
	{
		std::wstring fullContent;
		for (size_t i = 0; i < m_lines.size(); ++i)
		{
			fullContent += m_lines[i];
			if (i < m_lines.size() - 1)
			{
				fullContent += L"\r\n";
			}
		}

		std::string utf8Content = StringUtils::WideToUTF8(fullContent);

		std::ofstream file(path, std::ios::binary);
		file.write(utf8Content.c_str(), utf8Content.size());
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
		
		int relativeY = mousePosition.Y - m_editorArea.Y + m_offsetView.Y;
		auto lineHeight = GetLineHeight();
		auto it = std::lower_bound(m_visualLines.begin(), m_visualLines.end(), relativeY,
			[lineHeight](const VisualLine& vl, int y)
			{
				return static_cast<int>(vl.y) + static_cast<int>(lineHeight) < y;
			});
		
		if (it == m_visualLines.end())
		{
			it = std::prev(m_visualLines.end());
		}
#ifdef BT_PLATFORM_WINDOWS
		const auto& vl = *it;
		float localX = static_cast<float>(mousePosition.X - m_editorArea.X + m_offsetView.X);
		float localY = static_cast<float>(relativeY - vl.y);

		EnsureLayout(vl);
		if (!vl.m_textHandle.IsValid())
		{
			return { vl.logicalLineIndex, vl.charStart };
		}
		
		BOOL isTrailingHit;
		BOOL isInside;
		DWRITE_HIT_TEST_METRICS metrics;
		
		vl.m_textHandle.m_textLayout->HitTestPoint
		(
			localX,
			localY,
			&isTrailingHit, // Si clicamos en la mitad derecha del carácter
			&isInside,      // Si el clic cayó realmente sobre el texto
			&metrics
		);
		
		size_t finalColumn = vl.charStart + metrics.textPosition;
		if (isTrailingHit)
		{
			finalColumn++;
		}
		finalColumn = std::min<size_t>(finalColumn, m_lines[vl.logicalLineIndex].size());

		return { vl.logicalLineIndex, finalColumn };
#else
		
		int localX = mousePosition.X - m_editorArea.X + m_offsetView.X;
		int localY = mousePosition.Y - m_editorArea.Y + m_offsetView.Y;
		
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
#endif
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

	Point TextEditor::GetPointFromPosition(TextPosition pos) const
	{
		size_t vlIdx = GetVisualLineIndexFromPos(pos);
		const auto& vl = m_visualLines[vlIdx];
		
		EnsureLayout(vl);

		float localX = 0.0f;
		float localY = 0.0f;

		if (vl.m_textHandle.IsValid())
		{
			DWRITE_HIT_TEST_METRICS metrics;
			uint32_t relativePos = static_cast<uint32_t>(pos.column - vl.charStart);

#ifdef BT_PLATFORM_WINDOWS
			vl.m_textHandle.m_textLayout->HitTestTextPosition
			(
				relativePos,
				FALSE,
				&localX,
				&localY,
				&metrics
			);
#endif
		}

		int x = m_editorArea.X - m_offsetView.X + static_cast<int>(localX);
		int y = m_editorArea.Y - m_offsetView.Y + static_cast<int>(vl.y);

		return { x, y };
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

		for (size_t i = 0; i < m_lines.size(); ++i)
		{
			ComputeVisualLinesForLogicalLine(i, currentY, m_visualLines);
		}
    
		if (m_features.wordWrap) m_cachedMaxWidth = m_editorArea.Width;
	}

	void TextEditor::ComputeVisualLinesForLogicalLine(size_t logicalIndex, uint32_t& yOffset, std::vector<VisualLine>& outList)
	{
		const std::wstring& line = m_lines[logicalIndex];
		auto lineHeight = GetLineHeight();
		const uint32_t maxWidthLimit = (m_editorArea.Width > 15) ? m_editorArea.Width - 10 : 5;

		if (line.empty())
		{
			outList.emplace_back(logicalIndex, 0, 0, yOffset);
			yOffset += lineHeight;
			return;
		}

		if (!m_features.wordWrap)
		{
			m_cachedMaxWidth = std::max<uint32_t>(m_cachedMaxWidth, m_graphics.GetTextExtent(line).Width);
			outList.emplace_back(logicalIndex, 0, line.size(), yOffset);
			yOffset += lineHeight;
			return;
		}
#ifdef BT_PLATFORM_WINDOWS
		auto nativeAttr = m_graphics.GetNativeHandle();
		Microsoft::WRL::ComPtr<IDWriteTextLayout> tempLayout;
		
		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextLayout
		(
			line.c_str(),
			static_cast<UINT32>(line.size()),
			nativeAttr->m_textFormat,
			static_cast<float>(m_editorArea.Width),
			static_cast<float>(lineHeight),
			&tempLayout
		);

		if (SUCCEEDED(hr))
		{
			uint32_t actualLineCount = 0;
			tempLayout->GetLineMetrics(nullptr, 0, &actualLineCount);

			std::vector<DWRITE_LINE_METRICS> metrics(actualLineCount);
			tempLayout->GetLineMetrics(metrics.data(), actualLineCount, &actualLineCount);

			size_t currentPos = 0;
			for (const auto& lineMetric : metrics)
			{
				outList.emplace_back(logicalIndex, currentPos, lineMetric.length, yOffset);
            
				currentPos += lineMetric.length;
				yOffset += lineHeight;
			}
		}
#else
		size_t start = 0;
		while (start < line.size())
		{
			size_t low = 1, high = line.size() - start, count = 1;
			while (low <= high)
			{
				size_t mid = low + (high - low) / 2;
				if (static_cast<uint32_t>(std::ceilf(static_cast<float>(GetStringWidth(line.substr(start, mid))))) <= maxWidthLimit)
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
#endif
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

		size_t visualStartIndex = std::distance(m_visualLines.begin(), itStart);

		size_t affectedEndLine = startLine + (lineCountDelta < 0 ? -lineCountDelta : 0);
		auto itEnd = std::lower_bound(itStart, m_visualLines.end(), affectedEndLine + 1,
			[](const VisualLine& vl, size_t idx)
			{
				return vl.logicalLineIndex < idx;
			});

		uint32_t originalYStart = (itStart != m_visualLines.end()) ? itStart->y : 
								   (m_visualLines.empty() ? 0 : m_visualLines.back().y + GetLineHeight());
    
		uint32_t nextLineYBefore = (itEnd != m_visualLines.end()) ? itEnd->y : 
									(m_visualLines.empty() ? 0 : m_visualLines.back().y + GetLineHeight());

		m_visualLines.erase(itStart, itEnd);

		std::vector<VisualLine> newVisuals;
		uint32_t runningY = originalYStart;
		size_t lastLineToCompute = startLine + (lineCountDelta > 0 ? lineCountDelta : 0);

		for (size_t i = startLine; i <= lastLineToCompute; ++i)
		{
			ComputeVisualLinesForLogicalLine(i, runningY, newVisuals);
		}

		m_visualLines.insert(m_visualLines.begin() + visualStartIndex, newVisuals.begin(), newVisuals.end());

		int heightDelta = static_cast<int>(runningY) - static_cast<int>(nextLineYBefore);

		for (size_t i = visualStartIndex + newVisuals.size(); i < m_visualLines.size(); ++i)
		{
			m_visualLines[i].y += heightDelta;
			m_visualLines[i].logicalLineIndex += lineCountDelta;
		}
	}

	size_t TextEditor::GetFirstVisibleVisualLine() const
	{
		if (m_visualLines.empty())
		{
			return 0;
		}
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

	void TextEditor::EnsureLayout(const VisualLine& vl) const
	{
		if (vl.m_textHandle.IsValid())
		{
			return;
		}

		const std::wstring& fullLine = m_lines[vl.logicalLineIndex];
    
		if (fullLine.empty() && vl.charLength == 0)
		{
			return;
		}

#ifdef BT_PLATFORM_WINDOWS
		auto nativeAttr = m_graphics.GetNativeHandle();
		if (!nativeAttr || !nativeAttr->m_textFormat)
		{
			return;
		}

		float layoutWidth = m_features.wordWrap ? static_cast<float>(m_editorArea.Width) : 1000000.0f; 
		float layoutHeight = static_cast<float>(GetLineHeight());
		
		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextLayout
		(
			fullLine.c_str() + vl.charStart,
			static_cast<UINT32>(vl.charLength),
			nativeAttr->m_textFormat,
			layoutWidth,
			layoutHeight,
			&vl.m_textHandle.m_textLayout
		);

		if (FAILED(hr))
		{
			BT_CORE_ERROR << "Failed! Ensure layout with line " << vl.logicalLineIndex << std::endl;
		}
#endif
	}

	void TextEditor::InvalidateLayoutsForLogicalLine(size_t logicalIndex)
	{
		for (auto& vl : m_visualLines)
		{
			if (vl.logicalLineIndex == logicalIndex)
			{
				vl.m_textHandle.Release();
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
