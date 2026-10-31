/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/Button.h>

#include "Berta/Controls/ListBox.h"
#include "Berta/Controls/TextAreaBox.h"

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
	
	std::wstring textLong = L"Hola. Este es un texto muy largo que probaré durante el desarrollo de un editor de texto que estoy creando junto a GEMINI. Debe haber errores, se supone que los estar[e solucionando lo mas pronto posible. Estoy tratando de mejorar ciertas caracteristicas y agregar nuevas funcionalidades. No tengo más nada que decir, pero escribiré muchas cosas con acentos y un texto largo vacío sin sentido solo para alcanzar el máximo de caracteres posibles de Mercadolibre y dejar una buena impresión sin impresora. Gracias";
	
	Berta::TextAreaBox inputTextWordWrap(form, { 50,125,250,95 });
	inputTextWordWrap.SetCaption(textLong);
	inputTextWordWrap.SetWordWrap(true);
	
	form.Show();
	form.Exec();

	return 0;
}
