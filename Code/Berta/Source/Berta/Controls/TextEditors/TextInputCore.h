/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TEXT_INPUT_CORE_HEADER
#define BT_TEXT_INPUT_CORE_HEADER

#include <string>
#include <functional>
#include "Berta/Controls/TextEditors/EnumTypes.h"
#include "Berta/GUI/ControlEvents.h"
#include "Berta/Core/BasicTypes.h"

namespace Berta
{
	
	struct Window;
	class Graphics;
	
	class TextInputCore
	{
	public:
		using TextEditorCallback = std::function<void()>;

		TextInputCore(Window* owner) : m_owner(owner) {}
		virtual ~TextInputCore() = default;

		void SetValueChangedCallback(const TextEditorCallback& callback) { m_valueChangedCallback = callback; }

		bool IsEditable() const { return m_isEditable; }
		void SetEditable(bool isEditable) { m_isEditable = isEditable; }

		void SetCharFilter(std::function<bool(wchar_t)> predicate) { m_predicate = std::move(predicate); }
		
		void SetBehavior(TextFocusBehavior behavior) { m_selection.Behavior = behavior; }
		TextPosition GetEndPosition() const { return m_selection.m_endPosition; }

		bool Deselect(); 

		virtual void Cut() = 0;
		virtual void Copy() const = 0;
		virtual void Paste() = 0;
		virtual bool SelectAll() = 0;
		
		virtual const std::wstring& GetContent() const = 0;
		virtual void SetContent(const std::wstring& newContent) = 0;
		virtual std::wstring GetSelectedText() const = 0;

		virtual void Render(Graphics& graphics) = 0;

		virtual bool OnKeyChar(const ArgKeyboard& args);
		virtual bool OnFocus(const ArgFocus& args) = 0;

	protected:
		enum class CharClass : uint8_t
		{
			Alphanumeric,
			Whitespace,
			Punctuation
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
		
		virtual void InsertChar(wchar_t wChr) = 0;

		void EmitSelectionChanged(TextPosition oldStart, TextPosition oldEnd) const;
		void EmitValueChanged() const;
		Color GetBackgroundColor() const;
		static CharClass GetCharClass(wchar_t c);

		Window* m_owner{ nullptr };
		Selection m_selection;
		TextEditorCallback m_valueChangedCallback;
		std::function<bool(wchar_t)> m_predicate;
		bool m_isEditable{ true };
	};
}

#endif