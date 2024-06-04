/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FLOAT_BOX_HEADER
#define BT_FLOAT_BOX_HEADER

#include <string>
#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/Floating/InteractionData.h"

namespace Berta
{
	class FloatBox;

	class FloatBoxReactor : public ControlReactor
	{
	public:
		~FloatBoxReactor();

		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;

		void SetState(GUI::InteractionData& selection)
		{
			m_selectionState = &selection;
			m_state.m_index = selection.m_selectedIndex;
			selection.m_isSelected = false;
		}

		struct State {
			int m_index;
		};

		State& GetState() {	return m_state; }

	private:
		FloatBox* m_control{ nullptr };

		GUI::InteractionData* m_selectionState{ nullptr };
		State m_state;

		bool m_ignoreFirstMouseUp{ true };
	};

	class FloatBox : public Control<FloatBoxReactor, RootEvents>
	{
	public:
		FloatBox(Window* parent, const Rectangle& rectangle);
		~FloatBox();

		void Init(GUI::InteractionData& state)
		{
			m_reactor.SetState(state);
		}

		FloatBoxReactor::State& GetState() { return m_reactor.GetState(); }

	private:
	};
}

#endif