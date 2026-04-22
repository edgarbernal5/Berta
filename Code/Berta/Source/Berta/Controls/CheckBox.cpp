/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "CheckBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/Controls/Checking/CheckBoxHelpers.h"

namespace Berta
{
	namespace Internal::CheckBox
	{
		void Reactor::DoOnInit()
		{
			m_module.m_window = m_control->Handle();
			m_module.m_events = reinterpret_cast<Events*>(m_control->Handle()->Events.get());
		}

		void Reactor::Update(Graphics& graphics)
		{
			auto window = m_control->Handle();
			bool enabled = m_control->GetEnabled();
			auto backgroundRect = window->ClientSize.ToRectangle();
			auto leftTextMargin = window->ToScale(3);

			auto checkboxHeight = window->ToScale(window->Appearance->CheckboxHeight);
			Rectangle checkBoxRect{ 0, static_cast<int>((window->ClientSize.Height - checkboxHeight) >> 1),checkboxHeight, checkboxHeight };
			graphics.FillRectangle(checkBoxRect, window->Appearance->BoxBackground);
			graphics.DrawRectangle(checkBoxRect, window->Appearance->BoxBorderColor);

			if (m_module.m_checkState == CheckState::Checked)
			{
				int checkX = checkBoxRect.X;
				int checkY = checkBoxRect.Y;
				//if (checkY % 2 == 0) checkY--;
				
				Berta::Internal::CheckBox::DrawCheckmark(graphics, 
					{ checkX, checkY }, 
					static_cast<int>(checkboxHeight),
					window->Appearance->Foreground2nd, 1.5f);
			} 
			else if (m_module.m_checkState == CheckState::Indeterminate)
			{
				Rectangle indeterminateRect = checkBoxRect;
				indeterminateRect.X += 2;
				indeterminateRect.Y += 2;
				indeterminateRect.Width -= 4;
				indeterminateRect.Height -= 4;
				graphics.FillRectangle(indeterminateRect, window->Appearance->HighlightColor);
			}

			int positionY = static_cast<int>((window->ClientSize.Height - graphics.GetTextExtent().Height) >> 1);
			graphics.DrawString({ checkBoxRect.X + static_cast<int>(checkBoxRect.Width) + leftTextMargin, positionY }, m_control->GetCaption(), window->Appearance->Foreground);
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

			GUI::MarkAsNeedUpdate(*m_control);

			GUI::Capture(*m_control);
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			GUI::ReleaseCapture(*m_control);
			GUI::MarkAsNeedUpdate(*m_control);

			if (m_control->Handle()->ClientSize.IsInside(args.Position))
			{
				m_status = State::Hovered;
				if (m_module.m_checkState == CheckState::Checked)
				{
					m_module.m_checkState = CheckState::Unchecked;
				}
				else
				{
					m_module.m_checkState = CheckState::Checked;
				}
				m_module.EmitCheckedChangedEvent();
			}
			else
			{
				m_status = State::Normal;
			}
		}

		void Reactor::Module::EmitCheckedChangedEvent() const
		{
			ArgCheckBox argCheckBox{ m_checkState };
			m_events->CheckedChanged.Emit(argCheckBox);
		}
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
		return GetReactor().GetModule().m_checkState == CheckState::Checked;
	}

	void CheckBox::SetChecked(bool isChecked)
	{
		auto& module = GetReactor().GetModule();
		if (module.m_checkState == CheckState::Checked)
			return;
		
		module.m_checkState = isChecked ? CheckState::Checked : CheckState::Unchecked;
		GUI::UpdateWindow(m_handle);
	}

	CheckState CheckBox::GetState() const
	{
		return GetReactor().GetModule().m_checkState;
	}

	void CheckBox::SetState(CheckState state)
	{
		auto& module = GetReactor().GetModule();
		if (module.m_checkState == state)
			return;
		
		module.m_checkState = state;
		GUI::UpdateWindow(m_handle);
	}
}
