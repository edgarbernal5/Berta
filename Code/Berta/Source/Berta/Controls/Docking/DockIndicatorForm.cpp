/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "DockIndicatorForm.h"

namespace Berta
{
	void DockIndicatorFormReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		auto indicatorSize = static_cast<int>(window->ClientSize.Width);
		auto indicatorSizeHalf = indicatorSize >> 1;

		if (m_dockPosition == DockPosition::Tab)
		{
			int four = window->ToScale(4);
			int eight = window->ToScale(8);
			uint32_t six = window->ToScale(6u);
			graphics.DrawRectangle(window->Appearance->Background, true);
			graphics.DrawRectangle({ four,four,(uint32_t)(indicatorSize - eight), six }, window->Appearance->MenuBackground, true);
			graphics.DrawRectangle({ four,four,(uint32_t)(indicatorSize - eight), (uint32_t)(indicatorSize - eight) }, window->Appearance->BoxBorderColor, false);

			graphics.DrawRectangle(window->Appearance->BoxBorderColor, false);
		}
		else if (m_dockPosition == DockPosition::Up)
		{
			int two = window->ToScale(2);
			int four = window->ToScale(4);
			int eight = window->ToScale(8);
			int six = window->ToScale(6);
			graphics.DrawRectangle(window->Appearance->Background, true);
			graphics.DrawRectangle({ four,four,(uint32_t)(indicatorSize - eight), (uint32_t)six }, window->Appearance->MenuBackground, true);
			graphics.DrawRectangle({ four,four,(uint32_t)(indicatorSize - eight), (uint32_t)(indicatorSizeHalf) }, window->Appearance->BoxBorderColor, false);

			int arrowWidth = four;
			int arrowLength = two;
			Rectangle arrowRect{ 0, indicatorSizeHalf,(uint32_t)(indicatorSize), (uint32_t)(indicatorSizeHalf + two) };

			graphics.DrawArrow(arrowRect, arrowLength, arrowWidth,
				Graphics::ArrowDirection::Upwards,
				window->Appearance->Foreground2nd, true, window->Appearance->ButtonPressedBackground);

			graphics.DrawRectangle(window->Appearance->BoxBorderColor, false);
		}
		else if (m_dockPosition == DockPosition::Down)
		{
			int two = window->ToScale(2);
			int four = window->ToScale(4);
			int eight = window->ToScale(8);
			int six = window->ToScale(6);
			graphics.DrawRectangle(window->Appearance->Background, true);
			graphics.DrawRectangle({ four,indicatorSizeHalf - four,(uint32_t)(indicatorSize - eight), (uint32_t)six }, window->Appearance->MenuBackground, true);
			graphics.DrawRectangle({ four,indicatorSizeHalf - four,(uint32_t)(indicatorSize - eight), (uint32_t)(indicatorSizeHalf) }, window->Appearance->BoxBorderColor, false);

			int arrowWidth = four;
			int arrowLength = two;
			Rectangle arrowRect{ 0, 0,(uint32_t)(indicatorSize), (uint32_t)(indicatorSizeHalf - two) };

			graphics.DrawArrow(arrowRect, arrowLength, arrowWidth,
				Graphics::ArrowDirection::Downwards,
				window->Appearance->Foreground2nd, true, window->Appearance->ButtonPressedBackground);

			graphics.DrawRectangle(window->Appearance->BoxBorderColor, false);
		}
		else if (m_dockPosition == DockPosition::Left)
		{
			int two = window->ToScale(2);
			int four = window->ToScale(4);
			int eight = window->ToScale(8);
			int six = window->ToScale(6);
			graphics.DrawRectangle(window->Appearance->Background, true);
			graphics.DrawRectangle({ four,four,(uint32_t)six, (uint32_t)(indicatorSize - eight) }, window->Appearance->MenuBackground, true);
			graphics.DrawRectangle({ four,four, (uint32_t)(indicatorSizeHalf), (uint32_t)(indicatorSize - eight) }, window->Appearance->BoxBorderColor, false);

			int arrowWidth = four;
			int arrowLength = two;
			Rectangle arrowRect{ indicatorSizeHalf + two, 0,(uint32_t)(indicatorSizeHalf - two), (uint32_t)(indicatorSize) };

			graphics.DrawArrow(arrowRect, arrowLength, arrowWidth,
				Graphics::ArrowDirection::Left,
				window->Appearance->Foreground2nd, true, window->Appearance->ButtonPressedBackground);

			graphics.DrawRectangle(window->Appearance->BoxBorderColor, false);
		}
		else if (m_dockPosition == DockPosition::Right)
		{
			int two = window->ToScale(2);
			int four = window->ToScale(4);
			int eight = window->ToScale(8);
			int six = window->ToScale(6);
			graphics.DrawRectangle(window->Appearance->Background, true);
			graphics.DrawRectangle({ indicatorSize - four - six,four,(uint32_t)six, (uint32_t)(indicatorSize - eight) }, window->Appearance->MenuBackground, true);
			graphics.DrawRectangle({ indicatorSizeHalf - four,four, (uint32_t)(indicatorSizeHalf), (uint32_t)(indicatorSize - eight) }, window->Appearance->BoxBorderColor, false);

			int arrowWidth = four;
			int arrowLength = two;
			Rectangle arrowRect{ 0, 0,(uint32_t)(indicatorSizeHalf - two), (uint32_t)(indicatorSize) };

			graphics.DrawArrow(arrowRect, arrowLength, arrowWidth,
				Graphics::ArrowDirection::Right,
				window->Appearance->Foreground2nd, true, window->Appearance->ButtonPressedBackground);

			graphics.DrawRectangle(window->Appearance->BoxBorderColor, false);
		}
	}

	void DockIndicatorFormReactor::SetDockPosition(DockPosition position)
	{
		m_dockPosition = position;
	}

	DockIndicatorForm::DockIndicatorForm(Window* owner, const Size& size, const FormStyle& windowStyle)
	{
		Create(owner, false, GUI::GetCenteredOnScreen(size), windowStyle, false, false);

#if BT_DEBUG
		m_handle->Name = "DockIndicatorForm";
#endif
	}

	DockIndicatorForm::DockIndicatorForm(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle)
	{
		Create(owner, false, rectangle, windowStyle, false, false);

#if BT_DEBUG
		m_handle->Name = "DockIndicatorForm";
#endif
	}

	void DockIndicatorForm::SetDockPosition(DockPosition position)
	{
		m_reactor.SetDockPosition(position);
	}
}