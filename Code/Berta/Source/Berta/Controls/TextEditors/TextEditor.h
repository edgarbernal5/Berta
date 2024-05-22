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
		void OnMouseUp(const ArgMouse& args);
		void OnFocus(const ArgFocus& args);
		bool OnKeyChar(const ArgKeyboard& args);
		bool OnKeyPressed(const ArgKeyboard& args);

		uint32_t GetCaretPosition() const { return m_caretPosition; }
		const std::wstring& GetContent() const { return m_content; }

		void Render();

	private:
		void ActivateCaret();
		void DeactivateCaret();

		void Insert(wchar_t chr);

		void MoveCaretLeft();
		void MoveCaretRight();
		void Delete();
		void DeleteBack();

		void AdjustView(bool scrollToLeft = false);
		Size GetContentSize();
		uint32_t GetPositionUnderMouse(const Point& mousePosition);

		Graphics& m_graphics;
		uint32_t m_caretPosition{ 0 };
		uint32_t m_caretOriginPosition{ 0 };
		int m_offset{ 0 };
		std::wstring m_content;
		Caret* m_caret{ nullptr };
		Window* m_owner{ nullptr };
	};
}

#endif