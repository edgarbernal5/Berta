/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/PropertyGrid.h>
#include <Berta/Controls/InputText.h>

class PropertyGridFieldVector3 : public Berta::PropertyGridField
{
public:
	PropertyGridFieldVector3(const std::string& label, const std::string value = "") : 
		Berta::PropertyGridField(label, value)
	{
	}

	virtual void Draw(Berta::Graphics& graphics, const Berta::Rectangle& area, uint32_t labelWidth, const Berta::Color& textColor) override;

protected:
	void Create(Berta::Window* parent) override;

private:
	Berta::InputText m_inputTexts[3];
	std::string m_inputTextLabels[3]{ "X", "Y", "Z" };
};

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

	if (valueRect.Width > 0)
	{
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
			int innerLabelOffset = (int)((innerLabelWidth - inputTextExtent.Width)) >> 1;
			graphics.DrawString({ inputRect.X + innerLabelOffset, inputRect.Y }, m_inputTextLabels[i], textColor);

			inputRect.X += innerLabelWidth - panelSaved.X;
			inputRect.Y -= panelSaved.Y;
			inputRect.Width = inputSize;

			input.SetArea(inputRect);
			input.Show();

			x += eachSize;
		}
	}
}

int main()
{
	Berta::Form form(Berta::Size(750u, 650u), { true, true, true });
	form.SetCaption("Property Grid - Example");

	Berta::PropertyGrid propertyGrid(form, { 15,15,280,600 });

	auto categoryTransform = propertyGrid.Append("Transform");
	for (size_t i = 0; i < 50; i++)
	{
		categoryTransform.Append(Berta::PropertyGridFieldPtr(new PropertyGridFieldVector3("Position")));
	}

	auto categoryMesh = propertyGrid.Append("Mesh");

	form.SetLayout("{HorizontalLayout {a}{b}");

	auto& layout = form.GetLayout();
	layout.Attach("a", propertyGrid);
	layout.Apply();

	form.Show();
	form.Exec();

	return 0;
}
