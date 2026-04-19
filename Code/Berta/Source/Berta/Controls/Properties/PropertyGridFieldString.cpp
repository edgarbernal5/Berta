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
	}

	void PropertyGridFieldString::Refresh()
	{
	}

	std::string PropertyGridFieldString::GetValueAsString() const
	{
		return m_getter();
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
		m_inputText.SetCaption(m_getter());
		m_inputText.SetFocusBehavior(TextFocusBehavior::SelectOnClick);

		m_inputText.GetEvents().Click.Connect([this](const ArgClick& args)
			{
				
			});

		m_inputText.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
			{
				if (args.Key == KeyboardKey::Enter && m_inputText.GetCaption() != m_getter())
				{
					m_setter(m_inputText.GetCaption());
					NotifyValueChanged();
				}
			});

		m_inputText.GetEvents().Focus.Connect([this](const ArgFocus& args)
			{
				if (args.Focused)
				{
					NotifySelected();
					return;
				}

				if (m_inputText.GetCaption() != m_getter())
				{
					m_setter(m_inputText.GetCaption());
					NotifyValueChanged();
				}
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
}
