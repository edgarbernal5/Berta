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
			m_caret=nullptr;
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
			m_selection.m_endPosition = GetPositionUnderMouse(args.Position);
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
			if (m_ctrlPressed) m_selection.m_endPosition = GetPositionNextWord(m_selection.m_endPosition, -1);
			else MoveCaretLeft(m_shiftPressed);
			if (!m_shiftPressed && !m_ctrlPressed) m_selection.m_startPosition = m_selection.m_endPosition;
			break;

		case KeyboardKey::ArrowRight:
			if (m_ctrlPressed) m_selection.m_endPosition = GetPositionNextWord(m_selection.m_endPosition, 1);
			else MoveCaretRight(m_shiftPressed);
			if (!m_shiftPressed && !m_ctrlPressed) m_selection.m_startPosition = m_selection.m_endPosition;
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
		/*if (args.Key == KeyboardKey::ArrowLeft && (m_caretPosition > 0 || m_selection.m_startPosition != m_selection.m_endPosition))
		{
			MoveCaretLeft(m_shiftPressed);
			redraw = true;
		}
		else if (args.Key == KeyboardKey::ArrowRight && (m_caretPosition < contentSize || m_selection.m_startPosition != m_selection.m_endPosition))
		{
			MoveCaretRight(m_shiftPressed);
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
			HandleBackspace();
			redraw = true;
		}
		else if (args.Key == KeyboardKey::Delete && m_features.isEditable && (m_caretPosition < contentSize || m_selection.m_startPosition != m_selection.m_endPosition))
		{
			HandleDelete();
			redraw = true;
		}
		return redraw;*/
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
		if (line.empty()) return true;

		size_t start = clickPos.column;
		size_t end = clickPos.column;

		// Expandir hacia la izquierda
		while (start > 0 && iswalnum(line[start - 1])) {
			start--;
		}
		// Expandir hacia la derecha
		while (end < line.size() && iswalnum(line[end])) {
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
		
		auto& currentLine = m_lines[m_selection.m_endPosition.line];
    
		currentLine.insert(m_selection.m_endPosition.column, 1, chr);

		// 3. Avanzar el cursor una posición a la derecha
		m_selection.m_endPosition.column++;
		m_selection.m_startPosition = m_selection.m_endPosition; // Resetear selección
		
		//AdjustView();
		EmitValueChanged();
	}

	void TextEditor::MoveCaretLeft(bool select)
	{
		TextPosition& pos = m_selection.m_endPosition;

		if (pos.column > 0)
		{
			// Movimiento normal a la izquierda
			pos.column--;
		}
		else if (pos.line > 0)
		{
			// Salto a la línea superior (al final de esta)
			pos.line--;
			pos.column = m_lines[pos.line].size();
		}

		// Si no estamos seleccionando, el inicio de la selección sigue al cursor
		if (!select) 
		{
			m_selection.m_startPosition = pos;
		}
		
		/*size_t newCaretPosition = m_caretPosition;
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
		AdjustView();*/
	}

	void TextEditor::MoveCaretHome(bool select)
	{
		/*size_t newCaretPosition = 0;

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
		AdjustView();*/
	}

	void TextEditor::MoveCaretRight(bool select)
	{
		TextPosition& pos = m_selection.m_endPosition;
		const std::wstring& currentLine = m_lines[pos.line];

		if (pos.column < currentLine.size())
		{
			// Movimiento normal a la derecha
			pos.column++;
		}
		else if (pos.line + 1 < m_lines.size())
		{
			// Salto al inicio de la siguiente línea
			pos.line++;
			pos.column = 0;
		}

		if (!select)
		{
			m_selection.m_startPosition = pos;
		}
		/*size_t newCaretPosition = m_caretPosition;

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
		AdjustView();*/
	}

	void TextEditor::MoveCaretEnd(bool select)
	{
		/*size_t newCaretPosition = m_content.size();

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
		AdjustView();*/
	}

	void TextEditor::MoveCaretUp(bool select)
	{
		size_t vIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
		if (vIdx > 0) {
			const auto& currentV = m_visualLines[vIdx];
			const auto& targetV = m_visualLines[vIdx - 1];
			size_t offset = m_selection.m_endPosition.column - currentV.charStart;
        
			m_selection.m_endPosition.line = targetV.logicalLineIndex;
			m_selection.m_endPosition.column = targetV.charStart + (std::min)(offset, targetV.charLength);
		}
		if (!select) m_selection.m_startPosition = m_selection.m_endPosition;
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
		if (!select) m_selection.m_startPosition = m_selection.m_endPosition;
		AdjustView();
	}

	void TextEditor::HandleDelete()
	{
		/*if (m_selection.m_endPosition != m_selection.m_startPosition)
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
		EmitValueChanged();*/
	}

	void TextEditor::HandleBackspace()
	{
		if (m_caretPos.column > 0)
		{
			m_lines[m_caretPos.line].erase(m_caretPos.column - 1, 1);
			m_caretPos.column--;
		} 
		else if (m_caretPos.line > 0)
		{
			// Unir con la línea de arriba
			size_t prevLineIdx = m_caretPos.line - 1;
			m_caretPos.column = m_lines[prevLineIdx].size(); // Guardar posición de unión
        
			m_lines[prevLineIdx] += m_lines[m_caretPos.line];
			m_lines.erase(m_lines.begin() + m_caretPos.line);
        
			m_caretPos.line = prevLineIdx;
		}
		
		/*if (m_caretPosition == 0 && m_selection.m_endPosition == m_selection.m_startPosition)
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
		EmitValueChanged();*/
	}

	void TextEditor::HandleEnter()
	{
		if (!m_features.isMultiLines) return;

		auto& currentLine = m_lines[m_caretPos.line];
		std::wstring remainingText = currentLine.substr(m_caretPos.column);
    
		// Cortamos la línea actual
		currentLine.erase(m_caretPos.column);
    
		// Insertamos nueva línea justo debajo
		m_lines.insert(m_lines.begin() + m_caretPos.line + 1, remainingText);
    
		// Movemos el caret al inicio de la nueva línea
		m_caretPos.line++;
		m_caretPos.column = 0;
	}

	void TextEditor::DeleteRange(TextPosition start, TextPosition end)
	{
		if (start == end) return;

		// Aseguramos que start sea el menor
		if (end < start) std::swap(start, end);

		if (start.line == end.line) {
			// Borrado en la misma línea
			m_lines[start.line].erase(start.column, end.column - start.column);
		} 
		else {
			// Borrado multilínea
			// 1. Conservamos el inicio de la primera línea y el final de la última
			std::wstring head = m_lines[start.line].substr(0, start.column);
			std::wstring tail = m_lines[end.line].substr(end.column);

			// 2. La línea de inicio ahora contiene la unión de ambos extremos
			m_lines[start.line] = head + tail;

			// 3. Eliminamos todas las líneas que quedaron en medio y la línea final original
			m_lines.erase(m_lines.begin() + start.line + 1, m_lines.begin() + end.line + 1);
		}

		// El cursor siempre queda donde empezó el borrado
		m_selection.Reset(start);
	}

	size_t TextEditor::GetVisualLineIndexFromPos(TextPosition position) const
	{
		for (size_t i = 0; i < m_visualLines.size(); ++i)
		{
			const auto& vl = m_visualLines[i];
			if (vl.logicalLineIndex == position.line && position.column >= vl.charStart && position.column <= vl.charStart + vl.charLength) {
				// Evitar quedarse atrapado al final de una línea visual envuelta
				if (position.column == vl.charStart + vl.charLength && i + 1 < m_visualLines.size())
				{
					if (m_visualLines[i+1].logicalLineIndex == position.line) continue;
				}
				return i;
			}
		}
		return 0;
	}

	float TextEditor::GetLineHeight() const
	{
		return static_cast<float>(m_graphics.GetTextExtent("Ay").Height);
	}

	void TextEditor::EnsureCaretVisible()
	{
	}

	void TextEditor::UpdateCaretPhysicalPosition(float lh)
	{
		auto& p = m_selection.m_endPosition;
		int x = (int)m_graphics.GetTextExtent(m_lines[p.line].substr(0, p.column)).Width;
		int y = (int)(p.line * lh) - m_offsetView.Y;
		m_caret->SetPosition({x, y});
		(y < 0 || y + lh > m_owner->ClientSize.Height) ? m_caret->Deactivate() : m_caret->Activate();
	}

	void TextEditor::SetContent(const std::wstring& newContent)
	{
		m_content = newContent;
		m_lines.clear();
    
		// Si el texto está vacío, garantizamos al menos una línea vacía
		if (m_content.empty())
		{
			m_lines.push_back(L"");
		}
		else
		{
			// Splitting del texto por saltos de línea (\n o \r\n)
			std::size_t start = 0, end;
			while ((end = m_content.find(L'\n', start)) != std::wstring::npos)
			{
				std::wstring line = m_content.substr(start, end - start);
				// Limpiar \r si existe (estilo Windows)
				if (!line.empty() && line.back() == L'\r') line.pop_back();
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

	void TextEditor::Render()
	{
		bool enabled = GUI::IsWindowEnabled(m_owner);
		m_graphics.DrawRectangle(m_owner->ClientSize.ToRectangle(), GetBackgroundColor(), true);
		auto clientSize = m_owner->ClientSize;
		float lineHeight = GetLineHeight();
		
		TextPosition s = m_selection.Min();
		TextPosition e = m_selection.Max();
		
		for (const auto& vl : m_visualLines)
		{
			float drawY = vl.y - m_offsetView.Y;
			if (drawY + lineHeight < 0 || drawY > clientSize.Height) 
				continue;
			
			std::wstring fragment = m_lines[vl.logicalLineIndex].substr(vl.charStart, vl.charLength);
			
			if (s != e && vl.logicalLineIndex >= s.line && vl.logicalLineIndex <= e.line)
			{
				// Calcular dónde empieza y termina la selección DENTRO de este fragmento
				size_t selStartInV = (vl.logicalLineIndex == s.line) ? (std::max)(vl.charStart, s.column) : vl.charStart;
				size_t selEndInV = (vl.logicalLineIndex == e.line) ? (std::min)(vl.charStart + vl.charLength, e.column) : (vl.charStart + vl.charLength);

				if (selStartInV < selEndInV)
				{
					float x1 = m_graphics.GetTextExtent(fragment.substr(0, selStartInV - vl.charStart)).Width;
					float w = m_graphics.GetTextExtent(fragment.substr(selStartInV - vl.charStart, selEndInV - selStartInV)).Width;
					m_graphics.DrawRectangle(Rectangle{(int)x1, (int)drawY, (uint32_t)w, (uint32_t)lineHeight}, Color(0, 120, 215, 128), true);
				}
			}
			m_graphics.DrawString({ 0, (int)drawY }, fragment, m_owner->Appearance->Foreground);
		}
		//UpdateCaretPhysicalPosition(lineHeight);
		
		if (m_caret->IsVisible())
		{
			//m_graphics.DrawLine({ two + m_offsetView + static_cast<int>(contentSize.Width), one + textOffset }, { two + m_offsetView + static_cast<int>(contentSize.Width), one + textOffset + static_cast<int>(caretHeight) }, m_owner->Appearance->Foreground2nd);
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
		m_features.isMultiLines = enable;
	}

	void TextEditor::SetWordWrap(bool enable)
	{
		m_features.wordWrap = enable;
	}

	void TextEditor::SetCharFilter(std::function<bool(wchar_t)> predicate)
	{
		m_predicate = std::move(predicate);
	}

	bool TextEditor::Deselect()
	{
		if (m_selection.IsEmpty()) return false;
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
		int viewHeight = m_owner->ClientSize.Height;
    
		size_t vIdx = GetVisualLineIndexFromPos(m_selection.m_endPosition);
		float caretY = m_visualLines[vIdx].y;

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
		int relativeY = mousePosition.Y - m_offsetView.Y;
		size_t lineIdx = (relativeY < 0) ? 0 : static_cast<size_t>(relativeY / GetLineHeight());
    
		if (lineIdx >= m_lines.size()) 
			lineIdx = m_lines.size() - 1;

		// 2. Determinar la columna basándonos en X
		const std::wstring& lineText = m_lines[lineIdx];
		size_t bestCol = 0;
		int minDiff = (std::numeric_limits<int>::max)();

		for (size_t i = 0; i <= lineText.size(); ++i)
		{
			auto extent = m_graphics.GetTextExtent(lineText.substr(0, i));
			int diff = std::abs(static_cast<int>(extent.Width) + m_offsetView.X - mousePosition.X);
			if (diff < minDiff)
			{
				minDiff = diff;
				bestCol = i;
			} else {
				break; // Optimización: si la diferencia empieza a crecer, ya pasamos el punto
			}
		}
		return { lineIdx, bestCol };
	}

	TextPosition TextEditor::GetPositionNextWord(TextPosition currentPosition, int direction) const
	{
		if (m_content.empty())
		{
			return currentPosition; //0
		}

		const std::wstring& line = m_lines[currentPosition.line];
		if (direction > 0) // Hacia la derecha
		{
			if (currentPosition.column >= line.size()) {
				// Si estamos al final de la línea, saltar al inicio de la siguiente
				if (currentPosition.line + 1 < m_lines.size()) 
					return { currentPosition.line + 1, 0 };
				return currentPosition;
			}
        
			size_t p = currentPosition.column;
			// Saltar espacios iniciales
			while (p < line.size() && iswspace(line[p])) p++;
			// Saltar caracteres de palabra
			while (p < line.size() && !iswspace(line[p])) p++;
        
			return { currentPosition.line, p };
		}
		else // Hacia la izquierda
		{
			if (currentPosition.column == 0) {
				// Si estamos al inicio, saltar al final de la anterior
				if (currentPosition.line > 0) 
					return { currentPosition.line - 1, m_lines[currentPosition.line - 1].size() };
				return currentPosition;
			}

			size_t p = currentPosition.column;
			// Retroceder espacios
			while (p > 0 && iswspace(line[p - 1])) p--;
			// Retroceder palabra
			while (p > 0 && !iswspace(line[p - 1])) p--;
        
			return { currentPosition.line, p };
		}
	}

	void TextEditor::RecomputeWordWrap()
	{
		m_visualLines.clear();
		if (m_lines.empty()) 
			return;

		float maxWidth = static_cast<float>(m_owner->ClientSize.Width) - 10.0f;
		auto lineHeight = GetLineHeight();
		float currentY = 0;

		for (size_t i = 0; i < m_lines.size(); ++i)
			{
			const std::wstring& lineText = m_lines[i];
        
			// Caso línea vacía (un \n solitario)
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
					// Medición progresiva: buscamos cuánto texto cabe
					// Nota: Un editor pro usaría búsqueda binaria aquí para velocidad
					while (start + count < lineText.size())
					{
						float w = m_graphics.GetTextExtent(lineText.substr(start, count + 1)).Width;
						if (w > maxWidth && count > 0) break; 
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
