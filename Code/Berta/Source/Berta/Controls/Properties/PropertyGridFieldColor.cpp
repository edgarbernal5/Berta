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
	PropertyGridFieldColor::PropertyGridFieldColor(std::string_view label, GetterFn getter, SetterFn setter, ClickCallback onButtonClick) :
		TypedPropertyField(label, std::move(getter), std::move(setter)), m_clickCallback(std::move(onButtonClick))
	{
	}
	
	void PropertyGridFieldColor::Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config)
	{
		m_colorRegion.SetArea(area);
	}

	void PropertyGridFieldColor::SetFocus()
	{
	}

	bool PropertyGridFieldColor::HasFocus() const
	{
		return false;
	}

	std::wstring PropertyGridFieldColor::GetValueAsString() const
	{
		if (!m_getter)
		{
			return L"";
		}
		
		auto currentValue = m_getter();
		if (currentValue.has_value())
		{
			Color c = currentValue.value();
			return std::to_wstring(c.GetR()) + L"," + std::to_wstring(c.GetG()) + L"," + 
				   std::to_wstring(c.GetB()) + L"," + std::to_wstring(c.GetA());
		}
		return L"---";
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

	void PropertyGridFieldColor::SetValueInternal(const Color& value)
	{
		m_colorRegion.SetBackgroundColor(value);
		m_colorRegion.SetCaption("");
	}

	void PropertyGridFieldColor::SetMixedValuesInternal()
	{
		m_colorRegion.SetBackgroundColor(Color(128, 128, 128, 255));
		m_colorRegion.SetCaption("---");
	}
}
