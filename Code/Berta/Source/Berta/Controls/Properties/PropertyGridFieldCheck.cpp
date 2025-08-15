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
	void PropertyGridFieldCheck::Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor)
	{
		PropertyGridFieldBase::Draw(graphics, area, labelWidth, textColor);

		Rectangle valueRect = area;

		valueRect.X += static_cast<int>(labelWidth);
		valueRect.Width -= labelWidth;

		if (valueRect.Width == 0)
			return;

		valueRect.X = 0;
		valueRect.Y = 0;

		auto fieldSize = GetSize();
		valueRect.Height = fieldSize;
		valueRect.Width = fieldSize;
		m_checkBox.SetArea(valueRect);
		m_checkBox.Show();
	}

	bool PropertyGridFieldCheck::IsChecked() const
	{
		return PropertyGridFieldBase::GetValue() == "1";
	}

	void PropertyGridFieldCheck::SetCheck(bool checked)
	{
		PropertyGridFieldBase::SetValue(checked ? "1" : "0");
		m_checkBox.SetChecked(checked);
	}

	void PropertyGridFieldCheck::SetEnabled(bool enabled)
	{
		PropertyGridFieldBase::SetEnabled(enabled);
		m_checkBox.SetEnabled(enabled);
	}

	void PropertyGridFieldCheck::SetValue(const std::string& value)
	{
		if (value == "T" || value == "t" || value == "true" || value == "1")
		{
			m_checkBox.SetChecked(true);
			PropertyGridFieldBase::SetValue("1");
		}
		else if (value == "F" || value == "f" || value == "false" || value == "0")
		{
			m_checkBox.SetChecked(false);
			PropertyGridFieldBase::SetValue("0");
		}
	}

	void PropertyGridFieldCheck::Create(Window* parent)
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
