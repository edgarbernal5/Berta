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
	}

	void ScrollBarReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		auto buttonSize = window->Appereance->ScrollBarSize;

		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->ScrollBarBackground, true);
		if (m_isVertical)
		{
			int arrowWidth = static_cast<int>(6 * window->DPIScaleFactor);
			int arrowLength = static_cast<int>(3 * window->DPIScaleFactor);

			graphics.DrawRectangle({ 0, 0, window->Size.Width, buttonSize }, window->Appereance->BoxBorderColor, false);
			graphics.DrawArrow({ 0, 0, window->Size.Width, buttonSize }, arrowLength, arrowWidth, window->Appereance->BoxBorderColor, Graphics::ArrowDirection::Upwards, true);

			Rectangle remainRect{ 0, (int)buttonSize + 1, window->Size.Width, window->Size.Height - buttonSize * 2 - 2 };

			graphics.DrawRectangle(remainRect, window->Appereance->Background, true);
			graphics.DrawRectangle(remainRect, window->Appereance->BoxBorderColor, false);

			graphics.DrawRectangle({ 0, (int)(window->Size.Height - buttonSize), window->Size.Width, buttonSize }, window->Appereance->BoxBorderColor, false);
			graphics.DrawArrow({ 0, (int)(window->Size.Height - buttonSize), window->Size.Width, buttonSize }, arrowLength, arrowWidth, window->Appereance->BoxBorderColor, Graphics::ArrowDirection::Downwards, true);
		}
	}

	void ScrollBarReactor::SetOrientation(bool isVertical)
	{
		m_isVertical = isVertical;
	}

	void ScrollBarReactor::SetMinMax(int min, int max)
	{
		m_min = min;
		m_max = max;

		if (m_value < min)
			m_value = min;

		if (m_value > max)
			m_value = max;
	}

	void ScrollBarReactor::SetValue(int value)
	{
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