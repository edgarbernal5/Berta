/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldStringButton.h"

#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	void PropertyGridFieldStringButton::Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config)
	{
		int buttonWidth = m_parent->ToScale(24);
		int margin = m_parent->ToScale(2);

		Rectangle inputArea = area;
		inputArea.Width -= (buttonWidth + margin);

		Rectangle buttonArea = area;
		buttonArea.X += static_cast<int>(inputArea.Width) + margin;
		buttonArea.Width = buttonWidth;

		PropertyGridFieldString::Draw(graphics, inputArea, config);
		
		m_button.SetArea(buttonArea);
	}

	void PropertyGridFieldStringButton::SetFocus()
	{
		PropertyGridFieldString::SetFocus();
	}

	bool PropertyGridFieldStringButton::HasFocus() const
	{
		return false;
	}

	void PropertyGridFieldStringButton::SetButtonClick(ClickCallback callback)
	{
		m_clickCallback = std::move(callback);
	}

	void PropertyGridFieldStringButton::OnCreate(Window* parent)
	{
		PropertyGridFieldString::OnCreate(parent);

		m_button.Create(parent);
		m_button.SetCaption(m_buttonText);

		m_button.GetEvents().Click.Connect([this](const ArgClick& args)
		{
			NotifySelected();
			if (m_clickCallback && m_getter)
			{
				std::optional<std::wstring> result = m_clickCallback(m_getter());
				if (result.has_value() && m_setter) 
				{
					//this->SetValue(result.value());
					m_setter(result.value());
					NotifyValueChanged();
					Refresh();
				}
			}
		});

		m_button.MakeActive(false, m_textBox);
	}

	void PropertyGridFieldStringButton::OnVisibilityChanged(bool visible)
	{
		PropertyGridFieldString::OnVisibilityChanged(visible);
		if (visible)
		{
			m_button.Show();
		}
		else
		{
			m_button.Hide();
		}
	}
	
	void PropertyGridFieldStringButton::OnEnableChanged(bool enabled)
	{
		PropertyGridFieldString::OnEnableChanged(enabled);
		m_button.SetEnabled(enabled);
	}
}
