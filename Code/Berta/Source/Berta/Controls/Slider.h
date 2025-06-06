/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SLIDER_BAR_HEADER
#define BT_SLIDER_BAR_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Core/Timer.h"
#include "Berta/Core/BasicTypes.h"

#include <string>

namespace Berta
{
	struct ArgSlider
	{
		int Value;
	};

	struct SliderEvents : public ControlEvents
	{
		Event<ArgSlider>	ValueChanged;
	};

	class SliderReactor : public ControlReactor
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
		void SetStepValue(int value);
		void SetPageStepValue(int value);

		int GetMin() const { return m_min; }
		int GetMax() const { return m_max; }
		int GetValue() const { return m_value; }
		int GetStepValue() const { return m_step; }
		int GetPageStepValue() const { return m_pageStep; }

	private:
		enum class InteractionArea
		{
			None,
			Scrollbox,
			ScrollTrack
		};

		void DoScrollStep(bool fromTimer = false);
		void EmitValueChanged();
		inline bool isScrollable() const { return m_min != m_max; }

		void UpdateSliderBoxValue(int position);
		InteractionArea DetermineHoverArea(const Point& position) const;
		
		Rectangle GetSliderBoxRect() const;
		Rectangle GetSliderCircleRect() const;
		Rectangle GetSliderTrackRect() const;

		bool m_isVertical{ false };
		int m_min{ 0 };
		int m_max{ 1 };
		int m_value{ 0 };
		int m_step{ 1 };
		int m_localStep{ 1 };
		int m_pageStep{ 2 };
		int m_dragOffset{ 0 };

		uint32_t m_trackThickness{ 3u };
		Timer m_timer;

		InteractionArea m_hoverArea{ InteractionArea::None };
		InteractionArea m_pressedArea{ InteractionArea::None };
		Point m_mouseDownPosition{};
		int m_prevTrackValue{};
		bool m_trackPageUp{ false };
	};

	class Slider : public Control<SliderReactor, SliderEvents>
	{
	public:
		Slider() = default;
		Slider(Window* parent, const Rectangle& rectangle = {}, bool isVertical = true);
		Slider(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, bool isVertical = true);

		void SetMinMax(int min, int max);
		void SetOrientation(bool isVertical) { m_reactor.SetOrientation(isVertical); }
		void SetValue(int value);
		void SetStepValue(int stepValue);
		void SetPageStepValue(int pageStepValue);

		int GetMin() const { return m_reactor.GetMin(); }
		int GetMax() const { return m_reactor.GetMax(); }
		int GetValue() const { return m_reactor.GetValue(); }
		int GetStepValue() const { return m_reactor.GetStepValue(); }
		int GetPageStepValue() const { return m_reactor.GetPageStepValue(); }
	};
}

#endif
