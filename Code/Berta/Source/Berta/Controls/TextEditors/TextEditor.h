/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TEXT_EDITOR_HEADER
#define BT_TEXT_EDITOR_HEADER

#include <string>
#include <functional>

#include "Berta/Paint/Graphics.h"
#include "Berta/GUI/ControlEvents.h"
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
		bool OnDblClick(const ArgMouse& args);

		void SetValueChangedCallback(TextEditorCallback callback) { m_valueChangedCallback = callback; }
		size_t GetCaretPosition() const { return m_caretPosition; }
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

		void AdjustView();
		Size GetContentTextExtent(size_t position = 0) const;
		size_t GetPositionUnderMouse(const Point& mousePosition) const;
		size_t GetPositionNextWord(int64_t currentPosition, int direction) const;

		void EmitValueChanged();

		Graphics& m_graphics;
		size_t m_caretPosition{ 0 };
		int64_t m_selectionStartPosition{ -1 };
		int64_t m_selectionEndPosition{ -1 };
		bool isSelecting{ false };
		int m_offsetView{ 0 };
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
	};
}

#endif