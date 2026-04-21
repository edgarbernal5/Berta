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

	void PropertyGridFieldCheck::Refresh()
	{
		if (m_getter) 
		{
			auto value = m_getter();
			if (m_checkBox.IsChecked() != value)
			{
				m_checkBox.SetChecked(value);
			}
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
				m_setter(args.IsChecked);
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
