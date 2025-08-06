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
	void PropertyGridFieldString::Draw(Berta::Graphics& graphics, const Berta::Rectangle& area, uint32_t labelWidth, const Berta::Color& textColor)
	{
		Berta::PropertyGridField::Draw(graphics, area, labelWidth, textColor);

		Berta::Rectangle valueRect = area;

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
		PropertyGridField::SetEnabled(enabled);
		m_inputText.SetEnabled(enabled);
	}

	void PropertyGridFieldString::SetValue(const std::string& value)
	{
		PropertyGridField::SetValue(value);
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

	void PropertyGridFieldString::Create(Berta::Window* parent)
	{
		m_inputText.Create(parent);
		m_inputText.SetCaption(m_value);

		m_inputText.GetEvents().Click.Connect([](const ArgClick& args)
			{

			});

		m_inputText.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
			{
				if (args.Key == KeyboardKey::Enter && m_inputText.GetCaption() != PropertyGridField::GetValue())
				{
					PropertyGridField::SetValue(m_inputText.GetCaption());
					EmitEvent();
				}
			});

		m_inputText.GetEvents().Focus.Connect([](const ArgFocus& args)
			{

			});
	}
}
