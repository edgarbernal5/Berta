/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldSelection.h"

#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	void PropertyGridFieldSelection::Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config)
	{
		m_comboBox.SetArea(area);
	}

	void PropertyGridFieldSelection::SetFocus()
	{
		m_comboBox.Focus();
	}

	void PropertyGridFieldSelection::Refresh()
	{
	}

	std::string PropertyGridFieldSelection::GetValueAsString() const
	{
		return "";
	}

	void PropertyGridFieldSelection::SetOption(std::optional<size_t> index)
	{
		if (index && index >= m_comboBox.Count())
		{
			return;	
		}

		m_comboBox.SetSelectedIndex(index);
	}

	void PropertyGridFieldSelection::PushItem(const std::string& optionText)
	{
		m_comboBox.PushBack(optionText);
	}

	void PropertyGridFieldSelection::Set(const std::vector<std::string>& options, bool clear)
	{
		if (clear)
		{
			m_comboBox.Clear();	
		}

		for (auto& itemText : options)
		{
			m_comboBox.PushBack(itemText);
		}
	}
	
	void PropertyGridFieldSelection::OnCreate(Window* parent)
	{
		m_comboBox.Create(parent);
		Refresh();

		m_comboBox.GetEvents().Focus.Connect([this](const ArgFocus& args)
			{
				if (args.Focused)
				{
					NotifySelected();
				}
			});

		m_comboBox.GetEvents().Selected.Connect([this](const ArgComboBox& args)
			{
				SetOption(args.SelectedIndex);
				NotifyValueChanged();
			});
	}

	void PropertyGridFieldSelection::OnVisibilityChanged(bool visible)
	{
		if (visible)
		{
			m_comboBox.Show();
		}
		else
		{
			m_comboBox.Hide();
		}
	}

	void PropertyGridFieldSelection::OnEnableChanged(bool enabled)
	{
		m_comboBox.SetEnabled(enabled);
	}
}
