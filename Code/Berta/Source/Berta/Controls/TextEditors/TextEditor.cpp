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
	TextEditor::TextEditor(Window* owner) :
		m_owner(owner)
	{
		m_caret = std::make_unique<Caret>(owner, Size{2,0});
		
		m_scrollableView = std::make_unique<ScrollableView>(owner);
		m_scrollableView->SetOnScrollChange([this]()
		{
			GUI::UpdateWindow(m_owner);
		});
		
		m_selectionTimer.SetOwner(m_owner);
		m_selectionTimer.Connect([this](const ArgTimer& args)
		{
			if (!m_selection.m_isSelecting)
			{
				return;
			}
			
			Point currentScroll = m_scrollableView->GetScrollOffset();
			Point newScroll = currentScroll;

			Rectangle viewport = m_editorArea;
			
			int deltaY = 0;
			if (m_lastMousePosition.Y < viewport.Y)
			{
				deltaY = m_lastMousePosition.Y - viewport.Y; 
			}
			else if (m_lastMousePosition.Y > viewport.Y + viewport.Height)
			{
				deltaY = m_lastMousePosition.Y - (viewport.Y + viewport.Height); 
			}
			
			int deltaX = 0;
			if (!m_features.wordWrap)
			{
				if (m_lastMousePosition.X < viewport.X)
				{
					deltaX = m_lastMousePosition.X - viewport.X;
				} 
				else if (m_lastMousePosition.X > viewport.X + viewport.Width)
				{
					deltaX = m_lastMousePosition.X - (viewport.X + viewport.Width);
				}
			}
			
			if (deltaY != 0)
			{
				int velocityY = static_cast<int>(deltaY * 0.2f);
				velocityY = std::clamp(velocityY, -TEXT_EDITOR_SCROLL_SPEED, TEXT_EDITOR_SCROLL_SPEED); 
				newScroll.Y += velocityY; 
			}

			if (deltaX != 0)
			{
				int velocityX = static_cast<int>(deltaX * 0.2f);
				velocityX = std::clamp(velocityX, -TEXT_EDITOR_SCROLL_SPEED, TEXT_EDITOR_SCROLL_SPEED);
				newScroll.X += velocityX;
			}
            
			if (newScroll != currentScroll)
			{
				m_scrollableView->SetScrollToX(newScroll.X);
				m_scrollableView->SetScrollToY(newScroll.Y);
				m_selection.m_endPosition = GetPositionUnderMouse(m_lastMousePosition);
				
				UpdateCaretPosition();
				GUI::UpdateWindow(m_owner);
			}
		});
	}

	void TextEditor::OnMouseEnter(const ArgMouse& args)
	{
	}

	void TextEditor::OnMouseLeave(const ArgMouse& args)
	{
	}

	void TextEditor::OnMouseDown(const ArgMouse& args)
	{
		if (m_selection.m_ignoreMouseDown || m_lines.empty())
		{
			return;
		}
		
		m_lastMousePosition = args.Position;
		m_selectionMousePosition = args.Position;
		TextPosition clickedPos = GetPositionUnderMouse(args.Position);
		
		m_selection.m_isSelecting = args.ButtonState.LeftButton;
		if (args.ShiftPressed)
		{
			m_selection.m_endPosition = clickedPos;
		}
		else
		{
			m_selection.Reset(clickedPos);
		}
		
		UpdateCaretPosition();
		GUI::Capture(m_owner);
		GUI::MarkAsNeedUpdate(m_owner);
	}

	void TextEditor::OnMouseMove(const ArgMouse& args)
	{
		if (m_selection.m_isSelecting)
		{
			TextPosition newPosition = GetPositionUnderMouse(args.Position);
			if (m_selection.m_endPosition != newPosition)
			{
				m_selection.m_endPosition = newPosition;
				UpdateCaretPosition(); 
				GUI::MarkAsNeedUpdate(m_owner); 
			}
			
			bool selectionTimerRunning = m_selectionTimer.IsRunning();
			bool insideEditorArea = m_editorArea.Contains(args.Position);
        
			if (!selectionTimerRunning && !insideEditorArea)
			{
				m_selectionDirection.X = args.Position.X < m_editorArea.X ? 1 : (args.Position.X >= static_cast<int>(m_editorArea.Width) + m_editorArea.X ? -1 : 0);
				m_selectionDirection.Y = args.Position.Y < m_editorArea.Y ? 1 : (args.Position.Y >= static_cast<int>(m_editorArea.Height) + m_editorArea.Y ? -1 : 0);
				m_selectionTimer.SetInterval(90);
				m_selectionTimer.Start();
			}
			else if (selectionTimerRunning)
			{
				if (insideEditorArea)
				{
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
		if (m_wasDblClick || m_lines.empty())
		{
			m_wasDblClick = false;
			return;
		}

		GUI::ReleaseCapture(m_owner);
		
		m_selection.m_endPosition = GetPositionUnderMouse(args.Position);
		m_selection.m_isSelecting = false;
		GUI::MarkAsNeedUpdate(m_owner); 
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
			
			m_caret->Activate();
			return needUpdate;
		}

		if (m_selection.Behavior != TextFocusBehavior::None)
		{
			Deselect();
		}
		
		m_caret->Deactivate();
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

		auto savedStartPosition = m_selection.m_startPosition;
		auto savedEndPosition = m_selection.m_endPosition;
		
		switch (args.Key)
		{
		case KeyboardKey::ArrowLeft:
			MoveCaretHorizontal(-1,args.ButtonState.Ctrl, args.ButtonState.Shift);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;

		case KeyboardKey::ArrowRight:
			MoveCaretHorizontal(1,args.ButtonState.Ctrl, args.ButtonState.Shift);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;
		
		case KeyboardKey::ArrowUp:
			MoveCaretVertically(-1, args.ButtonState.Shift);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;
		
		case KeyboardKey::ArrowDown:
			MoveCaretVertically(1, args.ButtonState.Shift);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;
			
		case KeyboardKey::Enter:
			HandleEnter();
			redraw = true;
			break;
			
		//case KeyboardKey::A:
		//	break;

		//case KeyboardKey::A:
		//	if (ctrlPressed) return SelectAll();
		//	break;

		case KeyboardKey::Home:
			MoveCaretHome(args.ButtonState.Shift);
			redraw = savedEndPosition != m_selection.m_endPosition;
			break;

		case KeyboardKey::End:
			MoveCaretEnd(args.ButtonState.Shift);
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
		
		if (redraw)
		{
			UpdateCaretPosition();
		}
		return redraw;
	}

	bool TextEditor::OnKeyReleased(const ArgKeyboard& args)
	{
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
		auto [wordStart, wordEnd] = GetWordBounds(clickPos);

		m_selection.m_isSelecting = false; 
    
		m_selection.m_startPosition = wordStart;
		m_selection.m_endPosition = wordEnd;
		
		UpdateCaretPosition();
		AdjustView();
		return true;
	}

	void TextEditor::OnResize(const ArgResize& args)
	{
		RecomputeWordWrap();
		UpdateScrollMetrics();
		AdjustView();
		UpdateCaretPosition();
	}

	void TextEditor::OnDpiChanged()
	{
		RecomputeWordWrap();
		UpdateScrollMetrics();
		AdjustView();
		UpdateCaretPosition();
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
		m_scrollableView->ResetScroll();
    
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

	void TextEditor::Cut()
	{
		if (m_selection.m_startPosition == m_selection.m_endPosition)
		{
			return;
		}
		Copy();
		DeleteRange(m_selection.m_startPosition, m_selection.m_endPosition);
	}

	void TextEditor::Copy() const
	{
		std::wstring selectedText = GetSelectedText();
		if (!selectedText.empty())
		{
			return;
		}
		Platform::SetClipboardText(selectedText, m_owner->RootHandle);
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
		
		m_selection.Reset(pos);
		CommitDocumentChange();
	}

	void TextEditor::SetEditorArea(const Rectangle& area)
	{
		m_editorArea = area;
	}

	void TextEditor::Render(Graphics& graphics)
	{
		bool enabled = GUI::IsWindowEnabled(m_owner);
		graphics.FillRectangle(m_owner->ClientSize.ToRectangle(), GetBackgroundColor());
		
		graphics.SetClipping(m_editorArea);
		
		auto currentOffset = m_scrollableView->GetScrollOffset();
		graphics.PushTranslation(m_editorArea.X - currentOffset.X, m_editorArea.Y - currentOffset.Y);
		
		RenderVisibleLines(graphics);
		RenderCaret(graphics);
		
		graphics.PopTranslation();
		graphics.EndClipping();
		
		RenderUIElements(graphics);
	}

	void TextEditor::SetScrollBarVisibility(ScrollBarVisibility vertical, ScrollBarVisibility horizontal)
	{
		m_scrollableView->SetScrollBarVisibility(vertical, horizontal);
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

	void TextEditor::InsertChar(const wchar_t wChar)
	{
		if (!m_selection.IsEmpty())
		{
			DeleteRange(m_selection.m_startPosition, m_selection.m_endPosition);
			m_selection.Reset(m_selection.m_startPosition);
		}
		
		TextPosition& position = m_selection.m_endPosition;
    
		if (wChar == L'\n')
		{
			std::wstring remainder;
			{
				auto& currentLine = m_lines[position.line];
				remainder = currentLine.substr(position.column);
				
				currentLine.erase(position.column); 
			}
        
			InvalidateLayoutsForLogicalLine(position.line);
			
			m_lines.insert(m_lines.begin() + position.line + 1, std::move(remainder));
       
			UpdateLinesIncremental(position.line, 1);
			
			position.line++;
			position.column = 0;
		}
		else
		{
			m_lines[position.line].insert(position.column, 1, wChar);
        
			InvalidateLayoutsForLogicalLine(position.line);
			UpdateLinesIncremental(position.line, 0);
			
			position.column++;
		}
		m_selection.m_startPosition = position;
		CommitDocumentChange();
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
		auto currentOffset = m_scrollableView->GetScrollOffset();
		float targetX = static_cast<float>(currentPt.X - m_editorArea.X + currentOffset.X);

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
			CommitDocumentChange();
			return;
		}
		
		TextPosition& position = m_selection.m_endPosition;
		if (position.line >= m_lines.size())
		{
			return;
		}
		
		auto& currentLine = m_lines[position.line];
		bool changed = false;
		
		if (position.column < currentLine.size())
		{
			currentLine.erase(position.column, 1);
			InvalidateLayoutsForLogicalLine(position.line);
			UpdateLinesIncremental(position.line, 0);
			changed = true;
		}
		else if (position.line + 1 < m_lines.size())
		{
			m_lines[position.line] += m_lines[position.line + 1];
			m_lines.erase(m_lines.begin() + position.line + 1);
			InvalidateLayoutsForLogicalLine(position.line);
			UpdateLinesIncremental(position.line, -1);
			changed = true;
		}

		if (!changed)
		{
			return;
		}
		m_selection.m_startPosition = position;
		CommitDocumentChange();
	}

	void TextEditor::HandleBackspace()
	{
		if (!m_selection.IsEmpty())
		{
			DeleteRange(m_selection.Min(), m_selection.Max());
			CommitDocumentChange();
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
			UpdateLinesIncremental(targetLine, -1);
        
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
		
		CommitDocumentChange();
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

	void TextEditor::RenderVisibleLines(Graphics& graphics)
	{
		size_t firstLineIndex = GetFirstVisibleVisualLine();
		int viewportBottom = m_scrollableView->GetScrollOffset().Y + (int)m_editorArea.Height;
		Color textColor = m_owner->Appearance->Foreground;

		for (size_t i = firstLineIndex; i < m_visualLines.size(); ++i)
		{
			const auto& vl = m_visualLines[i];
    
			if ((int)vl.y > viewportBottom) break; 

			EnsureLayout(vl);

			DrawSelectionBackground(graphics, vl);

			if (vl.m_textHandle)
			{
				graphics.DrawTextLayout(vl.m_textHandle, { 0, static_cast<int>(vl.y) }, textColor);
			}
		}
	}

	void TextEditor::RenderCaret(Graphics& graphics)
	{
		if (!m_caret || !m_caret->IsVisible())
		{
			return;
		}
		
		Point logicalCaretPos = m_caret->GetPosition(); 
        
		auto caretHeight = graphics.GetCaretHeight(); 
		Color caretColor = m_owner->Appearance->Foreground2nd;

		graphics.DrawLine
		(
			{ logicalCaretPos.X, logicalCaretPos.Y }, 
			{ logicalCaretPos.X, logicalCaretPos.Y + static_cast<int>(caretHeight) }, 
			static_cast<float>(m_owner->ToScale(m_caret->GetSize().Width)),
			caretColor
		);
	}

	void TextEditor::RenderUIElements(Graphics& graphics)
	{
		if (m_scrollableView && m_scrollableView->HasVerticalScroll() && m_scrollableView->HasHorizontalScroll())
		{
			auto scrollSize = m_owner->ToScale(m_owner->Appearance->ScrollBarSize);
			graphics.FillRectangle(
				{ static_cast<int>(m_owner->ClientSize.Width - scrollSize) - 1, 
				  static_cast<int>(m_owner->ClientSize.Height - scrollSize) - 1, 
				  scrollSize, scrollSize }, 
				m_owner->Appearance->Background
			);
		}
	}
	
	void TextEditor::DrawSelectionBackground(Graphics& graphics, const VisualLine& vl) const
	{
		if (!vl.m_textHandle.m_textLayout)
		{
			return;
		}

		if (m_selection.IsEmpty())
		{
			return;
		}

		auto [localStart, selLength] = GetSelectionRangeForLine(vl);
    
		auto startPos = m_selection.Min();
		auto endPos = m_selection.Max();

		bool isLastVisualOfLogical = (vl.charStart + vl.charLength == m_lines[vl.logicalLineIndex].length());
    
		bool newlineSelected = isLastVisualOfLogical && 
							   (vl.logicalLineIndex >= startPos.line) && 
							   (vl.logicalLineIndex < endPos.line);

		if (selLength == 0 && !newlineSelected) return;

		Color selColor = m_owner->Appearance->HighlightColor;
		uint32_t uniformLineHeight = GetLineHeight();

#ifdef BT_PLATFORM_WINDOWS
		if (selLength > 0)
		{
			uint32_t maxHitTestMetrics = 4;
			std::vector<DWRITE_HIT_TEST_METRICS> hitTestMetrics(maxHitTestMetrics);
			uint32_t actualHitTestMetrics = 0;

			HRESULT hr = vl.m_textHandle.m_textLayout->HitTestTextRange(
				localStart, selLength, 0, 0,
				hitTestMetrics.data(), maxHitTestMetrics, &actualHitTestMetrics
			);

			if (hr == E_NOT_SUFFICIENT_BUFFER) {
				hitTestMetrics.resize(actualHitTestMetrics);
				hr = vl.m_textHandle.m_textLayout->HitTestTextRange(
					localStart, selLength, 0, 0,
					hitTestMetrics.data(), static_cast<UINT32>(hitTestMetrics.size()), &actualHitTestMetrics
				);
			}

			if (SUCCEEDED(hr)) {
				for (uint32_t i = 0; i < actualHitTestMetrics; ++i) {
					const auto& metrics = hitTestMetrics[i];
					Rectangle rect{
						static_cast<int>(metrics.left),
						static_cast<int>(metrics.top + vl.y), 
						static_cast<uint32_t>(metrics.width),
						uniformLineHeight 
					};
					graphics.FillRectangle(rect, selColor);
				}
			}
		}

		if (newlineSelected)
		{
			float textWidth = 0.0f;
			DWRITE_TEXT_METRICS textMetrics;
        
			if (SUCCEEDED(vl.m_textHandle.m_textLayout->GetMetrics(&textMetrics))) {
				textWidth = textMetrics.widthIncludingTrailingWhitespace;
			}

			Rectangle newlineRect{
				static_cast<int>(textWidth),
				static_cast<int>(vl.y),
				8u, // Ancho visual del "Enter" 
				uniformLineHeight
			};
			graphics.FillRectangle(newlineRect, selColor);
		}
#endif
	}

	std::pair<uint32_t, uint32_t> TextEditor::GetSelectionRangeForLine(const VisualLine& vl) const
	{
		if (!m_selection.m_isSelecting && m_selection.m_startPosition == m_selection.m_endPosition) {
			return { 0, 0 };
		}

		auto [startPos, endPos] = std::minmax(m_selection.m_startPosition, m_selection.m_endPosition);

		if (vl.logicalLineIndex < startPos.line || vl.logicalLineIndex > endPos.line) {
			return { 0, 0 }; 
		}

		uint32_t localStart = 0;
		uint32_t localEnd = static_cast<uint32_t>(vl.charLength);

		if (vl.logicalLineIndex == startPos.line)
		{
			if (startPos.column > vl.charStart + vl.charLength)
			{
				return { 0, 0 };
			}
			localStart = static_cast<uint32_t>(startPos.column > vl.charStart ? startPos.column - vl.charStart : 0);
		}

		if (vl.logicalLineIndex == endPos.line)
		{
			if (endPos.column < vl.charStart)
			{
				return { 0, 0 };
			}
			localEnd = static_cast<uint32_t>(endPos.column < vl.charStart + vl.charLength ? endPos.column - vl.charStart : vl.charLength);
		}

		uint32_t length = (localEnd > localStart) ? (localEnd - localStart) : 0;
		return { localStart, length };
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
		return m_owner->Renderer.GetGraphics().GetTextExtent("Ay").Height;
	}

	void TextEditor::AdjustView() const
	{
		auto viewHeight = static_cast<int>(m_editorArea.Height);
		auto viewWidth = static_cast<int>(m_editorArea.Width);
		
		size_t vIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
		if (vIdx >= m_visualLines.size())
		{
			return;
		}
		auto currentOffset = m_scrollableView->GetScrollOffset();
		
		const auto& vl = m_visualLines[vIdx];
		if (m_features.wordWrap || m_features.isMultiLines)
		{
			auto lineHeight = static_cast<int>(GetLineHeight());
			int caretY = static_cast<int>(vl.y);
			
			if (caretY + lineHeight - currentOffset.Y > viewHeight)
			{
				currentOffset.Y = caretY + lineHeight - viewHeight;
			}
			else if (caretY - currentOffset.Y < 0)
			{
				currentOffset.Y = caretY;
			}
		}
		else
		{
			currentOffset.Y = 0;
		}
		
		if (!m_features.wordWrap)
		{
			std::wstring textToCaret = m_lines[vl.logicalLineIndex].substr(vl.charStart, m_selection.m_endPosition.column - vl.charStart);
			int caretX = static_cast<int>(m_owner->Renderer.GetGraphics().GetTextExtent(textToCaret).Width);

			if (caretX > currentOffset.X + viewWidth)
			{
				currentOffset.X = caretX - viewWidth;
			}
			else if (caretX - currentOffset.X < 0)
			{
				currentOffset.X = caretX;
			}
		}
		else
		{
			currentOffset.X = 0;
		}
		
		m_scrollableView->SetScrollToX(currentOffset.X);
		m_scrollableView->SetScrollToY(currentOffset.Y);
	}

	TextPosition TextEditor::GetPositionUnderMouse(const Point& mousePosition) const
	{
		Point logicalPoint = ToEditorSpace(mousePosition);
		if (m_visualLines.empty()) return { 0, 0 };

		auto lineHeight = (int)GetLineHeight();

		auto it = std::lower_bound(m_visualLines.begin(), m_visualLines.end(), logicalPoint.Y,
			[lineHeight](const VisualLine& vl, int y) {
				return ((int)vl.y + lineHeight) <= y;
			});

		const VisualLine* targetLine = nullptr;

		if (it == m_visualLines.end()) 
		{
			targetLine = &m_visualLines.back();
		} 
		else 
		{
			targetLine = &(*it);
        
			if (logicalPoint.Y < static_cast<int>(m_visualLines.front().y))
			{
				return { 0, 0 };
			}
		}

		TextPosition pos;
		pos.line = targetLine->logicalLineIndex;

#ifdef BT_PLATFORM_WINDOWS
		EnsureLayout(*targetLine);
		
		if (targetLine->m_textHandle.m_textLayout)
		{
			BOOL isTrailingHit = FALSE;
			BOOL isInside = FALSE;
			DWRITE_HIT_TEST_METRICS metrics;

			HRESULT hr = targetLine->m_textHandle.m_textLayout->HitTestPoint(
				static_cast<FLOAT>(logicalPoint.X),
				0.0f, 
				&isTrailingHit,
				&isInside,
				&metrics
			);

			if (SUCCEEDED(hr))
			{
				pos.column = targetLine->charStart + metrics.textPosition;
            
				if (isTrailingHit)
				{
					pos.column += metrics.length;
				}
			}
		}
		else
		{
			pos.column = targetLine->charStart;
		}
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
		return pos;
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
		if (m_visualLines.empty())
		{
			return { 0, 0 };
		}

		auto it = std::lower_bound(m_visualLines.begin(), m_visualLines.end(), pos.line,
		[](const VisualLine& vl, size_t lineIdx)
			{
				return vl.logicalLineIndex < lineIdx;
			});

		const VisualLine* targetLine = nullptr;

		while (it != m_visualLines.end() && it->logicalLineIndex == pos.line)
		{
			if (pos.column >= it->charStart && pos.column <= (it->charStart + it->charLength))
			{
				targetLine = &(*it);
				break;
			}
			++it;
		}

		if (!targetLine)
		{
			return { 0, 0 };
		}
		
		float caretX = 0.0f;

#ifdef BT_PLATFORM_WINDOWS
		EnsureLayout(*targetLine);

		if (targetLine->m_textHandle.m_textLayout)
		{
			FLOAT hitX = 0.0f, hitY = 0.0f;
			DWRITE_HIT_TEST_METRICS metrics;
    
			uint32_t localCharIndex = static_cast<uint32_t>(pos.column - targetLine->charStart);

			HRESULT hr = targetLine->m_textHandle.m_textLayout->HitTestTextPosition
			(
				localCharIndex, 
				FALSE, 
				&hitX, 
				&hitY, 
				&metrics
			);

			if (SUCCEEDED(hr))
			{
				caretX = hitX;
			}
		}
#endif

		return { static_cast<int>(caretX), static_cast<int>(targetLine->y) };
	}

	void TextEditor::UpdateCaretPosition()
	{
		Point logicalPos = GetPointFromPosition(m_selection.m_endPosition);
		m_caret->SetPosition(logicalPos);

		if (m_scrollableView)
		{
			uint32_t caretHeight = GetLineHeight();
			uint32_t caretWidth = m_caret->GetSize().Width;

			Rectangle caretRect{ logicalPos.X, logicalPos.Y, caretWidth, caretHeight };
        
			m_scrollableView->EnsureVisibility(caretRect);
		}
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
		
		uint32_t workingWidth = m_editorArea.Width;
		
		for (size_t i = 0; i < m_lines.size(); ++i)
		{
			ComputeVisualLinesForLogicalLine(m_owner->Renderer.GetGraphics(), i, currentY, m_visualLines, workingWidth);
		}

		if (m_features.wordWrap)
		{
			if (currentY > m_editorArea.Height)
			{
				uint32_t scrollSize = m_owner->ToScale(m_owner->Appearance->ScrollBarSize);
            
				workingWidth = (m_editorArea.Width > scrollSize) ? (m_editorArea.Width - scrollSize) : 5;

				m_visualLines.clear();
				currentY = 0;
				for (size_t i = 0; i < m_lines.size(); ++i)
				{
					ComputeVisualLinesForLogicalLine(m_owner->Renderer.GetGraphics(), i, currentY, m_visualLines, workingWidth);
				}
			}
			m_cachedMaxWidth = workingWidth;
		}
		
		UpdateScrollMetrics();
	}

	void TextEditor::ComputeVisualLinesForLogicalLine(Graphics& graphics, size_t logicalIndex, uint32_t& yOffset, std::vector<VisualLine>& outList, uint32_t layoutWidth)
	{
		const std::wstring& line = m_lines[logicalIndex];
		auto lineHeight = GetLineHeight();
		const uint32_t maxWidthLimit = (layoutWidth > 15) ? layoutWidth - 10 : 5;

		if (line.empty())
		{
			outList.emplace_back(logicalIndex, 0, 0, yOffset);
			yOffset += lineHeight;
			return;
		}

		if (!m_features.wordWrap)
		{
			m_cachedMaxWidth = std::max<uint32_t>(m_cachedMaxWidth, graphics.GetTextExtent(line).Width);
			outList.emplace_back(logicalIndex, 0, line.size(), yOffset);
			yOffset += lineHeight;
			return;
		}
		
#ifdef BT_PLATFORM_WINDOWS
		auto nativeAttr = graphics.GetNativeHandle();
		Microsoft::WRL::ComPtr<IDWriteTextLayout> tempLayout;
		
		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextLayout
		(
			line.c_str(),
			static_cast<UINT32>(line.size()),
			nativeAttr->m_textFormat,
			static_cast<float>(layoutWidth),
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
			ComputeVisualLinesForLogicalLine(m_owner->Renderer.GetGraphics(), i, runningY, newVisuals, m_cachedMaxWidth);
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
		auto currentOffset = m_scrollableView->GetScrollOffset();
		auto it = std::lower_bound(m_visualLines.begin(), m_visualLines.end(), currentOffset.Y,
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
		auto nativeAttr = m_owner->Renderer.GetGraphics().GetNativeHandle();
		if (!nativeAttr || !nativeAttr->m_textFormat)
		{
			return;
		}

		float layoutWidth = m_features.wordWrap ? static_cast<float>(m_cachedMaxWidth) : 1000000.0f; 
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

	void TextEditor::InvalidateLayoutsForLogicalLine(size_t logicalIndex) const
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

	void TextEditor::UpdateScrollMetrics() const
	{
		if (!m_scrollableView)
		{
			return;
		}
		
		m_scrollableView->SetViewRect(m_editorArea);
		m_scrollableView->SetContentSize(GetContentTextExtent());
	}

	void TextEditor::CommitDocumentChange()
	{
		RecomputeWordWrap();
		UpdateScrollMetrics();
		AdjustView();
		UpdateCaretPosition();
		EmitValueChanged();
	}

	Point TextEditor::ToEditorSpace(Point windowPoint) const
	{
		Point offset = m_scrollableView ? m_scrollableView->GetScrollOffset() : Point{0,0};
		return { 
			windowPoint.X - m_editorArea.X + offset.X, 
			windowPoint.Y - m_editorArea.Y + offset.Y 
		};
	}

	Point TextEditor::ToWindowSpace(Point editorPoint) const
	{
		Point offset = m_scrollableView ? m_scrollableView->GetScrollOffset() : Point{0,0};
		return { 
			editorPoint.X + m_editorArea.X - offset.X, 
			editorPoint.Y + m_editorArea.Y - offset.Y 
		};
	}

	TextEditor::CharClass TextEditor::GetCharClass(wchar_t c)
	{
		if (std::iswalnum(c) || c == L'_') 
			return CharClass::Alphanumeric;
		if (std::iswspace(c)) 
			return CharClass::Whitespace;
    
		return CharClass::Punctuation;
	}

	std::pair<TextPosition, TextPosition> TextEditor::GetWordBounds(TextPosition pos) const
	{
		if (pos.line >= m_lines.size())
		{
			return { pos, pos };
		}

		const std::wstring& line = m_lines[pos.line];
    
		if (line.empty() || pos.column >= line.length())
		{
			return { pos, pos };
		}

		CharClass targetClass = GetCharClass(line[pos.column]);

		size_t startCol = pos.column;
		size_t endCol = pos.column;

		while (startCol > 0 && GetCharClass(line[startCol - 1]) == targetClass)
		{
			startCol--;
		}

		while (endCol < line.length() && GetCharClass(line[endCol]) == targetClass)
		{
			endCol++;
		}

		return { { pos.line, startCol }, { pos.line, endCol } };
	}
}
