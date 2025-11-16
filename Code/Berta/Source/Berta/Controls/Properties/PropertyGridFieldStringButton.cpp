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
	void PropertyGridFieldStringButton::Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor)
	{
		PropertyGridFieldBase::Draw(graphics, area, labelWidth, textColor);

		Rectangle valueRect = area;

		valueRect.X += static_cast<int>(labelWidth);
		valueRect.Width -= labelWidth;

		if (valueRect.Width == 0)
			return;

		valueRect.X = 0;
		valueRect.Y = 0;
		auto buttonSize = m_parent->ToScale(24);
		auto margin = m_parent->ToScale(2);
		valueRect.Width -= buttonSize + margin;

		m_inputText.SetArea(valueRect);
		m_inputText.Show();

		valueRect.X += valueRect.Width + margin;
		valueRect.Width = buttonSize;

		m_button.SetArea(valueRect);
		m_button.Show();
	}

	void PropertyGridFieldStringButton::SetEnabled(bool enabled)
	{
		PropertyGridFieldString::SetEnabled(enabled);
		m_button.SetEnabled(enabled);
	}

	void PropertyGridFieldStringButton::SetButtonClick(std::function<void(PropertyGridFieldStringButton*)> callback)
	{
		m_clickCallback = std::move(callback);
	}

	void PropertyGridFieldStringButton::Create(Window* parent)
	{
		PropertyGridFieldString::Create(parent);

		m_button.Create(parent);
		m_button.SetCaption(m_buttonText);

		m_button.GetEvents().Click.Connect([this](const ArgClick& args)
			{
				ScrollToView();
				if (m_clickCallback)
				{
					m_clickCallback(this);
				}
			});

		m_button.MakeActive(false, m_inputText);
	}
}
