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
#include <iostream>

int main()
{
	Berta::Form form(Berta::Size(800u, 600u), { true, true, true });
	form.SetDebugName("form");
	form.SetCaption(L"Window");

	Berta::MenuBar menuBar(form, {0,0, 100, 25});
	menuBar.SetDebugName("menuBar");
	menuBar.PushBack(L"File");
	menuBar.PushBack(L"Edit");
	menuBar.PushBack(L"Help");

	form.GetEvents().Resize.Connect([&menuBar](const Berta::ArgResize& args)
	{
		auto currentSize = menuBar.GetSize();
		menuBar.SetSize({ args.NewSize.Width, currentSize.Height });
	});
	menuBar.SetSize({ form.GetSize().Width, menuBar.GetSize().Height});

	Berta::Label label(form, { 50,35,130,40 }, L"Hello world!");
	label.GetAppearance().Background = Berta::Color{ 0x0000FF };
	label.SetDebugName("Label");
	label.GetEvents().MouseMove.Connect([](const Berta::ArgMouse& args)
	{
		//std::cout << "LABEL>mouse move" << std::endl;
	});

	Berta::Button button2(form, { 5,215,100,40 }, L"Disabled");
	button2.SetDebugName("button2");
	button2.SetEnabled(false);
	button2.GetEvents().Click.Connect([&form](const Berta::ArgClick& args)
	{
		std::cout << "BUTTON 2 >Click" << std::endl;
		//form.SetEnabled(false);
	});
	

	Berta::InputText inputText(form, { 190,55,200,25 });
	inputText.SetCaption(L"Hola edgar como estas espero que estes muy bien vale. saludos");
	inputText.GetEvents().ValueChanged.Connect([](const Berta::ArgTextChanged& args)
		{
			std::cout << "inputText > ValueChanged: " << std::string(args.NewValue.begin(), args.NewValue.end()) << std::endl;
		});

	inputText.SetDebugName("inputText");
	Berta::ComboBox comboBox(form, { 190,85,200,25 });
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

	Berta::ScrollBar scrollbar(form, { 300, 225, 20, 150 }, true);
	scrollbar.SetMinMax(0, 10);
	scrollbar.GetEvents().ValueChanged.Connect([](const Berta::ArgScrollBar& args)
		{
			std::cout << "scrollbar > ValueChanged: " << args.Value << std::endl;
		});
	Berta::ScrollBar scrollbar2(form, { 330, 225, 20, 150 }, true);
	scrollbar2.SetMinMax(0, 0);
	scrollbar2.SetEnabled(false);

	Berta::Button button(form, { 5,165,100,40 }, L"Click me!");
	button.SetDebugName("button");
	button.GetEvents().Click.Connect([&button2, &inputText, &comboBox](const Berta::ArgClick& args)
	{
		std::cout << "BUTTON > Click" << std::endl;
		button2.SetEnabled(!button2.GetEnabled());
		inputText.SetEnabled(!inputText.GetEnabled());
		comboBox.SetEnabled(!comboBox.GetEnabled());
	});
	button.GetEvents().MouseLeave.Connect([](const Berta::ArgMouse& args)
	{
		std::cout << "BUTTON < mouse leave" << std::endl;
	});
	button.GetEvents().MouseEnter.Connect([](const Berta::ArgMouse& args)
	{
		std::cout << "BUTTON > mouse enter" << std::endl;
	});

	form.Show();
	form.Exec();

	return 0;
}