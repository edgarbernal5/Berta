/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldColor.h"

#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	void PropertyGridFieldColor::Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor)
	{
		PropertyGridFieldBase::Draw(graphics, area, labelWidth, textColor);

		Rectangle valueRect = area;

		valueRect.X += static_cast<int>(labelWidth);
		valueRect.Width -= labelWidth;

		if (valueRect.Width == 0)
			return;

		valueRect.X = 0;
		valueRect.Y = 0;
		m_colorRegion.SetArea(valueRect);
		m_colorRegion.Show();
	}

	void PropertyGridFieldColor::SetEnabled(bool enabled)
	{
		PropertyGridFieldBase::SetEnabled(enabled);
		m_colorRegion.SetEnabled(enabled);
	}

	void PropertyGridFieldColor::SetValue(const std::string& value)
	{
		std::stringstream ss(value);
		std::string item;
		std::vector<int> items;

		try
		{
			while (getline(ss, item, ','))
			{
				items.push_back(item.empty() ? 0 : std::clamp(std::stoi(item), 0, 255));
			}
		}
		catch (...)
		{
			items.clear();
		}

		if (items.size() != 4)
			return;

		m_color = Color(items[0], items[1], items[2], items[3]);
		m_colorRegion.SetBackgroundColor(m_color);
		PropertyGridFieldBase::SetValue(std::to_string(items[0]) + "," + std::to_string(items[1]) + "," + std::to_string(items[2]) + "," + std::to_string(items[3]));
	}

	void PropertyGridFieldColor::SetValue(const Color& value)
	{
		PropertyGridFieldColor::SetValue(std::to_string(value.GetR()) + "," + std::to_string(value.GetB()) + "," + std::to_string(value.GetB()) + "," + std::to_string(value.GetA()));
	}

	Color PropertyGridFieldColor::ToColor() const
	{
		return m_color;
	}

	void PropertyGridFieldColor::SetButtonClick(std::function<void(PropertyGridFieldColor*)> callback)
	{
		m_clickCallback = std::move(callback);
	}

	void PropertyGridFieldColor::Create(Window* parent)
	{
		m_colorRegion.Create(parent);

		m_colorRegion.GetEvents().Click.Connect([this](const ArgClick& args)
			{
				if (m_clickCallback)
				{
					m_clickCallback(this);
				}
			});

		PropertyGridFieldColor::SetValue(m_value);
	}
}
