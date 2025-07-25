/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldVector3.h"

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
}
