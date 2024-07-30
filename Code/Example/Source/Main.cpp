/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/Label.h>
#include <Berta/Controls/Button.h>
#include <Berta/Controls/InputText.h>
#include <Berta/Controls/ComboBox.h>
#include <Berta/Controls/ScrollBar.h>
#include <Berta/Controls/MenuBar.h>
#include <Berta/Controls/Panel.h>
#include <Berta/Controls/TabBar.h>
#include <iostream>

class TabExample : public Berta::Panel
{
public:
	TabExample(Berta::Window* parent) : Panel(parent) {}
	TabExample() : Panel()
	{

	}
};
int main()
{
	Berta::Form form(Berta::Size(500u, 450u), { true, true, true });
	form.SetDebugName("form");
	form.SetCaption(L"Window");

	Berta::MenuBar menuBar(form, {0,0, 100, 25});
	menuBar.SetDebugName("menuBar");
	auto& menuFile = menuBar.PushBack(L"File");
	menuFile.Append(L"New", [](Berta::MenuItem& item) {});
	menuFile.Append(L"Open file...", [](Berta::MenuItem& item) {});
	menuFile.AppendSeparator();
	menuFile.Append(L"Exit", [](Berta::MenuItem& item)
	{
		std::cout << "EXITO" << std::endl;
	});
	Berta::Image image1("..\\..\\Resources\\Icons\\Icono1_16.png");
	menuFile.SetEnabled(1, false);
	menuFile.SetImage(0, image1);

	Berta::Image image2("..\\..\\Resources\\Icons\\Icono5_2_16.png");
	menuFile.SetImage(1, image2);
	menuFile.SetEnabled(1, false);

	Berta::Image image3("..\\..\\Resources\\Icons\\Icono5_2_16.png");
	menuFile.SetImage(3, image3);

	auto newSubmenu = menuFile.CreateSubMenu(0);
	newSubmenu->Append(L"Texture", [](Berta::MenuItem& item) {});
	newSubmenu->Append(L"Scene", [](Berta::MenuItem& item) {});
	newSubmenu->Append(L"Complex", [](Berta::MenuItem& item) {});
	auto complexSubMenu = newSubmenu->CreateSubMenu(2);
	newSubmenu->CreateSubMenu(1);
	//newSubmenu->SetEnabled(1, false);
	complexSubMenu->Append(L"Complex 1", [](Berta::MenuItem& item) {});

	auto& menuEdit = menuBar.PushBack(L"Edit");
	menuEdit.Append(L"Undo", [](Berta::MenuItem& item) {});

	menuBar.PushBack(L"Help");

	form.GetEvents().Resize.Connect([&menuBar](const Berta::ArgResize& args)
	{
		auto currentSize = menuBar.GetSize();
		menuBar.SetSize({ args.NewSize.Width, currentSize.Height });
	});
	menuBar.SetSize({ form.GetSize().Width, menuBar.GetSize().Height});

	Berta::Label label(form, { 50,35,105,90 }, L"Hello world!");
	label.GetAppearance().Background = Berta::Color{ 0x0000FF };
	label.SetDebugName("Label");
	label.GetEvents().MouseMove.Connect([](const Berta::ArgMouse& args)
	{
		//std::cout << "LABEL>mouse move" << std::endl;
	});

	Berta::Button button2(form, { 5,185,100,40 }, L"Disabled");
	button2.SetDebugName("button2");
	button2.SetEnabled(false);
	button2.GetEvents().Click.Connect([&form](const Berta::ArgClick& args)
	{
		std::cout << "BUTTON 2 >Click" << std::endl;
		//form.SetEnabled(false);
	});
	

	Berta::InputText inputText(form, { 190,35,200,25 });
	inputText.SetCaption(L"Hola edgar como estas espero que estes muy bien vale. saludos");
	inputText.GetEvents().ValueChanged.Connect([](const Berta::ArgTextChanged& args)
	{
		std::cout << "inputText > ValueChanged: " << std::string(args.NewValue.begin(), args.NewValue.end()) << std::endl;
	});

	inputText.SetDebugName("inputText");
	Berta::ComboBox comboBox(form, { 190,65,200,25 });
	for (size_t i = 0; i < 2; i++)
	{
		comboBox.PushItem(L"Ejemplo 1");
		comboBox.PushItem(L"Ejemplo 2");
		comboBox.PushItem(L"Ejemplo 3");
		comboBox.PushItem(L"Ejemplo 4");
		comboBox.PushItem(L"Ejemplo 5");
		comboBox.PushItem(L"Ejemplo 6");
		comboBox.PushItem(L"Ejemplo 7");
	}
	comboBox.SetDebugName("comboBox");
	comboBox.GetEvents().Selected.Connect([](const Berta::ArgComboBox& args)
	{
		std::cout << "ComboBox > Selected: " << args.SelectedIndex << std::endl;
	});

	Berta::ScrollBar scrollbar(form, { 10, 250, 20, 150 }, true);
	scrollbar.SetMinMax(0, 10);
	scrollbar.GetEvents().ValueChanged.Connect([](const Berta::ArgScrollBar& args)
		{
			std::cout << "scrollbar > ValueChanged: " << args.Value << std::endl;
		});
	Berta::ScrollBar scrollbar2(form, { 40,250, 20, 150 }, true);
	scrollbar2.SetMinMax(0, 0);
	scrollbar2.SetEnabled(false);

	Berta::Button button(form, { 5,135,100,40 }, L"Click me!");
	button.SetDebugName("button");
	button.GetEvents().Click.Connect([&button2, &inputText, &comboBox, &menuBar](const Berta::ArgClick& args)
	{
		std::cout << "BUTTON > Click" << std::endl;
		button2.SetEnabled(!button2.GetEnabled());
		inputText.SetEnabled(!inputText.GetEnabled());
		comboBox.SetEnabled(!comboBox.GetEnabled());
		menuBar.SetEnabled(!menuBar.GetEnabled());
	});
	button.GetEvents().MouseLeave.Connect([](const Berta::ArgMouse& args)
	{
		std::cout << "BUTTON < mouse leave" << std::endl;
	});
	button.GetEvents().MouseEnter.Connect([](const Berta::ArgMouse& args)
	{
		std::cout << "BUTTON > mouse enter" << std::endl;
	});

	//Berta::TabBar tabbar(form, { 70, 230, 400, 180 });
	//TabExample tab1;
	//auto tabExample1 = tabbar.PushBack<TabExample>("Tab1");
	//auto tabExample2 = tabbar.PushBack<TabExample>("Tab2");
	//auto tabExample3 = tabbar.PushBack<TabExample>("Tab3");

	form.Show();
	form.Exec();

	return 0;
}