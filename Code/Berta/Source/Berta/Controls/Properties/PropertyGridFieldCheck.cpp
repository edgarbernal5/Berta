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
	void PropertyGridFieldCheck::Draw(Berta::Graphics& graphics, const Berta::Rectangle& area, uint32_t labelWidth, const Berta::Color& textColor)
	{
		Berta::PropertyGridField::Draw(graphics, area, labelWidth, textColor);

		Berta::Rectangle valueRect = area;

		valueRect.X += static_cast<int>(labelWidth);
		valueRect.Width -= labelWidth;

		if (valueRect.Width == 0)
			return;

		valueRect.X = 0;
		valueRect.Y = 0;
		m_checkBox.SetArea(valueRect);
		m_checkBox.Show();
	}

	bool PropertyGridFieldCheck::IsChecked() const
	{
		return PropertyGridField::GetValue() == "1";
	}

	void PropertyGridFieldCheck::SetCheck(bool checked)
	{
		PropertyGridField::SetValue(checked ? "1" : "0");
	}

	void PropertyGridFieldCheck::SetEnabled(bool enabled)
	{
		PropertyGridField::SetEnabled(enabled);
		m_checkBox.SetEnabled(enabled);
	}

	void PropertyGridFieldCheck::SetValue(const std::string& value)
	{
		if (value == "T" || value == "t" || value == "true" || value == "1")
		{
			m_checkBox.SetChecked(true);
			PropertyGridField::SetValue("1");
		}
		else if (value == "F" || value == "f" || value == "false" || value == "0")
		{
			m_checkBox.SetChecked(false);
			PropertyGridField::SetValue("0");
		}
	}

	void PropertyGridFieldCheck::Create(Berta::Window* parent)
	{
		m_checkBox.Create(parent);
		SetValue(m_value);

		m_checkBox.GetEvents().CheckedChanged.Connect([this](const ArgCheckBox& args)
			{
				SetCheck(args.IsChecked);
				EmitEvent();
			});
	}
}
