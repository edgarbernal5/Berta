/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_MULTI_LINE_EDITOR_HEADER
#define BT_MULTI_LINE_EDITOR_HEADER

#include "Berta/Controls/TextEditors/TextInputCore.h"
#include "Berta/GUI/ScrollableView.h"
#include "Berta/GUI/Caret.h"
#include "Berta/Core/Timer.h"

namespace Berta
{
	class MultiLineEditor : public TextInputCore
	{
	public:
		MultiLineEditor(Window* owner);
		~MultiLineEditor() override = default;

		// --- Interfaz Implementada de TextInputCore ---
		void Cut() override;
		void Copy() const override;
		void Paste() override;
		bool SelectAll() override;

		std::wstring GetContent() const override;
		void SetContent(const std::wstring& newContent) override;
		std::wstring GetSelectedText() const override;

		void Render(Graphics& graphics) override;
		bool OnFocus(const ArgFocus& args) override;

		// --- Eventos Específicos ---
		void OnMouseDown(const ArgMouse& args);
		void OnMouseMove(const ArgMouse& args);
		void OnMouseUp(const ArgMouse& args);
		bool OnKeyPressed(const ArgKeyboard& args);
		bool OnDblClick(const ArgMouse& args);
		void OnResize(const ArgResize& args);
		void OnDpiChanged();

		void SetEditorArea(const Rectangle& area);
		void SetScrollBarVisibility(ScrollBarVisibility vertical, ScrollBarVisibility horizontal);
		void SetWordWrap(bool enabled);

		void LoadFile(const std::string& path);
		void SaveFile(const std::string& path) const;

	protected:
		void InsertChar(wchar_t wChr) override;

	private:
		// Toda la lógica compleja de geometría y salto de línea se conserva aquí
		void RecomputeWordWrap();
		void ComputeVisualLinesForLogicalLine(Graphics& graphics, size_t logicalIndex, uint32_t& yOffset, std::vector<VisualLine>& outList, uint32_t layoutWidth);
		void UpdateLinesIncremental(size_t startLine, int lineCountDelta);
		void EnsureLayout(const VisualLine& vl) const;
		void InvalidateLayoutsForLogicalLine(size_t logicalIndex) const;

		void MoveCaretHome(bool select);
		void MoveCaretEnd(bool select);
		void MoveCaretVertically(int direction, bool select);
		void MoveCaretHorizontal(int direction, bool wordJump, bool select);
		void HandleDelete();
		void HandleBackspace();
		void DeleteRange(TextPosition start, TextPosition end);

		void RenderVisibleLines(Graphics& graphics);
		void RenderCaret(Graphics& graphics);
		void DrawSelectionBackground(Graphics& graphics, const VisualLine& vl) const;

		[[nodiscard]] std::pair<uint32_t, uint32_t> GetSelectionRangeForLine(const VisualLine& vl) const;
		size_t GetVisualLineIndexFromPos(TextPosition position) const;
		size_t GetFirstVisibleVisualLine() const;
		uint32_t GetLineHeight() const;

		void AdjustView() const;
		void UpdateScrollMetrics() const;
		void CommitDocumentChange();

		TextPosition GetPositionUnderMouse(const Point& mousePosition) const;
		TextPosition GetPositionNextWord(TextPosition currentPosition, int direction) const;
		Point GetPointFromPosition(TextPosition pos) const;
		void UpdateCaretPosition();

		[[nodiscard]] Size GetContentTextExtent() const;
		[[nodiscard]] Point ToEditorSpace(Point windowPoint) const;
		[[nodiscard]] std::pair<TextPosition, TextPosition> GetWordBounds(TextPosition pos) const;

		// Estructuras pesadas exclusivas del editor multilínea
		std::vector<std::wstring> m_lines{ L"" };
		std::vector<VisualLine> m_visualLines;
		std::unique_ptr<ScrollableView> m_scrollableView;

		Rectangle m_editorArea;
		std::unique_ptr<Caret> m_caret;
		
		bool m_wordWrap{ false };
		bool m_wasDblClick{ false };
		uint32_t m_cachedMaxWidth{ 0 };

		Point m_lastMousePosition;
		Timer m_selectionTimer;
		Point m_selectionDirection{ 0,0 };
	};
}

#endif