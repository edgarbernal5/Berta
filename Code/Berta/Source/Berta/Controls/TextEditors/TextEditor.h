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

		void SetValueChangedCallback(const TextEditorCallback& callback) { m_valueChangedCallback = callback; }

		const std::wstring& GetContent() const { return m_content; }
		void SetContent(const std::wstring& newContent);
		void SetContent(const std::string& newContent);

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

		void MoveCaretLeft(bool select);
		void MoveCaretHome(bool select);
		void MoveCaretRight(bool select);
		void MoveCaretEnd(bool select);
		
		void MoveCaretUp(bool select);
		void MoveCaretDown(bool select);
		
		void HandleDelete();
		void HandleBackspace();
		void HandleEnter();
		
		void DeleteRange(TextPosition start, TextPosition end);
		
		size_t GetVisualLineIndexFromPos(TextPosition position) const;
		float GetLineHeight() const;
		
		void AdjustView();
		Size GetContentTextExtent(size_t position = 0) const;
		TextPosition GetPositionUnderMouse(const Point& mousePosition) const;
		TextPosition GetPositionNextWord(TextPosition currentPosition, int direction) const;

		void RecomputeWordWrap();
		void EmitValueChanged() const;
		
		Color GetBackgroundColor() const;

		std::vector<std::wstring> m_lines{ L"" };
		std::vector<VisualLine> m_visualLines;
		Selection m_selection;
		
		Graphics& m_graphics;
		Point m_offsetView{ 0, 0 };
		std::wstring m_content;
		bool m_shiftPressed{ false };
		bool m_ctrlPressed{ false };
		bool m_wasDblClick{ false };

		Point m_selectionMousePosition;
		Timer m_selectionTimer;
		bool m_selectionDirection{ false };

		Caret* m_caret{ nullptr };
		Window* m_owner{ nullptr };
		TextEditorCallback m_valueChangedCallback;

		Features m_features;
		std::function<bool(wchar_t)> m_predicate;
	};
}

#endif