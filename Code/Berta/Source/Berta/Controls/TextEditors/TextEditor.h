/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TEXT_EDITOR_HEADER
#define BT_TEXT_EDITOR_HEADER

#include <string>
#include "Berta/Paint/Graphics.h"

namespace Berta
{
	struct Window;
	class Caret;

	class TextEditor
	{
	public:
		TextEditor(Window* owner);

		void ActivateCaret();
		void DeactivateCaret();

		uint32_t GetCaretPosition() const { return m_caretPosition; }
		const std::wstring& GetContent() const { return m_content; }
		void Insert(wchar_t chr);

		void MoveCaretLeft();
		void MoveCaretRight();
		void Delete();
		void DeleteBack();
		void Render();
	private:
		Graphics& m_graphics;
		uint32_t m_caretPosition{ 0 };
		std::wstring m_content;
		Caret* m_caret{ nullptr };
		Window* m_owner{ nullptr };
	};
}

#endif