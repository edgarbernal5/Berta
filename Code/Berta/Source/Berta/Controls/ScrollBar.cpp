/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ScrollBar.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	constexpr int SCROLL_TIMER_INITIAL_DELAY = 400;
	constexpr int SCROLL_TIMER_REPEAT_DELAY = 50;
	constexpr uint32_t MIN_SCROLLBOX_SIZE = 6u;
	constexpr int ARROW_WIDTH = 6;
	constexpr int ARROW_LENGTH = 3;

	void ScrollBarReactor::Init(ControlBase& control)
	{
		m_control = &control;

		m_timer.SetOwner(control.Handle());
		m_timer.Connect([this](const ArgTimer& args)
		{
			DoScrollStep(true);
			m_timer.SetInterval(SCROLL_TIMER_REPEAT_DELAY);
		});

		GUI::MakeWindowActive(control, false, control.GetParent());
	}

	void ScrollBarReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		auto buttonSize = GetButtonSize();
		bool enabled = m_control->GetEnabled();
		graphics.DrawRectangle(window->ClientSize.ToRectangle(), window->Appearance->ScrollBarBackground, true);

		if (!IsValid())
		{
			return;
		}

		int arrowWidth = window->ToScale(4);
		int arrowLength = window->ToScale(2);

		Rectangle button1Rect = m_isVertical ?
			Rectangle{ 0, 0, window->ClientSize.Width, buttonSize } :
			Rectangle{ 0, 0, buttonSize, window->ClientSize.Height };
		DrawButton(graphics, button1Rect, arrowLength, arrowWidth, m_isVertical ? Graphics::ArrowDirection::Upwards : Graphics::ArrowDirection::Left, m_hoverArea == InteractionArea::Button1, enabled);

		if (isScrollable())
		{
			auto scrollBoxRect = GetScrollBoxRect();
			
			graphics.DrawRoundRectBox(scrollBoxRect, 2, m_hoverArea == InteractionArea::Scrollbox ? (window->Appearance->ButtonHighlightBackground) : window->Appearance->ButtonBackground, window->Appearance->BoxBorderColor, true);
		}

		Rectangle button2Rect = m_isVertical ?
			Rectangle{ 0, (int)(window->ClientSize.Height - buttonSize), window->ClientSize.Width, buttonSize } :
			Rectangle{ (int)(window->ClientSize.Width - buttonSize), 0, buttonSize, window->ClientSize.Height };
		DrawButton(graphics, button2Rect, arrowLength, arrowWidth, m_isVertical ? Graphics::ArrowDirection::Downwards : Graphics::ArrowDirection::Right, m_hoverArea == InteractionArea::Button2, enabled);
	}

	void ScrollBarReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		if (m_pressedArea != InteractionArea::None)
		{
			return;
		}

		m_hoverArea = InteractionArea::None;
		auto window = m_control->Handle();
		GUI::MarkAsNeedUpdate(window);
	}

	void ScrollBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
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

	void ScrollBarReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto window = m_control->Handle();
		auto buttonSize = GetButtonSize();

		if (args.ButtonState.NoButtonsPressed())
		{
			InteractionArea newHoverArea = DetermineHoverArea(args.Position);
			if (m_hoverArea != newHoverArea)
			{
				m_hoverArea = newHoverArea;
				GUI::MarkAsNeedUpdate(window);
			}
		}
		else if (args.ButtonState.LeftButton && m_pressedArea == InteractionArea::Scrollbox)
		{
			UpdateScrollBoxValue(m_isVertical ? args.Position.Y : args.Position.X, buttonSize);
		}
	}

	void ScrollBarReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
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
		if (!window->ClientSize.IsInside(args.Position))
		{
			m_hoverArea = InteractionArea::None;
		}
		else
		{
			m_hoverArea = DetermineHoverArea(args.Position);
		}

		GUI::MarkAsNeedUpdate(window);
	}

	void ScrollBarReactor::SetOrientation(bool isVertical)
	{
		m_isVertical = isVertical;
	}

	void ScrollBarReactor::SetMinMax(ScrollBarUnit min, ScrollBarUnit max)
	{
		m_min = (std::min)(min, max);
		m_max = (std::max)(min, max);
		//auto oldValue = m_value;
		m_value = std::clamp(m_value, m_min, m_max);

		//if (oldValue != m_value)
		//{
		//	EmitValueChanged();
		//}
		GUI::UpdateWindow(m_control->Handle());
	}

	void ScrollBarReactor::SetValue(ScrollBarUnit value)
	{
		m_value = std::clamp(value, m_min, m_max);
		//emitir evento.
		GUI::UpdateWindow(m_control->Handle());
	}

	void ScrollBarReactor::SetStepValue(ScrollBarUnit value)
	{
		if (m_max < 1)
		{
			return;
		}
		m_step = std::clamp(value, 1, m_max);
		GUI::UpdateWindow(m_control->Handle());
	}

	void ScrollBarReactor::SetPageStepValue(ScrollBarUnit value)
	{
		if (m_max < 1)
		{
			return;
		}
		m_pageStep = value;
	}

	bool ScrollBarReactor::IsValid() const
	{
		auto window = m_control->Handle();
		auto buttonSize = GetButtonSize();

		if (m_isVertical && window->ClientSize.Height < buttonSize * 2u + 2u + 4u)
		{
			return false;
		}

		if (!m_isVertical && window->ClientSize.Width < buttonSize * 2u + 2u + 4u)
		{
			return false;
		}

		return true;
	}

	void ScrollBarReactor::DoScrollStep(bool fromTimer)
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

			GUI::UpdateWindow(window);
		}
	}

	void ScrollBarReactor::EmitValueChanged()
	{
		ArgScrollBar argScrollbar{};
		argScrollbar.Value = m_value;
		reinterpret_cast<ScrollBarEvents*>(m_control->Handle()->Events.get())->ValueChanged.Emit(argScrollbar);
	}

	uint32_t ScrollBarReactor::GetButtonSize() const
	{
		return m_control->Handle()->ToScale(m_control->Handle()->Appearance->ScrollBarSize);
	}

	void ScrollBarReactor::DrawButton(Graphics& graphics, const Rectangle& rect, int arrowLength, int arrowWidth, Graphics::ArrowDirection direction, bool isHighlighted, bool isEnabled)
	{
		if (arrowWidth % 2 == 1)
			--arrowWidth;
		
		auto color = isEnabled ? (isHighlighted ? m_control->Handle()->Appearance->ButtonPressedBackground : m_control->Handle()->Appearance->ButtonBackground) : m_control->Handle()->Appearance->BoxBorderDisabledColor;
		graphics.DrawArrow(rect, arrowLength, arrowWidth, direction,
			color, true, color);
	}

	ScrollBarReactor::InteractionArea ScrollBarReactor::DetermineHoverArea(const Point& position) const
	{
		auto window = m_control->Handle();
		auto buttonSize = GetButtonSize();

		if (m_isVertical)
		{
			if (Rectangle{ 0, 0, window->ClientSize.Width, buttonSize }.IsInside(position))
			{
				return InteractionArea::Button1;
			}
			if (Rectangle{ 0, (int)(window->ClientSize.Height - buttonSize), window->ClientSize.Width, buttonSize }.IsInside(position))
			{
				return InteractionArea::Button2;
			}

			if (isScrollable())
			{
				auto scrollBoxRect = GetScrollBoxRect();

				if (scrollBoxRect.IsInside(position))
				{
					return InteractionArea::Scrollbox;
				}

				return InteractionArea::ScrollTrack;
			}
		}
		if (Rectangle{ 0, 0, buttonSize, window->ClientSize.Height }.IsInside(position))
		{
			return InteractionArea::Button1;
		}
		if (Rectangle{ (int)(window->ClientSize.Width - buttonSize), 0, buttonSize, window->ClientSize.Height }.IsInside(position))
		{
			return InteractionArea::Button2;
		}

		if (isScrollable())
		{
			auto scrollBoxRect = GetScrollBoxRect();

			if (scrollBoxRect.IsInside(position))
			{
				return InteractionArea::Scrollbox;
			}

			return InteractionArea::ScrollTrack;
		}
		return InteractionArea::None;
	}

	void ScrollBarReactor::UpdateScrollBoxValue(int position, int buttonSize)
	{
		auto window = m_control->Handle();
		auto one = window->ToScale(1);
		float scaleFactor = static_cast<float>(m_pageStep) / ((m_max - m_min) + m_pageStep);

		ScrollBarUnit newValue = m_value;
		Rectangle scrollTrackRect = m_isVertical ?
			Rectangle{ 0, buttonSize + one, window->ClientSize.Width, window->ClientSize.Height - 2u * buttonSize - 2u } :
			Rectangle{ buttonSize + one, 0, window->ClientSize.Width - 2u * buttonSize - 2u, window->ClientSize.Height };

		uint32_t scrollBoxSize = (std::max)(static_cast<uint32_t>((m_isVertical ? scrollTrackRect.Height : scrollTrackRect.Width) * scaleFactor), window->ToScale(6u));
		int newBoxPosition = position - m_dragOffset - (m_isVertical ? scrollTrackRect.Y : scrollTrackRect.X);
		newBoxPosition = std::clamp(newBoxPosition, 0, static_cast<int>((m_isVertical ? scrollTrackRect.Height : scrollTrackRect.Width) - scrollBoxSize));
		
		newValue = m_min + static_cast<ScrollBarUnit>(static_cast<float>(newBoxPosition * (m_max - m_min)) / (m_isVertical ? scrollTrackRect.Height - scrollBoxSize : scrollTrackRect.Width - scrollBoxSize));

		newValue = std::clamp(newValue, m_min, m_max);

		if (m_value != newValue)
		{
			m_value = newValue;
			EmitValueChanged();

			GUI::MarkAsNeedUpdate(window);
		}
	}

	Rectangle ScrollBarReactor::GetScrollBoxRect() const
	{
		auto window = m_control->Handle();
		auto buttonSize = GetButtonSize();
		auto one = window->ToScale(1);
		auto two = window->ToScale(2);
		float num = static_cast<float>(m_pageStep) / ((m_max - m_min) + m_pageStep);
		
		if (m_isVertical)
		{
			Rectangle scrollTrackRect{ 0, static_cast<int>(buttonSize) + one, window->ClientSize.Width, window->ClientSize.Height - 2u * buttonSize - one * 2u };
			uint32_t scrollBoxSize = (std::max)(static_cast<uint32_t>(scrollTrackRect.Height * num), window->ToScale(6u));
			
			return {
				two,
				static_cast<int>(buttonSize) + one + static_cast<int>((m_value - m_min) / static_cast<float>(m_max - m_min) * (scrollTrackRect.Height - scrollBoxSize)),
				window->ClientSize.Width - two * 2,
				scrollBoxSize
			};
		}
		Rectangle scrollTrackRect{ static_cast<int>(buttonSize) + one, 0, window->ClientSize.Width - 2u * buttonSize - 2u, window->ClientSize.Height };
		uint32_t scrollBoxSize = (std::max)(static_cast<uint32_t>(scrollTrackRect.Width * num), window->ToScale(6u));

		return {
			static_cast<int>(buttonSize) + one + static_cast<int>((m_value - m_min) / static_cast<float>(m_max - m_min) * (scrollTrackRect.Width - scrollBoxSize)),
			two,
			scrollBoxSize,
			window->ClientSize.Height - two * 2
		};
	}

	ScrollBar::ScrollBar(Window* parent, const Rectangle& rectangle, bool isVertical)
	{
		Create(parent, true, rectangle);
		m_reactor.SetOrientation(isVertical);

#if BT_DEBUG
		m_handle->Name = "ScrollBar";
#endif
	}

	ScrollBar::ScrollBar(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, bool isVertical)
	{
		Create(parent, isUnscaleRect, rectangle);
		m_reactor.SetOrientation(isVertical);

#if BT_DEBUG
		m_handle->Name = "ScrollBar";
#endif
	}

	void ScrollBar::SetMinMax(ScrollBarUnit min, ScrollBarUnit max)
	{
		m_reactor.SetMinMax(min, max);
	}

	void ScrollBar::SetValue(ScrollBarUnit value)
	{
		m_reactor.SetValue(value);
	}

	void ScrollBar::SetStepValue(ScrollBarUnit stepValue)
	{
		m_reactor.SetStepValue(stepValue);
	}

	void ScrollBar::SetPageStepValue(ScrollBarUnit pageStepValue)
	{
		m_reactor.SetPageStepValue(pageStepValue);
	}
}
