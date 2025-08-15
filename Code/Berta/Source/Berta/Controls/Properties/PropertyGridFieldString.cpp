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
	void PropertyGridFieldString::Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor)
	{
		PropertyGridFieldBase::Draw(graphics, area, labelWidth, textColor);

		Rectangle valueRect = area;

		valueRect.X += static_cast<int>(labelWidth);
		valueRect.Width -= labelWidth;

		if (valueRect.Width == 0)
			return;

		valueRect.X = 0;
		valueRect.Y = 0;
		m_inputText.SetArea(valueRect);
		m_inputText.Show();
	}

	void PropertyGridFieldString::SetEnabled(bool enabled)
	{
		PropertyGridFieldBase::SetEnabled(enabled);
		m_inputText.SetEnabled(enabled);
	}

	void PropertyGridFieldString::SetValue(const std::string& value)
	{
		PropertyGridFieldBase::SetValue(value);
		m_inputText.SetCaption(value);
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

	void PropertyGridFieldString::Create(Window* parent)
	{
		m_inputText.Create(parent);
		m_inputText.SetCaption(m_value);

		m_inputText.GetEvents().Click.Connect([](const ArgClick& args)
			{

			});

		m_inputText.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
			{
				if (args.Key == KeyboardKey::Enter && m_inputText.GetCaption() != PropertyGridFieldBase::GetValue())
				{
					PropertyGridFieldBase::SetValue(m_inputText.GetCaption());
					EmitEvent();
				}
			});

		m_inputText.GetEvents().Focus.Connect([this](const ArgFocus& args)
			{
				if (args.Focused)
					return;

				if (m_inputText.GetCaption() != PropertyGridFieldBase::GetValue())
				{
					PropertyGridFieldBase::SetValue(m_inputText.GetCaption());
					EmitEvent();
				}
			});
	}
}
