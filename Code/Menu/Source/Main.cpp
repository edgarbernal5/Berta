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

	Berta::Image cutImage("Resources\\Icons\\Icono9_16.png");
	Berta::Image imageImage("Resources\\Icons\\Image 128.png");
	Berta::Image hddImage("Resources\\Icons\\Hard drive 3 128.png");

	auto& fileMenu = menuBar.PushBack("File");
	fileMenu.Append("New");
	fileMenu.Append("Exit", [](Berta::MenuItem item)
		{
			Berta::GUI::Exit();
		});
	fileMenu.SetImage(0, imageImage);

	auto newSubMenu = std::make_unique<Berta::Menu>();
	newSubMenu->Append("Scene");
	newSubMenu->Append("Texture");
	newSubMenu->AppendSeparator();
	newSubMenu->SetImage(0, hddImage);
	fileMenu.AppendSubMenu(L"New", std::move(newSubMenu));

	auto& editMenu = menuBar.PushBack("Edit");
	editMenu.Append("Undo");
	editMenu.Append("Redo");
	editMenu.AppendSeparator();
	editMenu.Append("Cut");

	editMenu.SetImage(3, cutImage);

	auto& helpMenu = menuBar.PushBack("Help");
	helpMenu.Append("About");

	Berta::Menu popupMenu;
	popupMenu.Append(L"Cut", [](Berta::MenuItem item)
		{
			std::cout << "Context menu click > Cut" << std::endl;
		});
	popupMenu.SetImage(0, cutImage);
	popupMenu.Append(L"Copy", [](Berta::MenuItem item)
		{
			std::cout << "Context menu click > Copy" << std::endl;
		});
	popupMenu.Append(L"Paste", [](Berta::MenuItem item)
		{
			std::cout << "Context menu click > Paste" << std::endl;
		});
	popupMenu.AppendSeparator();
	popupMenu.Append(L"Select", [](Berta::MenuItem item)
		{
			std::cout << "Context menu click > Select" << std::endl;
		});
	auto selectSubMenu = std::make_unique<Berta::Menu>();
	selectSubMenu->Append("All", [](Berta::MenuItem item)
		{
			std::cout << "Context menu click > All" << std::endl;
		});
	selectSubMenu->Append("None", [](Berta::MenuItem item)
		{
			std::cout << "Context menu click > None" << std::endl;
		});
	popupMenu.AppendSubMenu(L"Select", std::move(selectSubMenu));

	form.GetEvents().MouseDown.Connect([&popupMenu, &form](const Berta::ArgMouse& args)
		{
			//popupMenu.ShowPopup(form.Handle(), args);
		});

	form.Show();
	form.Exec();

	return 0;
}