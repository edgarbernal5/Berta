/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/PropertyGrid.h>
#include <Berta/Controls/InputText.h>

#include <Berta/Controls/Properties/PropertyGridFields.h>

class NewPanel : public Berta::Panel
{
public:
	NewPanel(Berta::Window* parent) : Berta::Panel(parent)
	{
		m_inputText.Create(*this, true, {20,20,200,40});
		m_inputText.SetCaption("Two words!");
	}

private:
	Berta::InputText m_inputText;
};

int main()
{
	Berta::Form form(Berta::Size(750u, 650u), { true, true, true });
	form.SetCaption("Property Grid - Example");

	Berta::PropertyGrid propertyGrid(form, { 15,15,280,600 });

	auto categoryTransform = propertyGrid.Append("Transform");
	categoryTransform.Append(Berta::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldString("Name", "Car")));
	categoryTransform.Append(Berta::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldString("Tag", "Blue, Green")));
	for (size_t i = 0; i < 3; i++)
	{
		categoryTransform.Append(Berta::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldVector3("Position", "0.0/0.0/0.0")));
	}

	auto categoryEmpty = propertyGrid.Append("Empty");
	auto categoryMesh = propertyGrid.Append("Mesh");

	auto meshIdProp = new Berta::PropertyGridFieldString("Mesh ID", "71d3eed6-d363-428a-bc81-01576539b297");
	
	categoryMesh.Append(Berta::PropertyGridFieldBasePtr(meshIdProp)); meshIdProp->SetEditable(false);

	categoryMesh.Append(Berta::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldStringInt("Mesh Count", "0")));
	categoryMesh.Append(Berta::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldCheck("Enabled", "0")));
	std::vector<std::string> options{"None", "Material 1", "Material 2" };
	auto pgfSelection = new Berta::PropertyGridFieldSelection("Material");
	pgfSelection->Set(options);
	categoryMesh.Append(Berta::PropertyGridFieldBasePtr(pgfSelection));

	auto pgfSlider = new Berta::PropertyGridFieldSliderInt("Max Materials", "0");
	pgfSlider->SetMinMax(0, 10);

	categoryMesh.Append(Berta::PropertyGridFieldBasePtr(pgfSlider));

	auto pgfSliderFloat = new Berta::PropertyGridFieldSliderFloat("Max Materials", "0.0");
	pgfSliderFloat->SetMinMax(0, 15.0);

	categoryMesh.Append(Berta::PropertyGridFieldBasePtr(pgfSliderFloat));
	categoryMesh.Append(Berta::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldColor("Color", "0,0,0,255")));
	categoryMesh.Append(Berta::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldStringButton("Mesh file", "")));

	propertyGrid.GetEvents().PropertyChanged.Connect([](const Berta::ArgPropertyGrid& args)
		{
			std::cout << "Property changed! Label = " << args.Property.GetLabel() << ". value = " << args.Property.GetValue() << std::endl;
		});

	NewPanel newPanel(form);

	form.SetLayout("{HorizontalLayout {a}{b}");

	auto& layout = form.GetLayout();
	layout.Attach("a", propertyGrid);
	layout.Attach("b", newPanel);
	layout.Apply();

	form.Show();
	form.Exec();

	return 0;
}
