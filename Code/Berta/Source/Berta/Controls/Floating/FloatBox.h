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
#include "Berta/Controls/ScrollBar.h"

namespace Berta
{
	class FloatBox;
	struct ComboboxAppearance;

	class FloatBoxReactor : public ControlReactor
	{
	public:
		~FloatBoxReactor();

		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;

		void SetState(Float::InteractionData& selection);

		struct State
		{
			int m_hoveredIndex{ -1 };
			int m_selectedIndex{ -1 };
			int m_offset;
		};

		State& GetState() {	return m_state; }


		bool MoveSelectedItem(int direction);
	private:
		bool IsInside(const Point& point);
		void UpdateScrollBar();

		FloatBox* m_floatBox{ nullptr };
		ComboboxAppearance* m_comboBoxAppearance{ nullptr };

		Float::InteractionData* m_interactionData{ nullptr };
		State m_state;

		bool m_ignoreFirstMouseUp{ true };
		std::unique_ptr<ScrollBar> m_scrollBar;
	};

	class FloatBox : public Control<FloatBoxReactor, FormEvents>
	{
	public:
		FloatBox(Window* parent, const Rectangle& rectangle);
		~FloatBox();

		bool OnKeyPressed(const ArgKeyboard& args);
		void Init(Float::InteractionData& state)
		{
			m_reactor.SetState(state);
		}

		bool MoveSelectedItem(int direction);
		FloatBoxReactor::State& GetState() { return m_reactor.GetState(); }

	private:
	};
}

#endif