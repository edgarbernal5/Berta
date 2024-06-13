/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SCROLL_BAR_HEADER
#define BT_SCROLL_BAR_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Core/Timer.h"

#include <string>

namespace Berta
{
	class ScrollBarReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;

		void SetOrientation(bool isVertical);
		void SetMinMax(int min, int max);
		void SetValue(int value);

	private:
		inline bool isScrollable() const { return m_min != m_max; }
		uint32_t GetButtonSize() const;

		ControlBase* m_control{ nullptr };
		bool m_isVertical{ false };
		int m_min = 0;
		int m_max = 1;
		int m_step = 1;
		int m_localStep = 1;
		int m_value = 0;
		Timer m_timer;

		enum class InteractionArea
		{
			None,
			Button1,
			Button2,
			Scrollbox,
			ScrollTrack
		};

		InteractionArea m_hoverArea{ InteractionArea::None };
		InteractionArea m_pressedArea{ InteractionArea::None };
	};

	class ScrollBar : public Control<ScrollBarReactor, ScrollBarEvents>
	{
	public:
		ScrollBar(Window* parent, const Rectangle& rectangle, bool isVertical = true);

		void SetMinMax(int min, int max);
		void SetValue(int value);
	private:
	};
}

#endif