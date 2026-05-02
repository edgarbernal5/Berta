/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldString.h"

#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	void PropertyGridFieldString::Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config)
	{
		m_textBox.SetArea(area);
	}

	void PropertyGridFieldString::SetFocus()
	{
		m_textBox.Focus();
	}

	void PropertyGridFieldString::Refresh()
	{
		if (!m_getter)
		{
			return;
		}
		
		std::optional<std::string> currentOpt = m_getter();
		if (currentOpt.has_value())
		{
			if (m_textBox.GetCaption() != currentOpt.value())
			{
				m_textBox.SetCaption(currentOpt.value());
			}
		}
		else
		{
			if (m_textBox.GetCaption() != "---")
			{
				m_textBox.SetCaption("---");
			}
		}
	}

	std::string PropertyGridFieldString::GetValueAsString() const
	{
		if (m_getter)
		{
			std::optional<std::string> currentOpt = m_getter();
			return currentOpt.has_value() ? currentOpt.value() : "---";
		}
		return "";
	}
	
	void PropertyGridFieldString::SetEditable(bool isEditable)
	{
		m_textBox.SetEditable(isEditable);
	}

	bool PropertyGridFieldString::IsEditable() const
	{
		return m_textBox.IsEditable();
	}

	void PropertyGridFieldString::SetCharFilter(std::function<bool(wchar_t)> predicate)
	{
		m_textBox.SetCharFilter(predicate);
	}

	void PropertyGridFieldString::OnCreate(Window* parent)
	{
		m_textBox.Create(parent);
		m_textBox.SetFocusBehavior(TextFocusBehavior::SelectOnClick);
		m_textBox.SetScrollBarVisibility(ScrollBarVisibility::Hidden);
		
		Refresh();
		
		m_textBox.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
			{
				if (args.Key == KeyboardKey::Enter)
				{
					ApplyValue();
				}
			});

		m_textBox.GetEvents().Focus.Connect([this](const ArgFocus& args)
			{
				if (args.Focused)
				{
					NotifySelected();
					return;
				}

				ApplyValue();
			});
	}

	void PropertyGridFieldString::OnVisibilityChanged(bool visible)
	{
		if (visible)
		{
			m_textBox.Show();
		}
		else
		{
			m_textBox.Hide();
		}
	}

	void PropertyGridFieldString::OnEnableChanged(bool enabled)
	{
		m_textBox.SetEnabled(enabled);
	}

	void PropertyGridFieldString::ApplyValue()
	{
		if (!m_setter || !m_getter)
		{
			return;
		}

		std::string uiValue = m_textBox.GetCaption();
		std::optional<std::string> currentOpt = m_getter();

		if (!currentOpt.has_value() && uiValue == "---") 
		{
			return;
		}

		if (!currentOpt.has_value() || currentOpt.value() != uiValue)
		{
			m_setter(uiValue);
			NotifyValueChanged();
		}

		Refresh();
	}
}
