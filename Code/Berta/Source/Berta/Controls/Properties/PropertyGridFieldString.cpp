/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldString.h"

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

		valueRect.Y = 0;
		valueRect.X = 0;
		m_inputText.SetArea(valueRect);
		m_inputText.Show();
	}

	void PropertyGridFieldString::SetValue(const std::string& value)
	{
		PropertyGridField::SetValue(value);
		m_inputText.SetCaption(value);
	}

	void PropertyGridFieldString::SetEnabled(bool enabled)
	{
		PropertyGridField::SetEnabled(enabled);
		m_inputText.SetEnabled(enabled);
	}

	void PropertyGridFieldString::Create(Berta::Window* parent)
	{
		m_inputText.Create(parent);
		m_inputText.SetCaption(m_value);

		m_inputText.GetEvents().Click.Connect([](const ArgClick& args)
			{

			});

		m_inputText.GetEvents().KeyPressed.Connect([](const ArgKeyboard& args)
			{

			});

		m_inputText.GetEvents().Focus.Connect([](const ArgFocus& args)
			{

			});
	}
}
