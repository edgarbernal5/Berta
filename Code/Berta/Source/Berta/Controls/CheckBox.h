/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_CHECK_BOX_HEADER
#define BT_CHECK_BOX_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include <string>

namespace Berta
{
	struct CheckBoxEvents;

	class CheckBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;

		struct Module
		{
			void EmitCheckedChangedEvent();

			bool m_isChecked{ false };
			Window* m_window{ nullptr };
			CheckBoxEvents* m_events{ nullptr };
		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		enum class State
		{
			Normal,
			Pressed,
			Hovered
		};
		State m_status{ State::Normal };
		Module m_module;
	};

	struct ArgCheckBox
	{
		bool IsChecked{ false };
	};

	struct CheckBoxEvents : public ControlEvents
	{
		Event<ArgCheckBox> CheckedChanged;
	};

	class CheckBox : public Control<CheckBoxReactor, CheckBoxEvents>
	{
	public:
		CheckBox() = default;
		CheckBox(Window* parent, const Rectangle& rectangle);
		CheckBox(Window* parent, const Rectangle& rectangle, const std::wstring& text);

		bool IsChecked() const;
		void SetChecked(bool isChecked);
	};
}

#endif