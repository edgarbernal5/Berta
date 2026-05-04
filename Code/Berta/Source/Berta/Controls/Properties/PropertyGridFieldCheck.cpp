/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldCheck.h"

#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	void PropertyGridFieldCheck::Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config)
	{
		Rectangle valueRect = area;
		valueRect.Height = area.Height;
		valueRect.Width = area.Height;
		m_checkBox.SetArea(valueRect);
	}

	void PropertyGridFieldCheck::SetFocus()
	{
		m_checkBox.Focus(); 
	}

	bool PropertyGridFieldCheck::HasFocus() const
	{
		return false;
	}

	void PropertyGridFieldCheck::Refresh()
	{
		if (!m_getter)
		{
			return;
		}
		
		std::optional<bool> currentState = m_getter();
		CheckState targetState;

		if (!currentState.has_value())
		{
			targetState = CheckState::Indeterminate;
		}
		else if (currentState.value() == true)
		{
			targetState = CheckState::Checked;
		}
		else
		{
			targetState = CheckState::Unchecked;
		}
		
		if (m_checkBox.GetState() != targetState)
		{
			m_checkBox.SetState(targetState);
		}
	}

	std::string PropertyGridFieldCheck::GetValueAsString() const
	{
		if (m_getter)
		{
			auto value = m_getter();
			return value ? "1" : "0";
		}
		return "";
	}

	void PropertyGridFieldCheck::OnCreate(Window* parent)
	{
		m_checkBox.Create(parent);
		Refresh();
		
		m_checkBox.GetEvents().CheckedChanged.Connect([this](const ArgCheckBox& args)
		{
			if (!m_getter || !m_setter) return;
			
			std::optional<bool> currentState = m_getter();

			bool newValue;
			if (!currentState.has_value())
			{
				newValue = true; 
			}
			else
			{
					
				newValue = !currentState.value();
			}

			m_setter(newValue);
			NotifyValueChanged();
		});

		m_checkBox.GetEvents().Focus.Connect([this](const ArgFocus& args)
		{
			if (args.Focused)
			{
				NotifySelected();
			}
		});
	}

	void PropertyGridFieldCheck::OnVisibilityChanged(bool visible)
	{
		if (visible)
		{
			m_checkBox.Show();
		}
		else
		{
			m_checkBox.Hide();
		}
	}

	void PropertyGridFieldCheck::OnEnableChanged(bool enabled)
	{
		m_checkBox.SetEnabled(enabled);
	}
}
