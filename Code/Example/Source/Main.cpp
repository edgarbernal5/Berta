/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Widgets/Form.h>
#include <Berta/Widgets/Label.h>
#include <iostream>

int main()
{
	Berta::Form form({ 0,0,800,600 }, { true, false, true });
	form.Caption(L"Window");
	
	Berta::Label label(form, { 50,30,200,80 }, L"Hello world!");
	label.GetAppearance().Background = Berta::Color{ 0x0000FF };

	label.GetEvents().MouseMove.connect([](const Berta::ArgMouse& args)
	{
		//std::cout << "LABEL>mouse move" << std::endl;
	});
	
	label.GetEvents().Click.connect([](const Berta::ArgClick& args)
	{
		//std::cout << "LABEL>Click" << std::endl;
	});
	form.Show();
	form.Exec();

	return 0;
}