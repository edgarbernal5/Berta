/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Slider.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	constexpr int SLIDER_TIMER_INITIAL_DELAY = 400;
	constexpr int SLIDER_TIMER_REPEAT_DELAY = 50;
	constexpr uint32_t MIN_SCROLLBOX_SIZE = 6u;

	void SliderReactor::Init(ControlBase& control)
	{
		m_control = &control;

		m_timer.SetOwner(control.Handle());
		m_timer.Connect([this](const ArgTimer& args)
		{
			DoScrollStep(true);
			m_timer.SetInterval(SLIDER_TIMER_REPEAT_DELAY);
		});
	}

	void SliderReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->Background, true);

		auto trackRect = GetSliderTrackRect();
		graphics.DrawRoundRectBox(trackRect, 2, window->Appearance->ButtonBackground, window->Appearance->BoxBorderColor, true);
		if (isScrollable())
		{
			auto sliderBoxRect = GetSliderBoxRect();
			auto trackThickness = window->ToScale(m_trackThickness);
			
			Point circlePosition{ (sliderBoxRect.X + sliderBoxRect.X + (int)sliderBoxRect.Width) / 2,  (sliderBoxRect.Y + sliderBoxRect.Y + (int)sliderBoxRect.Height) / 2 };
			auto fillColor = m_hoverArea == InteractionArea::Scrollbox ? window->Appearance->ButtonHighlightBackground : window->Appearance->ButtonBackground;
			graphics.DrawCircle(circlePosition, (int)sliderBoxRect.Width / 2, fillColor, window->Appearance->BoxBorderColor, true);
			//graphics.DrawEllipse(sliderBoxRect, fillColor, window->Appearance->BoxBorderColor, true);
		}
	}

	void SliderReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		if (m_pressedArea != InteractionArea::None)
		{
			return;
		}

		auto window = m_control->Handle();
		m_hoverArea = InteractionArea::None;
		Update(graphics);
		GUI::MarkAsUpdated(window);
	}

	void SliderReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (!args.ButtonState.LeftButton || !isScrollable())
		{
			return;
		}

		m_mouseDownPosition = args.Position;
		m_prevTrackValue = m_value;
		GUI::Capture(*m_control);

		m_pressedArea = m_hoverArea;

		auto sliderBoxRect = GetSliderBoxRect();
		if (m_pressedArea == InteractionArea::Scrollbox)
		{
			m_dragOffset = m_isVertical ? (m_mouseDownPosition.Y - sliderBoxRect.Y) : (m_mouseDownPosition.X - sliderBoxRect.X);
		}

		if (m_pressedArea == InteractionArea::ScrollTrack)
		{
			m_localStep = m_pageStep;
			m_trackPageUp = m_isVertical ? m_mouseDownPosition.Y < sliderBoxRect.Y : m_mouseDownPosition.X < sliderBoxRect.X;
		}
		else
		{
			m_localStep = m_step;
		}

		if (m_pressedArea != InteractionArea::Scrollbox)
		{
			DoScrollStep();
			m_timer.SetInterval(SLIDER_TIMER_INITIAL_DELAY);
			m_timer.Start();
		}
	}

	void SliderReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto window = m_control->Handle();

		if (args.ButtonState.NoButtonsPressed())
		{
			InteractionArea newHoverArea = DetermineHoverArea(args.Position);
			if (m_hoverArea != newHoverArea)
			{
				m_hoverArea = newHoverArea;
				Update(graphics);
				GUI::MarkAsUpdated(window);
			}
		}
		else if (args.ButtonState.LeftButton && m_pressedArea == InteractionArea::Scrollbox)
		{
			UpdateSliderBoxValue(m_isVertical ? args.Position.Y : args.Position.X);
		}
	}

	void SliderReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (!isScrollable())
		{
			return;
		}

		auto window = m_control->Handle();
		if (args.ButtonState.LeftButton)
		{
			GUI::ReleaseCapture(window);
		}

		m_timer.Stop();
		m_pressedArea = InteractionArea::None;
		if (!window->Size.IsInside(args.Position))
		{
			m_hoverArea = InteractionArea::None;
		}
		else
		{
			m_hoverArea = DetermineHoverArea(args.Position);
		}

		Update(graphics);
		GUI::MarkAsUpdated(window);
	}

	void SliderReactor::SetOrientation(bool isVertical)
	{
		m_isVertical = isVertical;
	}

	void SliderReactor::SetMinMax(int min, int max)
	{
		m_min = (std::min)(min, max);
		m_max = (std::max)(min, max);

		m_value = std::clamp(m_value, m_min, m_max);
	}

	void SliderReactor::SetValue(int value)
	{
		m_value = std::clamp(value, m_min, m_max);
	}

	void SliderReactor::SetStepValue(int value)
	{
		if (m_max < 1)
		{
			return;
		}
		m_step = std::clamp(value, 1, m_max);
	}

	void SliderReactor::SetPageStepValue(int value)
	{
		if (m_max < 1)
		{
			return;
		}
		m_pageStep = value;
	}

	void SliderReactor::DoScrollStep(bool fromTimer)
	{
		auto oldValue = m_value;
		auto window = m_control->Handle();
		
		if (m_pressedArea == InteractionArea::ScrollTrack)
		{
			auto mousePosition = GUI::GetMousePositionToWindow(window);
			auto scrollBoxRect = GetSliderBoxRect();
			if (m_isVertical)
			{
				if (m_trackPageUp && mousePosition.Y <= scrollBoxRect.Y)
				{
					SetValue(m_value - m_localStep);
				}
				else if (!m_trackPageUp && mousePosition.Y >= scrollBoxRect.Y + (int)scrollBoxRect.Height)
				{
					SetValue(m_value + m_localStep);
				}
			}
			else
			{
				if (m_trackPageUp && mousePosition.X <= scrollBoxRect.X)
				{
					SetValue(m_value - m_localStep);
				}
				else if (!m_trackPageUp && mousePosition.X >= scrollBoxRect.X + (int)scrollBoxRect.Width)
				{
					SetValue(m_value + m_localStep);
				}
			}
		}

		if (oldValue != m_value)
		{
			EmitValueChanged();

			Update(window->Renderer.GetGraphics());
			if (fromTimer)
			{
				GUI::RefreshWindow(window);
			}
			else
			{
				GUI::MarkAsUpdated(window);
			}
			
		}
	}

	void SliderReactor::EmitValueChanged()
	{
		ArgSlider argScrollbar{};
		argScrollbar.Value = m_value;
		reinterpret_cast<SliderEvents*>(m_control->Handle()->Events.get())->ValueChanged.Emit(argScrollbar);
	}

	void SliderReactor::UpdateSliderBoxValue(int position)
	{
		auto window = m_control->Handle();
		auto trackThickness = window->ToScale(m_trackThickness);
		auto radius = trackThickness * 2u;

		Rectangle sliderTrackRect = GetSliderTrackRect();
		Rectangle sliderBoxRect = GetSliderBoxRect();

		int newValue = m_value;
		int newBoxPosition = position - m_dragOffset;

		newValue = m_min + static_cast<int>(static_cast<float>(newBoxPosition * (m_max - m_min)) / (m_isVertical ? (sliderTrackRect.Height - radius) : (sliderTrackRect.Width - radius)));
		newValue = std::clamp(newValue, m_min, m_max);

		if (m_value != newValue)
		{
			m_value = newValue;
			EmitValueChanged();

			Update(window->Renderer.GetGraphics());
			GUI::MarkAsUpdated(window);
		}
	}

	SliderReactor::InteractionArea SliderReactor::DetermineHoverArea(const Point& position) const
	{
		if (isScrollable())
		{
			auto sliderBoxRect = GetSliderBoxRect();

			if (sliderBoxRect.IsInside(position))
			{
				return InteractionArea::Scrollbox;
			}

			auto trackRect = GetSliderTrackRect();
			if (trackRect.IsInside(position))
			{
				return InteractionArea::ScrollTrack;
			}
		}
		return InteractionArea::None;
	}

	Rectangle SliderReactor::GetSliderBoxRect() const
	{
		auto window = m_control->Handle();
		auto one = window->ToScale(1);
		auto two = window->ToScale(2);
		float currentValue = static_cast<float>(m_value - m_min) / static_cast<float>(m_max - m_min);
		
		auto trackRect = GetSliderTrackRect();
		auto trackThickness = window->ToScale(m_trackThickness);
		auto radius = trackThickness * 2u;
		auto diameter = radius * 2u;
		if (m_isVertical)
		{
			trackRect.Y += (int)(radius);
			trackRect.Height -= diameter;

			return {
				trackRect.X - (int)(radius - trackThickness) - ((int)trackRect.Width >> 1), static_cast<int>(currentValue * trackRect.Height), diameter, diameter
			};
		}
		trackRect.X += (int)(radius);
		trackRect.Width -= diameter;

		return {
			static_cast<int>(currentValue * trackRect.Width), trackRect.Y - (int)(radius - trackThickness) - ((int)trackRect.Height >> 1), diameter, diameter
		};
	}

	Rectangle SliderReactor::GetSliderTrackRect() const
	{
		auto window = m_control->Handle();
		auto trackThickness = window->ToScale(m_trackThickness);
		if (m_isVertical)
		{
			auto margin = (window->Size.Width - trackThickness) >> 1;
			return Rectangle{ (int)margin, 0, trackThickness,window->Size.Height };
		}
		auto margin = (window->Size.Height - trackThickness) >> 1;
		return Rectangle{ 0, (int)margin, window->Size.Width, trackThickness };
	}

	Slider::Slider(Window* parent, const Rectangle& rectangle, bool isVertical)
	{
		Create(parent, true, rectangle);
		m_reactor.SetOrientation(isVertical);

#if BT_DEBUG
		m_handle->Name = "Slider";
#endif
	}

	Slider::Slider(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, bool isVertical)
	{
		Create(parent, isUnscaleRect, rectangle);
		m_reactor.SetOrientation(isVertical);

#if BT_DEBUG
		m_handle->Name = "Slider";
#endif
	}

	void Slider::SetMinMax(int min, int max)
	{
		m_reactor.SetMinMax(min, max);
	}

	void Slider::SetValue(int value)
	{
		m_reactor.SetValue(value);
	}

	void Slider::SetStepValue(int stepValue)
	{
		m_reactor.SetStepValue(stepValue);
	}

	void Slider::SetPageStepValue(int pageStepValue)
	{
		m_reactor.SetPageStepValue(pageStepValue);
	}
}
