/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/PropertyGrid.h>
#include <Berta/Controls/TextBox.h>
#include <Berta/Controls/Button.h>
#include <Berta/Controls/Panel.h>

#include <Berta/Controls/Properties/PropertyGridFields.h>

class NewPanel : public Berta::Panel
{
public:
	NewPanel(Berta::Window* parent) : Berta::Panel(parent)
	{
		m_textBox.Create(*this, true, {20,20,200,40});
		m_textBox.SetCaption("Two words!");
	}

private:
	Berta::TextBox m_textBox;
};

enum class MaterialTypeEnum
{
	Opaque,
	Transparent,
	Additive
};

struct Vector3
{
	float x {0.0f};
	float y {0.0f};
	float z {0.0f};
};

struct AppState 
{
	std::wstring CarName{ L"Car" };
	std::wstring Tags{ L"Blue, Green" };
	int EnginePower{ 5 };
	float MaxMaterials{ 0.0f };
	std::wstring HashMaterial0{ L"f0a0c85cd9b5035fe600d8a56f6ba79897f032ee" };
	bool CastShadows{ true };
	bool Static{ false };
	MaterialTypeEnum MaterialType { MaterialTypeEnum::Additive };
	
	std::wstring MeshFilter{ L"/home/edgar/meshfilter.x" };
	Berta::Color TintColor{255,0,0,255};
	float Roughness=255.0f;
	int Threshold=3;
	Vector3 Position;
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
	
	auto transformCategory = propertyGrid.Append("Transform");
	transformCategory.EmplaceVector3(transformCategory, "Position", 
		[&myApp]()
		{
			Berta::OptionalVector3 opt;
			opt.x = myApp.Position.x;
			opt.y = myApp.Position.y;
			opt.z = myApp.Position.z;
			return opt;
		},
		[&myApp](const Berta::OptionalVector3& val)
		{
			if (val.x.has_value())
				myApp.Position.x = val.x.value();
			
			if (val.y.has_value())
				myApp.Position.y = val.y.value();
			
			if (val.z.has_value())
				myApp.Position.z = val.z.value();
		});
	
	auto generalCategory = propertyGrid.Append("General");
	generalCategory.EmplaceProperty<Berta::PropertyGridFieldString>
		(
			"Name",
			[&myApp]() { return myApp.CarName; },
			[&myApp](const std::wstring& val) { myApp.CarName = val; }
		);
	
	generalCategory.SetIcon(m_hardDriveImg);
	
	generalCategory.EmplaceProperty<Berta::PropertyGridFieldString>
		(
			"Tags",
			[&myApp]() { return myApp.Tags; },
			[&myApp](const std::wstring& val) { myApp.Tags = val; }
		);
	generalCategory.EmplaceProperty<Berta::PropertyGridFieldInt>
		(
			"Engine power",
			[&myApp]() { return myApp.EnginePower; },
			[&myApp](int val) { myApp.EnginePower = val; }
		);
	
	generalCategory.EmplaceProperty<Berta::PropertyGridFieldFloat>
		(
			"Max materials",
			[&myApp]() { return myApp.MaxMaterials; },
			[&myApp](float val) { myApp.MaxMaterials = val; }
		);
	
	propertyGrid.Append("Empty");
	auto categoryMesh = propertyGrid.Append("Mesh").SetIcon(m_folderOpenImg);
	
	auto subcategoryMaterials = categoryMesh.AppendSubCategory("Materials");
	subcategoryMaterials.EmplaceProperty<Berta::PropertyGridFieldString>
		(
			"Hash", 
			[&myApp]() { return myApp.HashMaterial0; },
			[&myApp](const std::wstring& val) {  }
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
			
			std::vector<std::pair<std::wstring, MaterialTypeEnum>> {
				{ L"Opaque",      MaterialTypeEnum::Opaque },
				{ L"Transparent", MaterialTypeEnum::Transparent },
				{ L"Additive",    MaterialTypeEnum::Additive }
			}
		);
	
	auto selection = materialTypeSelection.As<Berta::PropertyGridFieldSelection<MaterialTypeEnum>>();
	//selection.set
	
	categoryMesh.EmplaceProperty<Berta::PropertyGridFieldStringButton>
		(
			"Mesh Filter", 
			[&myApp]() { return myApp.MeshFilter; },
			[&myApp](const std::wstring& val) { myApp.MeshFilter = val; },
			[](std::optional<std::wstring> currentValue) -> std::optional<std::wstring>
			{
				std::cout << "Opening file explorer...." << std::endl;
				std::wstring newPath = L"/home/new_path/filefilter.x";
				std::cout << "newPath = " << Berta::StringUtils::WideToUTF8(newPath) << std::endl;
				return newPath;
			});
	
	subcategoryMaterials.EmplaceProperty<Berta::PropertyGridFieldColor>(
		"Tint Color",
		[&myApp]() { return myApp.TintColor; },
			[&myApp](Berta::Color val) { myApp.TintColor = val; },
			[](std::optional<Berta::Color> currentColor) -> std::optional<Berta::Color>
			{
				std::cout << "Opening color picker...." << std::endl;
				std::cout << "ERROR...." << std::endl;

				return std::nullopt;
			});
	
	subcategoryMaterials.EmplaceProperty<Berta::PropertyGridFieldSliderFloat>(
		"Roughness",
		[&myApp]() { return myApp.Roughness; },
			[&myApp](float val) { myApp.Roughness = val; },
			0.0f, 255.0f);
	
	subcategoryMaterials.EmplaceProperty<Berta::PropertyGridFieldSliderInt>(
		"Threshold",
		[&myApp]() { return myApp.Threshold; },
			[&myApp](int val) { myApp.Threshold = val; },
			-5, 5);
	
	propertyGrid.GetEvents().PropertyChanged.Connect([](const Berta::ArgPropertyGrid& args)
		{
			std::cout << "Property changed! Label = " << args.Property.GetLabel() << ". Value = " << Berta::StringUtils::WideToUTF8(args.Property.GetValueAsString()) << std::endl;
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
