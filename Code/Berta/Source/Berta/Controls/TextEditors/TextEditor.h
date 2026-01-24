/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TEXT_EDITOR_HEADER
#define BT_TEXT_EDITOR_HEADER

#include <string>
#include <functional>

#include "Berta/Controls/TextEditors/TextEditorBase.h"
#include "Berta/Paint/Graphics.h"
#include "Berta/GUI/ControlEvents.h"
//#include "Berta/GUI/Caret.h"
#include "Berta/Core/Timer.h"

namespace Berta
{
	struct Window;
	class Caret;

	class TextEditor
	{
	public:
		using TextEditorCallback = std::function<void()>;

	public:
		TextEditor(Window* owner, Graphics* graphics);
		~TextEditor();
		
		void OnMouseEnter(const ArgMouse& args);
		void OnMouseLeave(const ArgMouse& args);
		void OnMouseDown(const ArgMouse& args);
		void OnMouseMove(const ArgMouse& args);
		void OnMouseUp(const ArgMouse& args);
		bool OnFocus(const ArgFocus& args);
		bool OnKeyChar(const ArgKeyboard& args);
		bool OnKeyPressed(const ArgKeyboard& args);
		bool OnKeyReleased(const ArgKeyboard& args);
		bool OnDblClick(const ArgMouse& args);
		void OnResize(ArgResize args);

		void SetValueChangedCallback(const TextEditorCallback& callback) { m_valueChangedCallback = callback; }

		std::wstring GetContent() const;
		void SetContent(const std::wstring& newContent);
		void SetContent(const std::string& newContent);
		std::wstring GetSelectedText() const;
		
		void Copy();
		void Cut();
		void Paste();
		
		void SetEditorArea(const Rectangle& area);
		void Render();

		bool IsEditable() const;
		void SetEditable(bool isEditable);
		
		void SetMultiline(bool enable);
		void SetWordWrap(bool enable);
		
		void SetCharFilter(std::function<bool(wchar_t)> predicate);
		void SetBehavior(TextFocusBehavior behavior)
		{
			m_selection.Behavior = behavior;
		}
		
		TextPosition GetEndPosition() const;

		bool Deselect();
		bool SelectAll();

	private:
		struct Features
		{
			bool isEditable{ true };
			bool isMultiLines{ false };
			bool wordWrap{ false };
		};

		struct Selection
		{
			TextPosition m_startPosition{ 0, 0 };
			TextPosition m_endPosition{ 0, 0 };
			bool m_isSelecting{ false };
			bool m_ignoreMouseDown{ false };
			TextFocusBehavior Behavior{ TextFocusBehavior::None };

			bool IsEmpty() const { return m_startPosition == m_endPosition; }
			TextPosition Min() const { return (m_startPosition < m_endPosition) ? m_startPosition : m_endPosition; }
			TextPosition Max() const { return (m_startPosition > m_endPosition) ? m_startPosition : m_endPosition; }
			void Reset(TextPosition position) { m_startPosition = m_endPosition = position; }
		};

		void ActivateCaret();
		void DeactivateCaret();

		void InsertChar(wchar_t chr);

		void MoveCaretLeft(bool wordJump, bool select);
		void MoveCaretHome(bool select);
		void MoveCaretRight(bool wordJump, bool select);
		void MoveCaretEnd(bool select);
		
		void MoveCaretUp(bool select);
		void MoveCaretDown(bool select);
		
		void HandleDelete();
		void HandleBackspace();
		void HandleEnter();
		
		void DeleteRange(TextPosition start, TextPosition end);
		
		size_t GetVisualLineIndexFromPos(TextPosition position) const;
		uint32_t GetLineHeight() const;
		
		void AdjustView();
		TextPosition GetPositionUnderMouse(const Point& mousePosition) const;
		TextPosition GetPositionNextWord(TextPosition currentPosition, int direction) const;

		Size GetContentTextExtent() const;
		void RecomputeWordWrap();
		void EmitValueChanged() const;
		
		Color GetBackgroundColor() const;

		std::vector<std::wstring> m_lines{ L"" };
		std::vector<VisualLine> m_visualLines;
		Selection m_selection;
		Rectangle m_editorArea;
		
		Graphics& m_graphics;
		Point m_offsetView{ 0, 0 };
		bool m_shiftPressed{ false };
		bool m_ctrlPressed{ false };
		bool m_wasDblClick{ false };

		Point m_selectionMousePosition;
		Point m_lastMousePosition;
		Timer m_selectionTimer;
		Point m_selectionDirection{ 0,0 };
		uint32_t m_cachedMaxWidth { 0 };
		Caret* m_caret{ nullptr };
		Window* m_owner{ nullptr };
		
		TextEditorCallback m_valueChangedCallback;

		Features m_features;
		std::function<bool(wchar_t)> m_predicate;
	};
}

#endif