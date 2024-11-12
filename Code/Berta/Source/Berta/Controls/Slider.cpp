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
	constexpr int SCROLL_TIMER_INITIAL_DELAY = 400;
	constexpr int SCROLL_TIMER_REPEAT_DELAY = 50;
	constexpr uint32_t MIN_SCROLLBOX_SIZE = 6u;
	constexpr int ARROW_WIDTH = 6;
	constexpr int ARROW_LENGTH = 3;

	void SliderReactor::Init(ControlBase& control)
	{
		m_control = &control;

		m_timer.SetOwner(control.Handle());
		m_timer.Connect([this](const ArgTimer& args)
		{
			DoScrollStep(true);
			m_timer.SetInterval(SCROLL_TIMER_REPEAT_DELAY);
		});
	}

	void SliderReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		auto buttonSize = GetButtonSize();
		bool enabled = m_control->GetEnabled();
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->Background, true);

		/*if (!IsValid())
		{
			return;
		}*/

		int arrowWidth = window->ToScale(4);
		int arrowLength = window->ToScale(2);

		Rectangle button1Rect = m_isVertical ?
			Rectangle{ 0, 0, window->Size.Width, buttonSize } :
			Rectangle{ 0, 0, buttonSize, window->Size.Height };
		
		if (isScrollable())
		{
			auto scrollBoxRect = GetScrollBoxRect();
			
			//graphics.DrawRectangle(scrollBoxRect, m_hoverArea == InteractionArea::Scrollbox ? (window->Appearance->ButtonHighlightBackground) : window->Appearance->ButtonBackground, true);
			//graphics.DrawRectangle(scrollBoxRect, window->Appearance->BoxBorderColor, false);

			graphics.DrawRoundRectBox(scrollBoxRect, 2, m_hoverArea == InteractionArea::Scrollbox ? (window->Appearance->ButtonHighlightBackground) : window->Appearance->ButtonBackground, window->Appearance->BoxBorderColor, true);
		}

		Rectangle button2Rect = m_isVertical ?
			Rectangle{ 0, (int)(window->Size.Height - buttonSize), window->Size.Width, buttonSize } :
			Rectangle{ (int)(window->Size.Width - buttonSize), 0, buttonSize, window->Size.Height };
		
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

		auto scrollBoxRect = GetScrollBoxRect();
		if (m_pressedArea == InteractionArea::Scrollbox)
		{
			m_dragOffset = m_isVertical ? (m_mouseDownPosition.Y - scrollBoxRect.Y) : (m_mouseDownPosition.X - scrollBoxRect.X);
		}

		if (m_pressedArea == InteractionArea::ScrollTrack)
		{
			m_localStep = m_pageStep;
			m_trackPageUp = m_isVertical ? m_mouseDownPosition.Y < scrollBoxRect.Y : m_mouseDownPosition.X < scrollBoxRect.X;
		}
		else
		{
			m_localStep = m_step;
		}

		if (m_pressedArea != InteractionArea::Scrollbox)
		{
			DoScrollStep();
			m_timer.SetInterval(SCROLL_TIMER_INITIAL_DELAY);
			m_timer.Start();
		}
	}

	void SliderReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto window = m_control->Handle();
		auto buttonSize = GetButtonSize();

		if (args.ButtonState.NoButtonsPressed())
		{
			//InteractionArea newHoverArea = DetermineHoverArea(args.Position);
			//if (m_hoverArea != newHoverArea)
			{
				//m_hoverArea = newHoverArea;
				//Update(graphics);
				//GUI::MarkAsUpdated(window);
			}
		}
		else if (args.ButtonState.LeftButton && m_pressedArea == InteractionArea::Scrollbox)
		{
			UpdateScrollBoxValue(m_isVertical ? (args.Position.Y) : args.Position.X, buttonSize);
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
			//m_hoverArea = DetermineHoverArea(args.Position);
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
		//auto oldValue = m_value;
		m_value = std::clamp(m_value, m_min, m_max);

		//if (oldValue != m_value)
		//{
		//	EmitValueChanged();
		//}
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
		
		if (m_pressedArea == InteractionArea::Button1)
		{
			SetValue(m_value - m_localStep);
		}
		else if (m_pressedArea == InteractionArea::Button2)
		{
			SetValue(m_value + m_localStep);
		}
		else if (m_pressedArea == InteractionArea::ScrollTrack)
		{
			auto mousePosition = GUI::GetMousePositionToWindow(window);
			auto scrollBoxRect = GetScrollBoxRect();
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
		ArgSlider argScrollbar;
		argScrollbar.Value = m_value;
		reinterpret_cast<SliderEvents*>(m_control->Handle()->Events.get())->ValueChanged.Emit(argScrollbar);
	}

	uint32_t SliderReactor::GetButtonSize() const
	{
		return m_control->Handle()->ToScale(m_control->Handle()->Appearance->ScrollBarSize);
	}

	void SliderReactor::UpdateScrollBoxValue(int position, int buttonSize)
	{
		auto window = m_control->Handle();
		auto one = window->ToScale(1);
		float scaleFactor = static_cast<float>(m_pageStep) / ((m_max - m_min) + m_pageStep);

		int newValue = m_value;
		Rectangle scrollTrackRect = m_isVertical ?
			Rectangle{ 0, buttonSize + one, window->Size.Width, window->Size.Height - 2u * buttonSize - 2u } :
			Rectangle{ buttonSize + one, 0, window->Size.Width - 2u * buttonSize - 2u, window->Size.Height };

		uint32_t scrollBoxSize = (std::max)(static_cast<uint32_t>((m_isVertical ? scrollTrackRect.Height : scrollTrackRect.Width) * scaleFactor), window->ToScale(6u));
		int newBoxPosition = position - m_dragOffset - (m_isVertical ? scrollTrackRect.Y : scrollTrackRect.X);
		newBoxPosition = std::clamp(newBoxPosition, 0, static_cast<int>((m_isVertical ? scrollTrackRect.Height : scrollTrackRect.Width) - scrollBoxSize));
		
		newValue = m_min + static_cast<int>(static_cast<float>(newBoxPosition * (m_max - m_min)) / (m_isVertical ? scrollTrackRect.Height - scrollBoxSize : scrollTrackRect.Width - scrollBoxSize));

		newValue = std::clamp(newValue, m_min, m_max);

		if (m_value != newValue)
		{
			m_value = newValue;
			EmitValueChanged();

			Update(window->Renderer.GetGraphics());
			GUI::MarkAsUpdated(window);
		}
	}

	Rectangle SliderReactor::GetScrollBoxRect() const
	{
		auto window = m_control->Handle();
		auto buttonSize = GetButtonSize();
		auto one = window->ToScale(1);
		auto two = window->ToScale(2);
		float num = static_cast<float>(m_pageStep) / ((m_max - m_min) + m_pageStep);
		
		if (m_isVertical)
		{
			Rectangle scrollTrackRect{ 0, static_cast<int>(buttonSize) + one, window->Size.Width, window->Size.Height - 2u * buttonSize - one * 2u };
			uint32_t scrollBoxSize = (std::max)(static_cast<uint32_t>(scrollTrackRect.Height * num), window->ToScale(6u));
			
			return {
				two,
				static_cast<int>(buttonSize) + one + static_cast<int>((m_value - m_min) / static_cast<float>(m_max - m_min) * (scrollTrackRect.Height - scrollBoxSize)),
				window->Size.Width - two * 2,
				scrollBoxSize
			};
		}
		Rectangle scrollTrackRect{ static_cast<int>(buttonSize) + one, 0, window->Size.Width - 2u * buttonSize - 2u, window->Size.Height };
		uint32_t scrollBoxSize = (std::max)(static_cast<uint32_t>(scrollTrackRect.Width * num), window->ToScale(6u));

		return {
			static_cast<int>(buttonSize) + one + static_cast<int>((m_value - m_min) / static_cast<float>(m_max - m_min) * (scrollTrackRect.Width - scrollBoxSize)),
			two,
			scrollBoxSize,
			window->Size.Height - two * 2
		};
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
