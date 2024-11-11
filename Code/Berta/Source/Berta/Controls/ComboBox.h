/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_COMBO_BOX_HEADER
#define BT_COMBO_BOX_HEADER

#include <string>
#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/Floating/InteractionData.h"
#include "Berta/Paint/Image.h"

namespace Berta
{
	class TextEditor;
	class FloatBox;
	class ComboBox;

	class ComboBoxReactor : public ControlReactor
	{
	public:
		~ComboBoxReactor();

		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;

		std::wstring GetText() const;
		void SetText(const std::wstring& text);

		enum class State
		{
			Normal,
			Pressed,
			Hovered
		};

		struct Module
		{
			Float::InteractionData Data;

			Window* m_owner{ nullptr };
			TextEditor* m_textEditor{ nullptr };
			std::wstring m_text;

			State m_status{ State::Normal };

			FloatBox* m_floatBox{ nullptr };

			void EmitSelectionEvent(int index);
		};

		void Clear();
		void Erase(uint32_t index);
		void PushItem(const std::wstring& text);
		void PushItem(const std::wstring& text, const Image& icon);
		uint32_t GetSelectedIndex() const;
		void SetSelectedIndex(uint32_t index);

	private:
		Module m_module;
	};

	class ComboBox : public Control<ComboBoxReactor, ComboboxEvents>
	{
	public:
		ComboBox() = default;
		ComboBox(Window* parent, const Rectangle& rectangle);

		void Clear();
		void Erase(uint32_t index);
		void PushItem(const std::wstring& text);
		void PushItem(const std::wstring& text, const Image& icon);
		int GetSelectedIndex() { m_reactor.GetSelectedIndex(); }
		void SetSelectedIndex(uint32_t index);

	protected:
		void DoOnCaption(const std::wstring& caption) override;
		std::wstring DoOnCaption() const override;

	private:
		bool m_isEditable{ false };
	};
}

#endif