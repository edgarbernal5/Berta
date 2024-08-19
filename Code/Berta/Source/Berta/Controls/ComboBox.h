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

		GUI::InteractionData& GetInteractionData() { return m_interactionData; }
		TextEditor* GetEditor() const { return m_textEditor; }

	private:
		enum class State
		{
			Normal,
			Pressed,
			Hovered
		};

		void EmitSelectionEvent(int index);

		ComboBox* m_control{ nullptr };
		TextEditor* m_textEditor{ nullptr };
		std::wstring m_text;

		State m_status{ State::Normal };
		GUI::InteractionData m_interactionData;
		
		FloatBox* m_floatBox{ nullptr };
	};

	class ComboBox : public Control<ComboBoxReactor, ComboboxEvents>
	{
	public:
		ComboBox() = default;
		ComboBox(Window* parent, const Rectangle& rectangle);

		void Clear();
		void Erase(uint32_t index);
		void PushItem(const std::wstring& text);
		int GetSelectedIndex() { m_reactor.GetInteractionData().m_selectedIndex; }
		void SetSelectedIndex(uint32_t index);

	protected:
		void DoOnCaption(const std::wstring& caption) override;
		std::wstring DoOnCaption() const override;

	private:
		bool m_isEditable{ false };
	};
}

#endif