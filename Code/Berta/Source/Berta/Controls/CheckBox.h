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
	namespace Internal::CheckBox
	{
		struct Events;
		
		class Reactor : public ControlReactor
		{
		public:
			void Update(Graphics& graphics) override;

			void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
			void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
			void MouseDown(Graphics& graphics, const ArgMouse& args) override;
			void MouseUp(Graphics& graphics, const ArgMouse& args) override;

			struct Module
			{
				void EmitCheckedChangedEvent() const;

				CheckState m_checkState{ CheckState::Unchecked };
				Window* m_window{ nullptr };
				Events* m_events{ nullptr };
			};

			Module& GetModule() { return m_module; }
			const Module& GetModule() const { return m_module; }

		protected:
			void DoOnInit() override;
			
		private:
			enum class State : uint8_t
			{
				Normal,
				Pressed,
				Hovered
			};
			State m_status{ State::Normal };
			Module m_module;
		};
	}
	
	struct ArgCheckBox
	{
		CheckState State { CheckState::None };
	};
	
	namespace Internal::CheckBox
	{
		struct Events : public ControlEvents
		{
			Event<ArgCheckBox> CheckedChanged;
		};
	}

	class CheckBox : public Control<Category::ControlTag, Internal::CheckBox::Reactor, Internal::CheckBox::Events>
	{
	public:
		CheckBox() = default;
		CheckBox(Window* parent, const Rectangle& rectangle);
		CheckBox(Window* parent, const Rectangle& rectangle, const std::wstring& text);
		CheckBox(Window* parent, const Rectangle& rectangle, const std::string& text);

		bool IsChecked() const;
		void SetChecked(bool isChecked);
		
		CheckState GetState() const;
		void SetState(CheckState state);
	};
}

#endif