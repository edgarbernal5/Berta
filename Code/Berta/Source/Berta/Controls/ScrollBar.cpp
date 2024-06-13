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
				BT_CORE_DEBUG << "timeer... " << (int)m_pressedArea << ". oldValue " << oldValue << std::endl;
				if (m_pressedArea == HoverArea::Button1)
				{
					SetValue(m_value - m_step);
				}
				else if (m_pressedArea == HoverArea::Button2)
				{
					SetValue(m_value + m_step);
				}
				if (oldValue != m_value)
				{
					ArgScrollBar argScrollbar;
					argScrollbar.Value = m_value;
					reinterpret_cast<ScrollBarEvents*>(m_control->Handle()->Events.get())->ValueChanged.Emit(argScrollbar);

					Update(m_control->Handle()->Renderer.GetGraphics());
					GUI::UpdateDeferred(*m_control);
				}
			});
	}

	void ScrollBarReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		auto buttonSize = static_cast<uint32_t>(window->Appereance->ScrollBarSize * window->DPIScaleFactor);
		bool isScrollable = m_min != m_max;
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->ScrollBarBackground, true);
		if (m_isVertical)
		{
			int arrowWidth = static_cast<int>(6 * window->DPIScaleFactor);
			int arrowLength = static_cast<int>(3 * window->DPIScaleFactor);

			//upward button
			if (m_hoverArea == HoverArea::Button1)
			{
				graphics.DrawRectangle({ 0, 0, window->Size.Width, buttonSize }, window->Appereance->ButtonHighlightBackground, true);
			}
			graphics.DrawRectangle({ 0, 0, window->Size.Width, buttonSize }, window->Appereance->BoxBorderColor, false);
			graphics.DrawArrow({ 0, 0, window->Size.Width, buttonSize }, arrowLength, arrowWidth, window->Appereance->BoxBorderColor, Graphics::ArrowDirection::Upwards, true);

			if (isScrollable)
			{
				float num = 1.0f / ((m_max - m_min) + 1.0f);
				Rectangle remainRect{ 0, (int)buttonSize + 1, window->Size.Width, window->Size.Height - buttonSize * 2 - 2 };
				uint32_t scrollButtonSize = (uint32_t)remainRect.Height * num;
				Rectangle buttonRect{ 0, (int)buttonSize + 1 + (int)((m_value - m_min) * scrollButtonSize), window->Size.Width, scrollButtonSize };

				graphics.DrawRectangle(buttonRect, window->Appereance->Background, true);
				graphics.DrawRectangle(buttonRect, window->Appereance->BoxBorderColor, false);
			}

			//downward button
			if (m_hoverArea == HoverArea::Button2)
			{
				graphics.DrawRectangle({ 0, (int)(window->Size.Height - buttonSize), window->Size.Width, buttonSize }, window->Appereance->ButtonHighlightBackground, true);
			}
			graphics.DrawRectangle({ 0, (int)(window->Size.Height - buttonSize), window->Size.Width, buttonSize }, window->Appereance->BoxBorderColor, false);
			graphics.DrawArrow({ 0, (int)(window->Size.Height - buttonSize), window->Size.Width, buttonSize }, arrowLength, arrowWidth, window->Appereance->BoxBorderColor, Graphics::ArrowDirection::Downwards, true);
		}
	}

	void ScrollBarReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		if (m_pressedArea != HoverArea::None)
			return;

		auto window = m_control->Handle();
		m_hoverArea = HoverArea::None;
		Update(graphics);
		GUI::UpdateDeferred(window);
	}

	void ScrollBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (args.ButtonState.LeftButton)
		{

			m_pressedArea = m_hoverArea;
			m_timer.SetInterval(100);
			m_timer.Start();
		}
	}

	void ScrollBarReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		if (!args.ButtonState.LeftButton && !args.ButtonState.RightButton && !args.ButtonState.MiddleButton)
		{
			auto window = m_control->Handle();
			auto buttonSize = static_cast<uint32_t>(window->Appereance->ScrollBarSize * window->DPIScaleFactor);
			HoverArea newHoverArea = HoverArea::None;
			bool isScrollable = m_min != m_max;
			if (isScrollable)
			{
				if (m_isVertical)
				{
					if (Rectangle{ 0, 0, buttonSize, buttonSize }.IsInside(args.Position))
					{
						newHoverArea = HoverArea::Button1;
					}
					else if (Rectangle{ 0, (int)(window->Size.Height - buttonSize), buttonSize, buttonSize }.IsInside(args.Position))
					{
						newHoverArea = HoverArea::Button2;
					}
					else
					{
						newHoverArea = HoverArea::Center;
					}
				}
			}
			
			if (m_hoverArea != newHoverArea)
			{
				m_hoverArea = newHoverArea;
				Update(graphics);
				GUI::UpdateDeferred(window);
			}
		}
	}

	void ScrollBarReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		m_timer.Stop();

		if (m_pressedArea!= HoverArea::None)
		{
		}
		m_pressedArea = HoverArea::None;
		m_hoverArea= HoverArea::None;
		auto window = m_control->Handle();
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

		if (m_value < m_min)
			m_value = m_min;
		else if (m_value > m_max)
			m_value = m_max;
	}

	void ScrollBarReactor::SetValue(int value)
	{
		if (value >= m_min && value <= m_max)
			m_value = value;
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
}