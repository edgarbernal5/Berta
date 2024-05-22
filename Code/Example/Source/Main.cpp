/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/Label.h>
#include <Berta/Controls/Button.h>
#include <Berta/Controls/InputText.h>
#include <iostream>

int main()
{
	Berta::Form form({ 0,0,800,600 }, { true, true, true });
	form.Caption(L"Window");
	
	Berta::Label label(form, { 50,10,130,40 }, L"Hello world!");
	label.GetAppearance().Background = Berta::Color{ 0x0000FF };

	label.GetEvents().MouseMove.Connect([](const Berta::ArgMouse& args)
	{
		//std::cout << "LABEL>mouse move" << std::endl;
	});
	
	Berta::Button button(form, { 0,140,100,40 }, L"Click me!");
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
	form.Show();
	form.Exec();

	return 0;
}