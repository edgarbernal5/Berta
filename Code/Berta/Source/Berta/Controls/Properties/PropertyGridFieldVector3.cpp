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
	void PropertyGridFieldVector3::Create(Window* parent)
	{
		for (auto& input : m_inputTexts)
		{
			input.Create(parent);
#if BT_DEBUG
			input.SetDebugName("InputText");
#endif
			input.SetFocusBehavior(TextFocusBehavior::SelectOnClick);

			input.GetEvents().Click.Connect([](const ArgClick& args)
				{

				});

			input.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
				{
					if (args.Key == KeyboardKey::Enter)
					{
						auto newValue = m_inputTexts[0].GetCaption() + "/" + m_inputTexts[1].GetCaption() + "/" + m_inputTexts[2].GetCaption();
						if (newValue != PropertyGridFieldBase::GetValue())
						{
							PropertyGridFieldBase::SetValue(newValue);
							EmitEvent();
						}
					}
				});

			input.SetCharFilter([&input](wchar_t chr)
				{
					auto isDigit = std::isdigit(chr);
					auto onlyOneDot = chr == '.' && input.GetText().find('.') == std::string::npos;

					return isDigit || onlyOneDot;
				});

			input.GetEvents().Focus.Connect([this](const ArgFocus& args)
				{
					if (args.Focused)
					{
						EmitSelectionEvent();
						return;
					}

					auto newValue = m_inputTexts[0].GetCaption() + "/" + m_inputTexts[1].GetCaption() + "/" + m_inputTexts[2].GetCaption();
					if (newValue != PropertyGridFieldBase::GetValue())
					{
						PropertyGridFieldBase::SetValue(newValue);
						EmitEvent();
					}
				});
		}

		PropertyGridFieldVector3::SetValue(m_value);
	}

	void PropertyGridFieldVector3::Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor)
	{
		PropertyGridFieldBase::Draw(graphics, area, labelWidth, textColor);

		Rectangle valueRect = area;

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

			Rectangle inputRect = panelSaved;
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
		PropertyGridFieldBase::SetEnabled(enabled);
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

		if (items.size() != 3)
			return;

		for (size_t i = 0; i < 3; ++i)
		{
			m_inputTexts[i].SetCaption(std::to_string(items[i]));
		}

		PropertyGridFieldBase::SetValue(std::to_string(items[0]) + "/" + std::to_string(items[1]) + "/" + std::to_string(items[2]));
	}
}
