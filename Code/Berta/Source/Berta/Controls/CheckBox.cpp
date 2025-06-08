/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "CheckBox.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void CheckBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;

		m_module.m_window = m_control->Handle();
		m_module.m_events = reinterpret_cast<CheckBoxEvents*>(m_control->Handle()->Events.get());
	}

	void CheckBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		auto backgroundRect = window->ClientSize.ToRectangle();
		auto leftTextMargin = window->ToScale(3);
		graphics.DrawRectangle(backgroundRect, enabled ? window->Appearance->Background : window->Appearance->ButtonDisabledBackground, true);

		auto checkboxHeight = window->ToScale(window->Appearance->CheckboxHeight);
		Rectangle checkBoxRect{ 0, static_cast<int>((window->ClientSize.Height - checkboxHeight) >> 1),checkboxHeight, checkboxHeight };
		graphics.DrawRectangle(checkBoxRect, window->Appearance->BoxBackground, true);
		graphics.DrawRectangle(checkBoxRect, window->Appearance->BoxBorderColor, false);

		if (m_module.m_isChecked)
		{
			auto one = window->ToScale(1);
			auto two = window->ToScale(2);
			auto three = window->ToScale(3);
			auto five = window->ToScale(5);
			auto six = window->ToScale(6);
			auto lineWidth = window->ToScale(2.0f);
			
			graphics.DrawLine({ checkBoxRect.X + one * 2, checkBoxRect.Y + static_cast<int>(checkBoxRect.Height) - six },
				{ checkBoxRect.X + five, checkBoxRect.Y + static_cast<int>(checkBoxRect.Height) - three }, lineWidth,
				window->Appearance->Foreground2nd);
			
			graphics.DrawLine({ checkBoxRect.X + five, checkBoxRect.Y + static_cast<int>(checkBoxRect.Height) - three },
				{ checkBoxRect.X + static_cast<int>(checkBoxRect.Width) - three, checkBoxRect.Y + one * 2 }, lineWidth,
				window->Appearance->Foreground2nd);
		}

		int positionY = static_cast<int>((window->ClientSize.Height - graphics.GetTextExtent().Height) >> 1);
		graphics.DrawString({ checkBoxRect.X + static_cast<int>(checkBoxRect.Width) + leftTextMargin, positionY }, m_control->GetCaption(), window->Appearance->Foreground);
	}

	void CheckBoxReactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Hovered;

		GUI::MarkAsNeedUpdate(*m_control);
	}

	void CheckBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Normal;

		GUI::MarkAsNeedUpdate(*m_control);
	}

	void CheckBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Pressed;

		GUI::MarkAsNeedUpdate(*m_control);

		GUI::Capture(*m_control);
	}

	void CheckBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		GUI::ReleaseCapture(*m_control);
		GUI::MarkAsNeedUpdate(*m_control);

		if (m_control->Handle()->ClientSize.IsInside(args.Position))
		{
			m_status = State::Hovered;
			m_module.m_isChecked = !m_module.m_isChecked;
			m_module.EmitCheckedChangedEvent();
		}
		else
		{
			m_status = State::Normal;
		}
	}

	void CheckBoxReactor::Module::EmitCheckedChangedEvent()
	{
		ArgCheckBox argCheckBox{ m_isChecked };
		m_events->CheckedChanged.Emit(argCheckBox);
	}

	CheckBox::CheckBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "CheckBox";
#endif
	}

	CheckBox::CheckBox(Window* parent, const Rectangle& rectangle, const std::wstring& text)
	{
		Create(parent, true, rectangle);
		SetCaption(text);

#if BT_DEBUG
		m_handle->Name = "CheckBox";
#endif
	}

	CheckBox::CheckBox(Window* parent, const Rectangle& rectangle, const std::string& text)
	{
		Create(parent, true, rectangle);
		SetCaption(text);

#if BT_DEBUG
		m_handle->Name = "CheckBox";
#endif
	}

	bool CheckBox::IsChecked() const
	{
		return m_reactor.GetModule().m_isChecked;
	}

	void CheckBox::SetChecked(bool isChecked)
	{
		m_reactor.GetModule().m_isChecked = isChecked;
	}
}