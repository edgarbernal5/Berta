/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Berta/Controls/TextEditors/SingleLineEditor.h"

#include "Berta/API/PlatformAPI.h"
#include "Berta/GUI/Interface.h"
#include "Berta/GUI/Window.h"
#include "Berta/GUI/ControlAppearance.h"
#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	SingleLineEditor::SingleLineEditor(Window* owner) : TextInputCore(owner)
	{
		m_caret = std::make_unique<Caret>(owner, Size{ 2, 0 });
		
		m_selectionTimer.SetOwner(m_owner);
		m_selectionTimer.Connect([this](const ArgTimer& args)
		{
			if (!m_selection.m_isSelecting)
			{
				return;
			}

			int viewWidth = static_cast<int>(m_editorArea.Width);
			int previousScroll = m_scrollX;

			if (m_lastMousePosition.X < m_editorArea.X)
			{
				m_scrollX = std::max<int>(0, m_scrollX - TEXT_EDITOR_SCROLL_SPEED);
			}
			else if (m_lastMousePosition.X > m_editorArea.X + viewWidth)
			{
				int contentWidth = 0;
		#ifdef BT_PLATFORM_WINDOWS
				EnsureLayout();
				if (m_textHandle.m_textLayout)
				{
					DWRITE_TEXT_METRICS metrics{};
					if (SUCCEEDED(m_textHandle.m_textLayout->GetMetrics(&metrics)))
					{
						contentWidth = static_cast<int>(metrics.widthIncludingTrailingWhitespace);
					}
				}
		#else
				contentWidth = static_cast<int>(m_owner->Renderer.GetGraphics().GetTextExtent(m_content).Width);
		#endif
				int maxScroll = std::max<int>(0, contentWidth - viewWidth + 5);
				
				m_scrollX = std::min<int>(maxScroll, m_scrollX + TEXT_EDITOR_SCROLL_SPEED);
			}

			if (m_scrollX != previousScroll)
			{
				m_selection.m_endPosition = GetPositionUnderMouse(m_lastMousePosition);
				
				UpdateCaretPosition();
				GUI::UpdateWindow(m_owner);
			}
		});
	}

	void SingleLineEditor::SetEditorArea(const Rectangle& area)
	{
		m_editorArea = area;
		AdjustView();
	}

	std::wstring SingleLineEditor::GetContent() const
	{
		return m_content;
	}

	void SingleLineEditor::SetContent(const std::wstring& newContent)
	{
		m_content = newContent;
		
		// Limpiar cualquier salto de línea que intente colarse
		m_content.erase(std::remove(m_content.begin(), m_content.end(), L'\n'), m_content.end());
		m_content.erase(std::remove(m_content.begin(), m_content.end(), L'\r'), m_content.end());
		
		InvalidateLayout();
		m_selection.Reset({ 0, 0 });
		AdjustView();
	}

	std::wstring SingleLineEditor::GetSelectedText() const
	{
		auto minCol = m_selection.Min().column;
		auto maxCol = m_selection.Max().column;
		return m_content.substr(minCol, maxCol - minCol);
	}

	void SingleLineEditor::InsertChar(wchar_t wChr)
	{
		// Cero saltos de línea permitidos en este editor
		if (wChr == L'\n' || wChr == L'\r') return;

		if (!m_selection.IsEmpty())
		{
			DeleteRange(m_selection.Min(), m_selection.Max());
		}

		m_content.insert(m_selection.m_endPosition.column, 1, wChr);
		m_selection.m_endPosition.column++;
		m_selection.m_startPosition = m_selection.m_endPosition;
		
		InvalidateLayout();
		AdjustView();
		EmitValueChanged();
	}

	void SingleLineEditor::DeleteRange(TextPosition start, TextPosition end)
	{
		if (start == end)
		{
			return;
		}
		
		m_content.erase(start.column, end.column - start.column);
		InvalidateLayout();
		m_selection.Reset(start);
	}

	int SingleLineEditor::GetCaretX() const
	{
		if (m_content.empty() || m_selection.m_endPosition.column == 0)
		{
			return 0;
		}

		int caretX = 0;

#ifdef BT_PLATFORM_WINDOWS
		EnsureLayout();

		if (m_textHandle.m_textLayout)
		{
			FLOAT hitX = 0.0f, hitY = 0.0f;
			DWRITE_HIT_TEST_METRICS metrics{};

			uint32_t localCharIndex = static_cast<uint32_t>(m_selection.m_endPosition.column);

			HRESULT hr = m_textHandle.m_textLayout->HitTestTextPosition(
				localCharIndex,
				FALSE, // isTrailingHit = FALSE para obtener el borde izquierdo de la posición
				&hitX,
				&hitY,
				&metrics
			);

			if (SUCCEEDED(hr))
			{
				caretX = static_cast<int>(hitX);
			}
		}
#else
		// Fallback por medición para plataformas no-Windows
		caretX = static_cast<int>(m_owner->Renderer.GetGraphics().GetTextExtent(m_content.substr(0, m_selection.m_endPosition.column)).Width);
#endif

		return caretX;
	}

	void SingleLineEditor::HandleDelete()
	{
		if (!m_selection.IsEmpty())
		{
			DeleteRange(m_selection.Min(), m_selection.Max());
			EmitValueChanged();
			return;
		}

		if (m_selection.m_endPosition.column < m_content.length())
		{
			m_content.erase(m_selection.m_endPosition.column, 1);
			InvalidateLayout();
			EmitValueChanged();
		}
	}

	void SingleLineEditor::HandleBackspace()
	{
		if (!m_selection.IsEmpty())
		{
			DeleteRange(m_selection.Min(), m_selection.Max());
			EmitValueChanged();
			return;
		}

		if (m_selection.m_endPosition.column > 0)
		{
			m_content.erase(m_selection.m_endPosition.column - 1, 1);
			m_selection.m_endPosition.column--;
			m_selection.m_startPosition = m_selection.m_endPosition;
			
			InvalidateLayout();
			AdjustView();
			EmitValueChanged();
		}
	}

	void SingleLineEditor::Render(Graphics& graphics)
	{
		// 1. Fondo
		graphics.FillRectangle(m_editorArea, GetBackgroundColor());

		uint32_t lineHeight = graphics.GetTextExtent("Ay").Height;
		int textY = std::max<int>(0, (static_cast<int>(m_editorArea.Height) - static_cast<int>(lineHeight)) / 2);
		
		graphics.SetClipping(m_editorArea);
		graphics.PushTranslation(m_editorArea.X - m_scrollX, m_editorArea.Y + textY);
		
		EnsureLayout();
		
		// 2. Fondo de Selección
		if (!m_selection.IsEmpty())
		{
			auto minCol = m_selection.Min().column;
			auto maxCol = m_selection.Max().column;

			int startX = graphics.GetTextExtent(m_content.substr(0, minCol)).Width;
			int selWidth = graphics.GetTextExtent(m_content.substr(minCol, maxCol - minCol)).Width;
        
			// La selección cubre exactamente la altura de la línea
			Rectangle selRect{ startX, 0, static_cast<uint32_t>(selWidth), lineHeight };
			graphics.FillRectangle(selRect, m_owner->Appearance->HighlightColor);
		}

		// 3. Texto Lineal Rápido
		// El área de dibujo es "infinita" en el eje X porque el recorte ya se encarga de limitar la vista
#ifdef BT_PLATFORM_WINDOWS
		if (m_textHandle.IsValid())
		{
			graphics.DrawTextLayout(m_textHandle, { 0, 0 }, m_owner->Appearance->Foreground);
		}
#else
		graphics.DrawText(m_content, { 0, 0, 100000, lineHeight }, m_owner->Appearance->Foreground);
#endif
		
		// 4. Caret
		if (m_caret && m_caret->IsVisible())
		{
			int caretX = GetCaretX();
			auto caretHeight = graphics.GetCaretHeight();

			graphics.DrawLine(
				{ caretX, 0 }, 
				{ caretX, static_cast<int>(caretHeight) }, 
				static_cast<float>(m_owner->ToScale(m_caret->GetSize().Width)),
				m_owner->Appearance->Foreground2nd
			);
		}

		graphics.PopTranslation();
		graphics.EndClipping();
	}

	bool SingleLineEditor::OnFocus(const ArgFocus& args)
	{
		bool needUpdate = false;
		if (args.Focused)
		{
			if (m_selection.Behavior == TextFocusBehavior::Select || 
			   (m_selection.Behavior == TextFocusBehavior::SelectOnClick && args.FocusReason == ArgFocus::Reason::MousePress))
			{
				SelectAll();
				needUpdate = true;
				m_selection.m_ignoreMouseDown = (args.FocusReason == ArgFocus::Reason::MousePress);
			}
			m_caret->Activate();
		}
		else
		{
			if (m_selection.Behavior != TextFocusBehavior::None)
			{
				Deselect();
			}
			m_caret->Deactivate();
			m_selection.m_isSelecting = false;
			needUpdate = true;
		}
		return needUpdate;
	}

	void SingleLineEditor::OnMouseDown(const ArgMouse& args)
	{
		if (m_selection.m_ignoreMouseDown || m_content.empty())
		{
			return;
		}
	
		m_lastMousePosition = args.Position;
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

	void SingleLineEditor::OnMouseMove(const ArgMouse& args)
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
				m_selectionTimer.SetInterval(90);
				m_selectionTimer.Start();
			}
			else if (selectionTimerRunning && insideEditorArea)
			{
				m_selectionTimer.Stop();
			}
		}

		m_lastMousePosition = args.Position;
	}

	void SingleLineEditor::OnMouseUp(const ArgMouse& args)
	{
		if (m_selection.m_ignoreMouseDown)
		{
			m_selection.m_ignoreMouseDown = false;
			return;
		}
	
		m_selectionTimer.Stop();
		if (m_wasDblClick || m_content.empty())
		{
			m_wasDblClick = false;
			return;
		}

		GUI::ReleaseCapture(m_owner);
	
		m_selection.m_endPosition = GetPositionUnderMouse(args.Position);
		m_selection.m_isSelecting = false;
		GUI::MarkAsNeedUpdate(m_owner);
	}

	bool SingleLineEditor::OnKeyPressed(const ArgKeyboard& args)
	{
		bool redraw = false;
		auto savedEndPos = m_selection.m_endPosition;

		switch (args.Key)
		{
		case KeyboardKey::ArrowLeft:
			MoveCaretHorizontal(-1, args.ButtonState.Ctrl, args.ButtonState.Shift);
			redraw = true;
			break;

		case KeyboardKey::ArrowRight:
			MoveCaretHorizontal(1, args.ButtonState.Ctrl, args.ButtonState.Shift);
			redraw = true;
			break;

		case KeyboardKey::Home:
			m_selection.m_endPosition.column = 0;
			if (!args.ButtonState.Shift)
			{
				m_selection.m_startPosition = m_selection.m_endPosition;
			}
			AdjustView();
			redraw = true;
			break;

		case KeyboardKey::End:
			m_selection.m_endPosition.column = m_content.length();
			if (!args.ButtonState.Shift)
			{
				m_selection.m_startPosition = m_selection.m_endPosition;
			}
			
			AdjustView();
			redraw = true;
			break;

		case KeyboardKey::Delete:
			if (args.ButtonState.Ctrl && m_selection.IsEmpty())
			{
				// Seleccionar la palabra hacia la derecha y borrarla
				TextPosition end = GetPositionNextWord(m_selection.m_endPosition, 1);
				DeleteRange(m_selection.m_endPosition, end);
				EmitValueChanged();
			}
			else
			{
				HandleDelete();
			}
			redraw = true;
			break;

		case KeyboardKey::Backspace:
			if (args.ButtonState.Ctrl && m_selection.IsEmpty())
			{
				// Seleccionar la palabra hacia la izquierda y borrarla
				TextPosition start = GetPositionNextWord(m_selection.m_endPosition, -1);
				DeleteRange(start, m_selection.m_endPosition);
				EmitValueChanged();
			}
			else
			{
				HandleBackspace();
			}
			redraw = true;
			break;

		case KeyboardKey::Enter:
			// En un SingleLineEditor, Enter no hace nada visualmente. 
			// El Reactor del LineTextBox debería interceptarlo para emitir OnSubmit.
			break;
		}

		if (redraw)
		{
			UpdateCaretPosition();
		}
		return redraw;
	}

	bool SingleLineEditor::OnDblClick(const ArgMouse& args)
	{
		m_wasDblClick = true;
		if (m_content.empty())
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

	void SingleLineEditor::OnResize(const ArgResize& args)
	{
		AdjustView();
		UpdateCaretPosition();
	}

	void SingleLineEditor::MoveCaretHorizontal(int direction, bool wordJump, bool select)
	{
		if (wordJump)
		{
			m_selection.m_endPosition = GetPositionNextWord(m_selection.m_endPosition, direction);
		}
		else
		{
			if (direction < 0 && m_selection.m_endPosition.column > 0)
			{
				m_selection.m_endPosition.column--;
			}
			else if (direction > 0 && m_selection.m_endPosition.column < m_content.length())
			{
				m_selection.m_endPosition.column++;
			}
		}

		if (!select)
		{
			m_selection.m_startPosition = m_selection.m_endPosition;
		}

		AdjustView();
	}

	void SingleLineEditor::AdjustView()
	{
		if (m_content.empty())
		{
			m_scrollX = 0;
			return;
		}

		int caretX = GetCaretX();
		int viewWidth = static_cast<int>(m_editorArea.Width);

		// Mantener el cursor dentro del área visible de la caja de texto
		if (caretX > m_scrollX + viewWidth - 5) // Margen de 5px a la derecha
		{
			m_scrollX = caretX - viewWidth + 5;
		}
		else if (caretX < m_scrollX + 5) // Margen de 5px a la izquierda
		{
			m_scrollX = std::max<int>(0, caretX - 5);
		}
	}

	TextPosition SingleLineEditor::GetPositionNextWord(TextPosition currentPos, int direction) const
	{
		if (m_content.empty())
		{
			return { 0, 0 };
		}

		size_t col = currentPos.column;
		size_t len = m_content.length();

		if (direction > 0) // Mover hacia la derecha (Ctrl + Right)
		{
			// 1. Saltar espacios o puntuación inicial
			while (col < len && GetCharClass(m_content[col]) != CharClass::Alphanumeric)
			{
				col++;
			}
			// 2. Saltar el bloque de la palabra
			while (col < len && GetCharClass(m_content[col]) == CharClass::Alphanumeric)
			{
				col++;
			}
		}
		else // Mover hacia la izquierda (Ctrl + Left)
		{
			// 1. Retroceder espacios/puntuación iniciales
			while (col > 0 && GetCharClass(m_content[col - 1]) != CharClass::Alphanumeric)
			{
				col--;
			}
			// 2. Retroceder el bloque de la palabra
			while (col > 0 && GetCharClass(m_content[col - 1]) == CharClass::Alphanumeric)
			{
				col--;
			}
		}

		return { 0, col };
	}

	TextPosition SingleLineEditor::GetPositionUnderMouse(const Point& mousePosition) const
	{
		if (m_content.empty())
		{
			return { 0, 0 };
		}

		int localX = mousePosition.X - m_editorArea.X + m_scrollX;
		if (localX <= 0)
		{
			return { 0, 0 };
		}

		TextPosition pos{ 0, 0 };

#ifdef BT_PLATFORM_WINDOWS
		EnsureLayout();

		if (m_textHandle.m_textLayout)
		{
			BOOL isTrailingHit = FALSE;
			BOOL isInside = FALSE;
			DWRITE_HIT_TEST_METRICS metrics{};

			HRESULT hr = m_textHandle.m_textLayout->HitTestPoint(
				static_cast<FLOAT>(localX),
				0.0f, // Para una sola línea, Y local es 0.0f
				&isTrailingHit,
				&isInside,
				&metrics
			);

			if (SUCCEEDED(hr))
			{
				pos.column = metrics.textPosition;
				if (isTrailingHit)
				{
					pos.column += metrics.length;
				}
			}
		}
		else
		{
			pos.column = m_content.length();
		}
#else
		// Fallback mediante Búsqueda Binaria para otras plataformas
		size_t low = 0, high = m_content.length(), foundOffset = 0;

		while (low <= high)
		{
			size_t mid = low + (high - low) / 2;
			uint32_t width = m_owner->Renderer.GetGraphics().GetTextExtent(m_content.substr(0, mid)).Width;

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

		if (foundOffset < m_content.length())
		{
			uint32_t widthBefore = m_owner->Renderer.GetGraphics().GetTextExtent(m_content.substr(0, foundOffset)).Width;
			uint32_t widthAfter = m_owner->Renderer.GetGraphics().GetTextExtent(m_content.substr(0, foundOffset + 1)).Width;
			uint32_t halfChar = widthBefore + (widthAfter - widthBefore) / 2;

			if (static_cast<uint32_t>(localX) > halfChar)
			{
				foundOffset++;
			}
		}

		pos.column = foundOffset;
#endif

		return pos;
	}

	std::pair<TextPosition, TextPosition> SingleLineEditor::GetWordBounds(TextPosition pos) const
	{
		if (pos.column >= m_content.length())
		{
			return { pos, pos };
		}

		CharClass targetClass = GetCharClass(m_content[pos.column]);

		size_t startCol = pos.column;
		size_t endCol = pos.column;

		while (startCol > 0 && GetCharClass(m_content[startCol - 1]) == targetClass)
		{
			startCol--;
		}

		while (endCol < m_content.length() && GetCharClass(m_content[endCol]) == targetClass)
		{
			endCol++;
		}

		return { { 0, startCol }, { 0, endCol } };
	}

	void SingleLineEditor::EnsureLayout() const
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_textHandle.IsValid() || m_content.empty())
		{
			return; // Ya está en caché o no hay nada que dibujar
		}

		auto nativeAttr = m_owner->Renderer.GetGraphics().GetNativeHandle();
		if (!nativeAttr || !nativeAttr->m_textFormat)
		{
			return;
		}
		
		// Ancho infinito para evitar que DirectWrite intente hacer salto de línea
		float layoutWidth = 1000000.0f; 
		float layoutHeight = static_cast<float>(m_editorArea.Height);

		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextLayout
		(
			m_content.c_str(),
			static_cast<UINT32>(m_content.length()),
			nativeAttr->m_textFormat,
			layoutWidth,
			layoutHeight,
			&m_textHandle.m_textLayout
		);
#endif
	}

	void SingleLineEditor::InvalidateLayout()
	{
#ifdef BT_PLATFORM_WINDOWS
		m_textHandle.Release();
#endif
	}

	// --- Lógica del Portapapeles (Rápida y directa) ---
	void SingleLineEditor::Cut()
	{
		if (m_selection.IsEmpty())
		{
			return;
		}
		
		Copy();
		DeleteRange(m_selection.Min(), m_selection.Max());
		EmitValueChanged();
	}

	void SingleLineEditor::Copy() const
	{
		std::wstring selectedText = GetSelectedText();
		if (!selectedText.empty())
		{
			Platform::SetClipboardText(selectedText, m_owner->RootHandle);
		}
	}

	void SingleLineEditor::Paste()
	{
		std::wstring clipboardText;
		if (!Platform::GetClipboardText(clipboardText) || clipboardText.empty())
		{
			return;
		}
		
		// Limpiamos la cadena que viene del portapapeles
		clipboardText.erase(std::remove(clipboardText.begin(), clipboardText.end(), L'\n'), clipboardText.end());
		clipboardText.erase(std::remove(clipboardText.begin(), clipboardText.end(), L'\r'), clipboardText.end());

		if (!m_selection.IsEmpty())
		{
			DeleteRange(m_selection.Min(), m_selection.Max());
		}
		m_content.insert(m_selection.m_endPosition.column, clipboardText);
		m_selection.m_endPosition.column += clipboardText.length();
		m_selection.m_startPosition = m_selection.m_endPosition;
		
		InvalidateLayout();
		AdjustView();
		EmitValueChanged();
	}

	bool SingleLineEditor::SelectAll()
	{
		m_selection.Reset({ 0, 0 });
		m_selection.m_endPosition = { 0, m_content.length() };
		AdjustView();
		
		return true;
	}

	void SingleLineEditor::UpdateCaretPosition()
	{
		uint32_t lineHeight = m_owner->Renderer.GetGraphics().GetTextExtent("Ay").Height;
		int textY = std::max<int>(0, (static_cast<int>(m_editorArea.Height) - static_cast<int>(lineHeight)) / 2);

		int caretX = GetCaretX();

		// La coordenada Y relativa al espacio del editor alineada con el texto
		m_caret->SetPosition({ caretX, textY });
	}

	// Implementar delegados de ratón (OnMouseDown, etc.) idénticos a la clase base original 
	// pero usando GetPositionUnderMouse() local.
}
