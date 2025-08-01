/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/MenuBar.h>

int main()
{
	Berta::Form form(Berta::Size(450u, 350u), { true, true, true });
	form.SetCaption("Menu - Example");

	Berta::MenuBar menuBar(form, { 15,15,200,45 });

	auto& fileMenu = menuBar.PushBack("File");
	fileMenu.Append("New");
	fileMenu.Append("Exit", [](Berta::MenuItem& item)
		{
			Berta::GUI::Exit();
		});

	form.Show();
	form.Exec();

	return 0;
}