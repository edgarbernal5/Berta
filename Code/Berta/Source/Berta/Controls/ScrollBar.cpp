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
	void ScrollBarReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_timer.SetOwner(control.Handle());
		m_timer.Connect([this](const ArgTimer& args)
			{
				auto oldValue = m_value;
				auto window = m_control->Handle();
				BT_CORE_DEBUG << "timer... " << static_cast<int>(m_pressedArea) << ". oldValue " << oldValue << std::endl;
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
					if (m_isVertical)
					{
						auto mousePosition = GUI::GetMousePositionToWindow(window);
						if (m_mouseDownPosition.Y > mousePosition.Y)
						{

						}
					}
				}
				if (oldValue != m_value)
				{
					ArgScrollBar argScrollbar;
					argScrollbar.Value = m_value;
					reinterpret_cast<ScrollBarEvents*>(window->Events.get())->ValueChanged.Emit(argScrollbar);

					Update(window->Renderer.GetGraphics());
					GUI::UpdateDeferred(window);
				}
			});
	}

	void ScrollBarReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		auto buttonSize = GetButtonSize();
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->ScrollBarBackground, true);

		int arrowWidth = static_cast<int>(6 * window->DPIScaleFactor);
		int arrowLength = static_cast<int>(3 * window->DPIScaleFactor);

		DrawButton(graphics, { 0, 0, window->Size.Width, buttonSize }, arrowLength, arrowWidth, Graphics::ArrowDirection::Upwards, m_hoverArea == InteractionArea::Button1);

		if (isScrollable())
		{
			float num = 1.0f / ((m_max - m_min) + 1.0f);
			Rectangle scrollTrackRect{ 0, static_cast<int>(buttonSize) + 1, window->Size.Width, window->Size.Height - 2 * buttonSize - 2 };
			uint32_t scrollBoxSize = static_cast<uint32_t>(scrollTrackRect.Height * num);
			Rectangle scrollBoxRect{ 0, static_cast<int>(buttonSize) + 1 + static_cast<int>((m_value - m_min) / static_cast<float>(m_max - m_min) * (scrollTrackRect.Height - scrollBoxSize)), window->Size.Width, scrollBoxSize };

			graphics.DrawRectangle(scrollBoxRect, window->Appereance->Background, true);
			graphics.DrawRectangle(scrollBoxRect, window->Appereance->BoxBorderColor, false);
		}

		DrawButton(graphics, { 0, (int)(window->Size.Height - buttonSize), window->Size.Width, buttonSize }, arrowLength, arrowWidth, Graphics::ArrowDirection::Downwards, m_hoverArea == InteractionArea::Button2);
	}

	void ScrollBarReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		if (m_pressedArea != InteractionArea::None)
			return;

		auto window = m_control->Handle();
		m_hoverArea = InteractionArea::None;
		Update(graphics);
		GUI::UpdateDeferred(window);
	}

	void ScrollBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (!args.ButtonState.LeftButton || !isScrollable())
			return;

		m_mouseDownPosition = args.Position;
		m_prevTrackValue = m_value;
		GUI::Capture(*m_control);

		m_pressedArea = m_hoverArea;
		if (m_pressedArea != InteractionArea::Scrollbox)
		{
			m_timer.SetInterval(100);
			m_timer.Start();
		}
	}

	void ScrollBarReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto window = m_control->Handle();
		auto buttonSize = GetButtonSize();

		if (!args.ButtonState.LeftButton && !args.ButtonState.RightButton && !args.ButtonState.MiddleButton)
		{
			InteractionArea newHoverArea = DetermineHoverArea(args.Position);
			if (m_hoverArea != newHoverArea)
			{
				m_hoverArea = newHoverArea;
				Update(graphics);
				GUI::UpdateDeferred(window);
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
			return;

		auto window = m_control->Handle();
		if (args.ButtonState.LeftButton)
			GUI::ReleaseCapture(window);

		m_timer.Stop();
		m_pressedArea = InteractionArea::None;
		m_hoverArea = InteractionArea::None;

		Update(graphics);
		GUI::UpdateDeferred(window);
	}

	void ScrollBarReactor::SetOrientation(bool isVertical)
	{
		m_isVertical = isVertical;
	}

	void ScrollBarReactor::SetMinMax(int min, int max)
	{
		m_min = (std::min)(min, max);
		m_max = (std::max)(min, max);
		m_value = std::clamp(m_value, m_min, m_max);
	}

	void ScrollBarReactor::SetValue(int value)
	{
		m_value = std::clamp(value, m_min, m_max);
	}

	uint32_t ScrollBarReactor::GetButtonSize() const
	{
		return static_cast<uint32_t>(m_control->Handle()->Appereance->ScrollBarSize * m_control->Handle()->DPIScaleFactor);
	}

	ScrollBar::ScrollBar(Window* parent, const Rectangle& rectangle, bool isVertical)
	{
		Create(parent, rectangle);
		m_reactor.SetOrientation(isVertical);
	}

	void ScrollBar::SetMinMax(int min, int max)
	{
		m_reactor.SetMinMax(min, max);
	}

	void ScrollBar::SetValue(int value)
	{
		m_reactor.SetValue(value);
	}

	void ScrollBarReactor::DrawButton(Graphics& graphics, const Rectangle& rect, int arrowLength, int arrowWidth, Graphics::ArrowDirection direction, bool isHighlighted)
	{
		if (isHighlighted)
		{
			graphics.DrawRectangle(rect, m_control->Handle()->Appereance->ButtonHighlightBackground, true);
		}
		graphics.DrawRectangle(rect, m_control->Handle()->Appereance->BoxBorderColor, false);
		graphics.DrawArrow(rect, arrowLength, arrowWidth, m_control->Handle()->Appereance->BoxBorderColor, direction, true);
	}

	ScrollBarReactor::InteractionArea ScrollBarReactor::DetermineHoverArea(const Point& position) const
	{
		auto window = m_control->Handle();
		auto buttonSize = GetButtonSize();

		if (m_isVertical)
		{
			if (Rectangle{ 0, 0, window->Size.Width, buttonSize }.IsInside(position))
				return InteractionArea::Button1;
			if (Rectangle{ 0, (int)(window->Size.Height - buttonSize), window->Size.Width, buttonSize }.IsInside(position))
				return InteractionArea::Button2;

			if (isScrollable())
			{
				float num = 1.0f / ((m_max - m_min) + 1.0f);
				Rectangle scrollTrackRect{ 0, static_cast<int>(buttonSize) + 1, window->Size.Width, window->Size.Height - 2 * buttonSize - 2 };
				uint32_t scrollBoxSize = static_cast<uint32_t>(scrollTrackRect.Height * num);
				Rectangle scrollBoxRect{ 0, static_cast<int>(buttonSize) + 1 + static_cast<int>((m_value - m_min) / static_cast<float>(m_max - m_min) * (scrollTrackRect.Height - scrollBoxSize)), window->Size.Width, scrollBoxSize };

				if (scrollBoxRect.IsInside(position))
					return InteractionArea::Scrollbox;

				return InteractionArea::ScrollTrack;
			}
		}
		return InteractionArea::None;
	}

	void ScrollBarReactor::UpdateScrollBoxValue(int position, int buttonSize)
	{
		auto window = m_control->Handle();
		float num = 1.0f / ((m_max - m_min) + 1.0f);
		int newValue = m_value;
		if (m_isVertical)
		{
			Rectangle scrollTrackRect{ 0, buttonSize + 1, window->Size.Width,  window->Size.Height - 2 * buttonSize - 2 };
			uint32_t scrollBoxSize = static_cast<uint32_t>(scrollTrackRect.Height * num);

			newValue = static_cast<int>((static_cast<float>(position - scrollTrackRect.Y) / (scrollTrackRect.Height - scrollBoxSize)) * (m_max - m_min)) + m_min;
		}
		else
		{
			Rectangle scrollTrackRect{ buttonSize + 1, 0 , window->Size.Width - 2 * buttonSize - 2, window->Size.Height };
			uint32_t scrollBoxSize = static_cast<uint32_t>(scrollTrackRect.Width * num);

			newValue = static_cast<int>((static_cast<float>(position - scrollTrackRect.X) / (scrollTrackRect.Width - scrollBoxSize)) * (m_max - m_min)) + m_min;
		}
		
		newValue = std::clamp(newValue, m_min, m_max);

		if (m_value != newValue)
		{
			m_value = newValue;
			ArgScrollBar argScrollbar;
			argScrollbar.Value = m_value;
			reinterpret_cast<ScrollBarEvents*>(window->Events.get())->ValueChanged.Emit(argScrollbar);

			Update(window->Renderer.GetGraphics());
			GUI::UpdateDeferred(window);
		}
	}
}
