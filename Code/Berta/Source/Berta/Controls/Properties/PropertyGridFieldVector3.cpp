/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldVector3.h"

#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	void PropertyGridFieldVector3::Create(Berta::Window* parent)
	{
		for (auto& input : m_inputTexts)
		{
			input.Create(parent);
#if BT_DEBUG
			input.SetDebugName("InputText");
#endif

			input.GetEvents().Click.Connect([](const ArgClick& args)
				{

				});

			input.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
				{
					if (args.Key == KeyboardKey::Enter)
					{
						auto newValue = m_inputTexts[0].GetCaption() + "/" + m_inputTexts[1].GetCaption() + "/" + m_inputTexts[2].GetCaption();
						if (newValue != PropertyGridField::GetValue())
						{
							PropertyGridField::SetValue(newValue);
							EmitEvent();
						}
					}
				});

			input.GetEvents().Focus.Connect([](const ArgFocus& args)
				{

				});
		}
	}

	void PropertyGridFieldVector3::Draw(Berta::Graphics& graphics, const Berta::Rectangle& area, uint32_t labelWidth, const Berta::Color& textColor)
	{
		Berta::PropertyGridField::Draw(graphics, area, labelWidth, textColor);

		Berta::Rectangle valueRect = area;

		valueRect.X += static_cast<int>(labelWidth);
		valueRect.Width -= labelWidth;

		if (valueRect.Width == 0)
			return;
		
		auto innerLabelExtents = graphics.GetTextExtent("X");
		auto innerLabelWidth = innerLabelExtents.Width * 2;
		auto panelSaved = valueRect;
		auto eachSize = valueRect.Width / 3;
		auto inputSize = eachSize - innerLabelWidth - m_parent->ToScale(4u);
		int x = 0;
		for (size_t i = 0; i < 3; i++)
		{
			auto& input = m_inputTexts[i];
			auto inputTextExtent = graphics.GetTextExtent(m_inputTextLabels[i]);

			Berta::Rectangle inputRect = panelSaved;
			inputRect.X += x;
			int innerLabelOffsetX = (int)((innerLabelWidth - inputTextExtent.Width)) >> 1;
			int innerLabelOffsetY = (int)((area.Height - innerLabelExtents.Height)) >> 1;
			graphics.DrawString({ inputRect.X + innerLabelOffsetX, inputRect.Y + innerLabelOffsetY }, m_inputTextLabels[i], textColor);

			inputRect.X += innerLabelWidth - panelSaved.X;
			inputRect.Y -= panelSaved.Y;
			inputRect.Width = inputSize;

			input.SetArea(inputRect);
			input.Show();

			x += eachSize;
		}
	}

	void PropertyGridFieldVector3::SetEnabled(bool enabled)
	{
		PropertyGridField::SetEnabled(enabled);
		for (auto& input : m_inputTexts)
		{
			input.SetEnabled(enabled);
		}
	}

	void PropertyGridFieldVector3::SetValue(const std::string& value)
	{
		std::stringstream ss(value);
		std::string item;
		std::vector<float> items;

		try
		{
			while (getline(ss, item, '/'))
			{
				items.push_back(item.empty() ? 0 : std::stof(item));
			}
		}
		catch (...)
		{
			items.clear();
		}

		for (size_t i = 0; i < 3; ++i)
		{
			m_inputTexts[i].SetCaption(std::to_string(items[i]));
		}

		PropertyGridField::SetValue(std::to_string(items[0]) + "/" + std::to_string(items[1]) + "/" + std::to_string(items[2]));
	}
}
