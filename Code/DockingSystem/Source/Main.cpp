/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/Button.h>
#include <Berta/Controls/MenuBar.h>

#include <iostream>

int main()
{
	Berta::Form form(Berta::Size(700u, 450u), { true, true, true });
	form.SetCaption(L"Docking system - Example");

	Berta::MenuBar menuBar(form, { 0,0, 100, 25 });
	auto& menuFile = menuBar.PushBack(L"File");
	menuFile.Append("Exit", [](Berta::MenuItem& item)
		{
			std::cout << "EXIT()..." << std::endl;
		});

	Berta::Button buttonPaneScene(form, { 320,250, 200, 200 }, "Scene");
	Berta::Button buttonPanePropierties(form, { 320,250, 200, 200 }, "Properties");
	Berta::Button buttonPaneExplorer(form, { 320,250, 200, 200 }, "Explorer");

	form.SetLayout("{VerticalLayout {menuBar Height=24}{Dock dockRoot}}");

	auto& layout = form.GetLayout();
	layout.Attach("menuBar", menuBar);

	layout.AddPaneTab("dockScene", "tab-Scene", &buttonPaneScene, "", Berta::DockPosition::Tab);
	layout.AddPaneTab("dockProp", "tab-Properties", &buttonPanePropierties, "dockScene", Berta::DockPosition::Right);
	layout.AddPaneTab("dockProp", "tab-Explorer", &buttonPaneExplorer);

	layout.Apply();

	form.Show();
	form.Exec();

	return 0;
}