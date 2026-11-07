/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/Button.h>

#include "Berta/Controls/ListBox.h"

int main()
{
	Berta::Form form(Berta::Size(450u, 350u), { true, true, true });
	form.SetCaption("Hello World - Example");

	Berta::Button buttonDisabled(form, { 175,15,150,50 }, "Disabled!");
	buttonDisabled.SetEnabled(false);

	Berta::Button button(form, { 15,15,150,50 }, "Click me!");
	button.GetEvents().Click.Connect([&buttonDisabled](const Berta::ArgClick& args)
	{
		buttonDisabled.SetEnabled(!buttonDisabled.GetEnabled());
	});
	
	Berta::Image image1("..\\..\\Resources\\Icons\\Icono1_16.png");
	
	Berta::Button buttonWithIcon(form, { 50,190,32,32 }, image1);
	
	Berta::Button buttonWithIconAndText(form, { 85,190,80,32 },"Foto", image1);
	
	form.Show();
	form.Exec();

	return 0;
}
