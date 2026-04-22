/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/PropertyGrid.h>
#include <Berta/Controls/InputText.h>
#include <Berta/Controls/Button.h>

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

enum class MaterialTypeEnum
{
	Opaque,
	Transparent,
	Additive
};
struct AppState 
{
	std::string CarName{ "Car" };
	std::string Tags{ "Blue, Green" };
	int EnginePower{ 5 };
	float MaxMaterials{ 0.0f };
	std::string HashMaterial0{ "f0a0c85cd9b5035fe600d8a56f6ba79897f032ee" };
	bool CastShadows{ true };
	bool Static{ false };
	MaterialTypeEnum MaterialType { MaterialTypeEnum::Additive };
	// ...
};

int main()
{
	Berta::Form form(Berta::Size(750u, 650u), { true, true, true });
	form.SetCaption("Property Grid - Example");

	Berta::Image m_folderOpenImg{ "..\\..\\Resources\\Icons\\Folder 128.png" };
	Berta::Image m_fileImg{ "..\\..\\Resources\\Icons\\File 128.png" };
	Berta::Image m_hardDriveImg{ "..\\..\\Resources\\Icons\\Hard drive 3 128.png" };

	
	AppState myApp;
	Berta::PropertyGrid propertyGrid(form, { 15,15,280,600 });
	propertyGrid.ShowCategoryIcons(true);
	
	auto categoryTransform = propertyGrid.Append("Transform");
	categoryTransform.EmplaceProperty<Berta::PropertyGridFieldString>
		(
			"Name",
			[&myApp]() { return myApp.CarName; },
			[&myApp](const std::string& val) { myApp.CarName = val; }
		);
	
	categoryTransform.SetIcon(m_hardDriveImg);
	
	categoryTransform.EmplaceProperty<Berta::PropertyGridFieldString>
		(
			"Tags",
			[&myApp]() { return myApp.Tags; },
			[&myApp](const std::string& val) { myApp.Tags = val; }
		);
	categoryTransform.EmplaceProperty<Berta::PropertyGridFieldInt>
		(
			"Engine power",
			[&myApp]() { return myApp.EnginePower; },
			[&myApp](int val) { myApp.EnginePower = val; }
		);
	
	categoryTransform.EmplaceProperty<Berta::PropertyGridFieldFloat>
		(
			"Max materials",
			[&myApp]() { return myApp.MaxMaterials; },
			[&myApp](float val) { myApp.MaxMaterials = val; }
		);
	
	auto categoryEmpty = propertyGrid.Append("Empty");
	auto categoryMesh = propertyGrid.Append("Mesh").SetIcon(m_folderOpenImg);
	
	auto subcategoryMaterials = categoryMesh.AppendSubCategory("Materials");
	subcategoryMaterials.EmplaceProperty<Berta::PropertyGridFieldString>
		(
			"Hash", 
			[&myApp]() { return myApp.HashMaterial0; },
			[&myApp](const std::string& val) {  }
		);
	
	categoryMesh.EmplaceProperty<Berta::PropertyGridFieldCheck>
		(
			"Cast Shadows", 
			[&myApp]() { return myApp.CastShadows; },
			[&myApp](const bool& val) { myApp.CastShadows = val;  }
			);
	
	categoryMesh.EmplaceProperty<Berta::PropertyGridFieldCheck>
		(
			"Static", 
			[&myApp]() { return myApp.Static; },
			[&myApp](const bool& val) { myApp.Static = val;  }
			);
	
	auto materialTypeSelection = categoryMesh.EmplaceProperty<Berta::PropertyGridFieldSelection<MaterialTypeEnum>>
		(
			"Type", 
			[&myApp]() { return myApp.MaterialType; },
			[&myApp](MaterialTypeEnum val) { myApp.MaterialType = val;  },
			
			std::vector<std::pair<std::string, MaterialTypeEnum>> {
				{ "Opaque",      MaterialTypeEnum::Opaque },
				{ "Transparent", MaterialTypeEnum::Transparent },
				{ "Additive",    MaterialTypeEnum::Additive }
			}
		);
	
	auto selection = materialTypeSelection.As<Berta::PropertyGridFieldSelection<MaterialTypeEnum>>();
	//selection.set
	/*for (size_t i = 0; i < 3; i++)
	{
		categoryTransform.Append(Berta::PropertyGrid::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldVector3("Position", "0.0/0.0/0.0")));
	}

	auto categoryEmpty = propertyGrid.Append("Empty");
	auto categoryMesh = propertyGrid.Append("Mesh");

	auto meshIdProp = new Berta::PropertyGridFieldString("Mesh ID", "71d3eed6-d363-428a-bc81-01576539b297");
	
	categoryMesh.Append(Berta::PropertyGrid::PropertyGridFieldBasePtr(meshIdProp)); meshIdProp->SetEditable(false);

	categoryMesh.Append(Berta::PropertyGrid::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldStringInt("Mesh Count", "0")));
	categoryMesh.Append(Berta::PropertyGrid::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldCheck("Enabled", "0")));
	std::vector<std::string> options{"None", "Material 1", "Material 2" };
	auto pgfSelection = new Berta::PropertyGridFieldSelection("Material");
	pgfSelection->Set(options);
	categoryMesh.Append(Berta::PropertyGrid::PropertyGridFieldBasePtr(pgfSelection));

	auto pgfSlider = new Berta::PropertyGridFieldSliderInt("Max Materials", "0");
	pgfSlider->SetMinMax(0, 10);

	categoryMesh.Append(Berta::PropertyGrid::PropertyGridFieldBasePtr(pgfSlider));

	auto pgfSliderFloat = new Berta::PropertyGridFieldSliderFloat("Max Materials Float", "0.0");
	pgfSliderFloat->SetMinMax(0, 15.0);

	categoryMesh.Append(Berta::PropertyGrid::PropertyGridFieldBasePtr(pgfSliderFloat));
	categoryMesh.Append(Berta::PropertyGrid::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldColor("Color", "0,0,0,255")));
	categoryMesh.Append(Berta::PropertyGrid::PropertyGridFieldBasePtr(new Berta::PropertyGridFieldStringButton("Mesh file", "")));
	*/
	propertyGrid.GetEvents().PropertyChanged.Connect([](const Berta::ArgPropertyGrid& args)
		{
			std::cout << "Property changed! Label = " << args.Property.GetLabel() << ". Value = " << args.Property.GetValueAsString() << std::endl;
		});


	propertyGrid.GetEvents().SelectionChanged.Connect([](const Berta::ArgPropertyGrid& args)
		{
			std::cout << "Selection changed! Label = " << args.Property.GetLabel() << std::endl;
		});

	Berta::Button buttonClear(form, {15,15,120,35}, "Clear");
	buttonClear.GetEvents().Click.Connect([&propertyGrid](const Berta::ArgClick& args)
	{
		propertyGrid.Clear();
	});
	
	NewPanel newPanel(form);

	form.SetLayout("{HorizontalLayout {VerticalLayout {VerticalLayout {e}{a}}{VerticalLayout {d}{c}}}{b}");

	auto& layout = form.GetLayout();
	layout.Attach("a", propertyGrid);
	layout.Attach("b", newPanel);
	layout.Attach("c", buttonClear);
	layout.Apply();

	form.Show();
	form.Exec();

	return 0;
}
