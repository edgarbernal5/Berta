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
#include <iostream>

int main()
{
	Berta::Form form(Berta::Size(800u, 600u), { true, true, true });
	form.SetDebugName("form");
	form.Caption(L"Window");
	
	Berta::Label label(form, { 50,10,130,40 }, L"Hello world!");
	label.GetAppearance().Background = Berta::Color{ 0x0000FF };
	label.SetDebugName("Label");
	label.GetEvents().MouseMove.Connect([](const Berta::ArgMouse& args)
	{
		//std::cout << "LABEL>mouse move" << std::endl;
	});
	
	Berta::Button button(form, { 0,140,100,40 }, L"Click me!");
	button.SetDebugName("button");
	button.GetEvents().Click.Connect([](const Berta::ArgClick& args)
	{
		std::cout << "BUTTON>Click" << std::endl;
	});
	
	button.GetEvents().MouseLeave.Connect([](const Berta::ArgMouse& args)
	{
		std::cout << "BUTTON < mouse leave" << std::endl;
	});
	button.GetEvents().MouseEnter.Connect([](const Berta::ArgMouse& args)
	{
		std::cout << "BUTTON > mouse enter" << std::endl;
	});

	Berta::InputText inputText(form, { 190,30,200,25 });
	inputText.Caption(L"Hola edgar como estas espero que estes muy bien vale. saludos");

	inputText.SetDebugName("inputText");
	Berta::ComboBox comboBox(form, { 190,60,200,25 });
	comboBox.PushItem(L"Ejemplo 1");
	comboBox.PushItem(L"Ejemplo 2");
	comboBox.SetDebugName("comboBox");

	form.Show();
	form.Exec();

	return 0;
}