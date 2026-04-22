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
		m_inputText.SetArea(area);
	}

	void PropertyGridFieldString::SetFocus()
	{
		m_inputText.Focus();
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
			if (m_inputText.GetCaption() != currentOpt.value())
			{
				m_inputText.SetCaption(currentOpt.value());
			}
		}
		else
		{
			if (m_inputText.GetCaption() != "---")
			{
				m_inputText.SetCaption("---");
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
		m_inputText.SetEditable(isEditable);
	}

	bool PropertyGridFieldString::IsEditable() const
	{
		return m_inputText.IsEditable();
	}

	void PropertyGridFieldString::SetCharFilter(std::function<bool(wchar_t)> predicate)
	{
		m_inputText.SetCharFilter(predicate);
	}

	void PropertyGridFieldString::OnCreate(Window* parent)
	{
		m_inputText.Create(parent);
		m_inputText.SetFocusBehavior(TextFocusBehavior::SelectOnClick);
		Refresh();
		
		m_inputText.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
			{
				if (args.Key == KeyboardKey::Enter)
				{
					ApplyValue();
				}
			});

		m_inputText.GetEvents().Focus.Connect([this](const ArgFocus& args)
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
			m_inputText.Show();
		}
		else
		{
			m_inputText.Hide();
		}
	}

	void PropertyGridFieldString::OnEnableChanged(bool enabled)
	{
		m_inputText.SetEnabled(enabled);
	}

	void PropertyGridFieldString::ApplyValue()
	{
		if (!m_setter || !m_getter)
		{
			return;
		}

		std::string uiValue = m_inputText.GetCaption();
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
