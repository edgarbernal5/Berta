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
	void PropertyGridFieldSelection::Draw(Berta::Graphics& graphics, const Berta::Rectangle& area, uint32_t labelWidth, const Berta::Color& textColor)
	{
		Berta::PropertyGridField::Draw(graphics, area, labelWidth, textColor);

		Berta::Rectangle valueRect = area;

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
		PropertyGridField::SetEnabled(enabled);
		m_comboBox.SetEnabled(enabled);
	}

	void PropertyGridFieldSelection::SetValue(const std::string& value)
	{
		PropertyGridField::SetValue(value);

	}

	void PropertyGridFieldSelection::SetOption(uint32_t index)
	{
		if (index >= m_comboBox.Count())
			return;

		m_comboBox.SetSelectedIndex(index);
		PropertyGridField::SetValue(std::to_string(index));
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

	void PropertyGridFieldSelection::Create(Berta::Window* parent)
	{
		m_comboBox.Create(parent);
		m_comboBox.SetCaption(m_value);

		m_comboBox.GetEvents().Click.Connect([](const ArgClick& args)
			{

			});
		m_comboBox.GetEvents().Selected.Connect([this](const ArgComboBox& args)
			{
				SetOption(args.SelectedIndex);
				EmitEvent();
			});
	}
}
