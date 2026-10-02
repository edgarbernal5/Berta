/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ColorPicker.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	namespace Internal::ColorPicker
	{
		void Reactor::Update(Graphics& graphics)
		{
			auto window = m_control->Handle();
			bool enabled = m_control->GetEnabled();
			auto backgroundRect = window->ClientSize.ToRectangle();

			auto color = window->Appearance->Background;
			if (!enabled)
			{
				color = window->Appearance->ButtonDisabledBackground;
			}
			else if (m_status == State::Normal)
			{
				color = window->Appearance->ButtonBackground;
			}
			else if (m_status == State::Hovered)
			{
				color = window->Appearance->ButtonHighlightBackground;
			}
			else if (m_status == State::Pressed)
			{
				color = window->Appearance->ButtonPressedBackground;
			}
			graphics.DrawRoundRectBox(backgroundRect, color, enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, true);

			auto two =window->ToScale(3);
			auto colorRect = backgroundRect;
			colorRect.X += two;
			colorRect.Y += two;
			colorRect.Width -= two * 2;
			colorRect.Height -= two * 2;
			graphics.FillRectangle(colorRect, m_colorSelected);
		}

		void Reactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
		{
			m_status = State::Hovered;

			GUI::MarkAsNeedUpdate(*m_control);
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			m_status = State::Normal;

			GUI::MarkAsNeedUpdate(*m_control);
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			m_status = State::Pressed;

			GUI::Capture(*m_control);
			GUI::MarkAsNeedUpdate(*m_control);
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			GUI::ReleaseCapture(*m_control);
			
			if (m_control->Handle()->ClientSize.IsInside(args.Position))
			{
				m_status = State::Hovered;
			}
			else
			{
				m_status = State::Normal;
			}

			GUI::MarkAsNeedUpdate(*m_control);
		}
	}
	
	ColorPicker::ColorPicker(Window* parent, const Rectangle& rectangle, const Color& color)
	{
		Create(parent, true, rectangle);

		SetCaption(color.ToWString());
		
#if BT_DEBUG
		m_handle->Name = "ColorPicker";
#endif
	}

	Color ColorPicker::GetColor() const
	{
		return GetReactor().GetColor();
	}

	void ColorPicker::SetColor(const Color& color)
	{
		GetReactor().SetColor(color);
		GUI::MarkAsNeedUpdate(m_handle);
	}
}
