/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TEXT_EDITOR_HEADER
#define BT_TEXT_EDITOR_HEADER

#include <string>
#include "Berta/Paint/Graphics.h"
#include "Berta/GUI/CommonEvents.h"

namespace Berta
{
	struct Window;
	class Caret;

	class TextEditor
	{
	public:
		TextEditor(Window* owner);
		~TextEditor();

		void OnMouseEnter(const ArgMouse& args);
		void OnMouseLeave(const ArgMouse& args);
		void OnMouseDown(const ArgMouse& args);
		void OnMouseMove(const ArgMouse& args);
		void OnMouseUp(const ArgMouse& args);
		void OnFocus(const ArgFocus& args);
		bool OnKeyChar(const ArgKeyboard& args);
		bool OnKeyPressed(const ArgKeyboard& args);
		bool OnKeyReleased(const ArgKeyboard& args);
		bool OnDblClick(const ArgClick& args);

		uint32_t GetCaretPosition() const { return m_caretPosition; }
		const std::wstring& GetContent() const { return m_content; }
		void SetContent(const std::wstring& newContent) { m_content = newContent; }

		void Render();

	private:
		void ActivateCaret();
		void DeactivateCaret();

		void Insert(wchar_t chr);

		void MoveCaretLeft();
		void MoveCaretHome();
		void MoveCaretRight();
		void MoveCaretEnd();
		void Delete();
		void DeleteBack();

		void AdjustView(bool scrollToLeft = false);
		Size GetContentTextExtent() const;
		uint32_t GetPositionUnderMouse(const Point& mousePosition) const;
		uint32_t GetPositionNextWord(int currentPosition, int direction) const;
		Graphics& m_graphics;
		uint32_t m_caretPosition{ 0 };
		uint32_t m_selectionStartPosition{ 0 };
		uint32_t m_selectionEndPosition{ 0 };
		bool isSelecting{ false };
		int m_offsetView{ 0 };
		std::wstring m_content;
		bool m_shiftPressed{ false };
		bool m_ctrlPressed{ false };
		bool m_wasDblClick{ false };

		Caret* m_caret{ nullptr };
		Window* m_owner{ nullptr };
	};
}

#endif