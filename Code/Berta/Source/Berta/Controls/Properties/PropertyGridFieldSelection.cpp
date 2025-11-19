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
	void PropertyGridFieldSelection::Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor)
	{
		PropertyGridFieldBase::Draw(graphics, area, labelWidth, textColor);

		Rectangle valueRect = area;

		valueRect.X += static_cast<int>(labelWidth);
		valueRect.Width -= labelWidth;

		if (valueRect.Width == 0)
			return;

		valueRect.X = 0;
		valueRect.Y = 0;
		m_comboBox.SetArea(valueRect);
		m_comboBox.Show();
	}

	void PropertyGridFieldSelection::SetEnabled(bool enabled)
	{
		PropertyGridFieldBase::SetEnabled(enabled);
		m_comboBox.SetEnabled(enabled);
	}

	void PropertyGridFieldSelection::SetValue(const std::string& value)
	{
		try
		{
			int indexValue{};
			std::istringstream iss(value);
			iss >> indexValue;

			m_comboBox.SetSelectedIndex(indexValue);
			PropertyGridFieldBase::SetValue(value);
		}
		catch (...)
		{
		}
	}

	void PropertyGridFieldSelection::SetOption(uint32_t index)
	{
		if (index >= m_comboBox.Count())
			return;

		m_comboBox.SetSelectedIndex(index);
		PropertyGridFieldBase::SetValue(std::to_string(index));
	}

	void PropertyGridFieldSelection::PushItem(const std::string& optionText)
	{
		m_comboBox.PushItem(optionText);
	}

	void PropertyGridFieldSelection::Set(const std::vector<std::string>& options, bool clear)
	{
		if (clear)
			m_comboBox.Clear();

		for (auto& itemText : options)
		{
			m_comboBox.PushItem(itemText);
		}
	}

	void PropertyGridFieldSelection::Create(Window* parent)
	{
		m_comboBox.Create(parent);
		m_comboBox.SetCaption(m_value);

		m_comboBox.GetEvents().MouseDown.Connect([this](const ArgMouse& args)
			{
				ScrollToView();
			});

		m_comboBox.GetEvents().Focus.Connect([this](const ArgFocus& args)
			{
				if (args.Focused)
				{
					EmitSelectionEvent();
					return;
				}
			});

		m_comboBox.GetEvents().Selected.Connect([this](const ArgComboBox& args)
			{
				SetOption(args.SelectedIndex);
				EmitEvent();
			});
	}
}
