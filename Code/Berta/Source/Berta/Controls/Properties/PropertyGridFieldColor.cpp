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
	void PropertyGridFieldColor::Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config)
	{
		m_colorRegion.SetArea(area);
	}

	void PropertyGridFieldColor::SetFocus()
	{
	}

	void PropertyGridFieldColor::Refresh()
	{
		if (!m_getter)
		{
			return;
		}

		std::optional<Color> currentOpt = m_getter();
		if (currentOpt.has_value())
		{
			m_colorRegion.SetBackgroundColor(currentOpt.value());
			m_colorRegion.SetCaption("");
		}
		else
		{
			m_colorRegion.SetBackgroundColor(Color(128, 128, 128, 255));
			m_colorRegion.SetCaption("---");
		}
	}

	std::string PropertyGridFieldColor::GetValueAsString() const
	{
		if (!m_getter)
		{
			return "";
		}
		
		auto currentValue = m_getter();
		if (currentValue.has_value())
		{
			Color c = currentValue.value();
			return std::to_string(c.GetR()) + "," + std::to_string(c.GetG()) + "," + 
				   std::to_string(c.GetB()) + "," + std::to_string(c.GetA());
		}
		return "---";
	}

	void PropertyGridFieldColor::SetButtonClick(ClickCallback callback)
	{
		m_clickCallback = std::move(callback);
	}

	void PropertyGridFieldColor::OnCreate(Window* parent)
	{
		m_colorRegion.Create(parent);

		m_colorRegion.GetEvents().Click.Connect([this](const ArgClick& args)
		{
			NotifySelected();
			if (m_clickCallback && m_getter)
			{
				std::optional<Color> result = m_clickCallback(m_getter());
				if (result.has_value()) 
				{
					m_setter(result.value());
					NotifyValueChanged();
					Refresh();
				}
			}
		});
		m_colorRegion.GetEvents().Focus.Connect([this](const ArgFocus& args)
			{
				if (args.Focused)
				{
					NotifySelected();
				}
			});
		
		Refresh();
	}

	void PropertyGridFieldColor::OnVisibilityChanged(bool visible)
	{
		if (visible)
		{
			m_colorRegion.Show();
		}
		else
		{
			m_colorRegion.Hide();
		}
	}

	void PropertyGridFieldColor::OnEnableChanged(bool enabled)
	{
		m_colorRegion.SetEnabled(enabled);
	}
}
