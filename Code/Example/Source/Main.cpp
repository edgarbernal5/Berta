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
#include <Berta/Controls/ThumbListBox.h>
#include <Berta/Controls/ListBox.h>
#include <Berta/Controls/Slider.h>
#include <Berta/Controls/TreeBox.h>
#include <Berta/Controls/CheckBox.h>
#include <iostream>

class ButtonPane : public Berta::Panel
{
public:
	ButtonPane()
	{

	}
};

class TabExample1 : public Berta::Panel
{
public:
	TabExample1(Berta::Window* parent) : Panel(parent)
	{
#ifdef BT_DEBUG
		SetDebugName("Tab Apariencia");
#endif

		m_eraseSelectedbutton.Create(*this, true, { 5,15,50,25 });
		m_eraseSelectedbutton.SetCaption(L"Erase");
#ifdef BT_DEBUG
		m_eraseSelectedbutton.SetDebugName("Erase Tree Button");
#endif
		m_eraseSelectedbutton.GetEvents().Click.Connect([this](const Berta::ArgClick& args)
		{
			auto selected = m_treeBox.GetSelected();
			if (selected.empty())
				return;

			m_treeBox.Erase(selected[0]);
		});

		m_collapseAll.Create(*this, true, { 60,15,80,25 });
		m_collapseAll.SetCaption(L"Collapse all");
#ifdef BT_DEBUG
		m_collapseAll.SetDebugName("Collapse all");
#endif
		m_collapseAll.GetEvents().Click.Connect([this](const Berta::ArgClick& args)
		{
			m_treeBox.CollapseAll();
		});

		m_expandAll.Create(*this, true, { 145,15,80,25 });
		m_expandAll.SetCaption(L"Expand all");
#ifdef BT_DEBUG
		m_expandAll.SetDebugName("Expand all");
#endif
		m_expandAll.GetEvents().Click.Connect([this](const Berta::ArgClick& args)
			{
				m_treeBox.ExpandAll();
			});

		m_treeBox.Create(*this, true, { 5,50,300,200 });
#ifdef BT_DEBUG
		m_treeBox.SetDebugName("Tree box");
#endif
		this->GetEvents().Resize.Connect([this](const Berta::ArgResize& args)
			{
				auto currentPosition = m_treeBox.GetPosition();
				auto margin = m_treeBox.Handle()->ToScale(10);
				m_treeBox.SetSize({ args.NewSize.Width - currentPosition.X - margin, args.NewSize.Height - currentPosition.Y - margin });
			});

		Berta::Image image1("..\\..\\Resources\\Icons\\Icono5_2_16.png");
		Berta::Image image2("..\\..\\Resources\\Icons\\Icono2_16.png");
		Berta::Image image3("..\\..\\Resources\\Icons\\Icono4_16.png");
		Berta::Image imageRed("..\\..\\Resources\\Icons\\Red_16.png");
		m_treeBox.Insert("C:", "C:/").SetIcon(imageRed);
		m_treeBox.Insert("C:/Archivos de programas", "Archivos de programas").SetIcon(imageRed);
		m_treeBox.Insert("C:/Windows", "Windows").SetIcon(imageRed);
		m_treeBox.Insert("C:/Windows/addins", "addins").SetIcon(imageRed);
		m_treeBox.Insert("C:/Windows/appcompat", "appcompat").SetIcon(image3);
		m_treeBox.Insert("C:/Windows/Performance", "Performance").SetIcon(image1);
		m_treeBox.Insert("C:/Windows/Performance/Fast", "Fast");
		m_treeBox.Insert("C:/Windows/Performance/Slow", "Slow");
		m_treeBox.Insert("C:/Windows/Performance/Slow/Records", "Records");
		m_treeBox.Insert("C:/Windows/Performance/Slow/Records/x64", "x64");
		m_treeBox.Insert("C:/Windows/System32", "System32");
		m_treeBox.Insert("C:/Temp", "Temp").SetIcon(image2);
		m_treeBox.Insert("C:/Temp/x64", "x64").SetIcon(image3);
		m_treeBox.Insert("C:/Archivos de programas/Adobe Acrobat", "Adobe Acrobat");
		m_treeBox.Insert("C:/Archivos de programas/Adobe Acrobat/Crack", "Crack");
		m_treeBox.Insert("C:/Archivos de programas/Adobe Acrobat/Temp", "Temp");
		m_treeBox.Insert("C:/Archivos de programas/Office 365", "Office 365");
		m_treeBox.Insert("C:/Archivos de programas/Unity", "Unity");
		m_treeBox.Insert("C:/Archivos de programas/Unity/Editor", "Editor");
		m_treeBox.Insert("C:/Archivos de programas/Unity/Editor/Cache", "Cache");
		m_treeBox.Insert("C:/Archivos de programas/Unity/Editor/Tools", "Tools");
		m_treeBox.Insert("C:/Archivos de programas/Unity/Projects", "Projects");
		m_treeBox.Insert("C:/Archivos de programas/Unity/Tools", "Tools");
		m_treeBox.Insert("C:/Archivos de programas/Visual Studio", "Visual Studio");
		m_treeBox.Insert("C:/Archivos de programas/Visual Studio/Crack", "Crack");
		m_treeBox.Insert("C:/Archivos de programas/Visual Studio/x64", "x64");
		m_treeBox.Insert("C:/Archivos de programas/Visual Studio/Tools", "Tools");
		m_treeBox.Insert("C:/Archivos de programas/NVIDIA Corporation", "NVIDIA Corporation");
		m_treeBox.Insert("C:/Archivos de programas/ScreenToGif", "ScreenToGif");
		m_treeBox.Insert("C:/Archivos de programas/ScreenToGif/Records", "Records");
		m_treeBox.Insert("C:/Archivos de programas/Windows", "Windows");
		m_treeBox.Insert("D:", "D:/");
		m_treeBox.Insert("D:/Juegos", "Juegos");
		m_treeBox.Insert("D:/Juegos/AoE", "AoE");
		m_treeBox.Insert("D:/Juegos/Control", "Control");
		m_treeBox.Insert("D:/Juegos/Cuphead", "Cuphead");
		m_treeBox.Insert("D:/Juegos/Never alone", "Never alone");

		m_treeBox.ExpandAll();

		m_treeBox.GetEvents().Selected.Connect([this](const Berta::ArgTreeBoxSelection& args)
			{
				BT_CORE_TRACE << " - tree box selected event: " << args.Items.size() << std::endl;
			});
	}

	TabExample1() : Panel()
	{
	}

private:
	Berta::Button m_eraseSelectedbutton;
	Berta::Button m_collapseAll;
	Berta::Button m_expandAll;
	Berta::TreeBox m_treeBox;
};

class TabExample2 : public Berta::Panel
{
public:
	TabExample2(Berta::Window* parent) : Panel(parent)
	{
#ifdef BT_DEBUG
		SetDebugName("Tab Player");

		m_buttonClear.Create(*this, true, { 15,10,75,25 });
#ifdef BT_DEBUG
		m_buttonClear.SetDebugName("buttonClear");
#endif
		m_buttonClear.SetCaption(L"Clear");
		m_buttonClear.GetEvents().Click.Connect([this](const Berta::ArgClick& args)
			{
				std::cout << "BUTTON > Clear Click" << std::endl;
				m_listBox.Clear();
			});

		m_buttonErase.Create(*this, true, { 95,10,75,25 });
#ifdef BT_DEBUG
		m_buttonErase.SetDebugName("buttonErase");
#endif
		m_buttonErase.SetCaption(L"Erase");
		m_buttonErase.GetEvents().Click.Connect([this](const Berta::ArgClick& args)
			{
				auto selected = m_listBox.GetSelected();
				if (selected.empty())
					return;

				m_listBox.Erase(selected[0]);
			});

		m_listBox.Create(*this, true, { 15, 38, 200, 200 });
#ifdef BT_DEBUG
		m_listBox.SetDebugName("ListBox");
#endif
		m_listBox.AppendHeader("Nombre completo", 100);
		m_listBox.AppendHeader("Edad", 60);
		m_listBox.AppendHeader("Lugar de Nacimiento", 200);

		m_listBox.Append({ "Edgar Alejandro Bernal Oropeza", "38", "Caracas"});
		m_listBox.Append({ "Bruno Emmanuel Bernal", "3", "Buenos Aires" });
		m_listBox.Append({ "Adriana Desiree", "37", "Caracas" });
		m_listBox.Append({ "Luna Bernal", "0", "..." });
		m_listBox.Append({ "Alicia Mercedes Oropeza de Bernal", "74", "Caracas" });
		m_listBox.Append({ "Toyi Medina", "68", "Caracas" });
		m_listBox.Append({ "Orlando Urdaneta Jimenez", "71", "Caracas" });
		m_listBox.Append({ "Abraham Leonardo Urdaneta", "29", "Caracas" });

		Berta::Image image1("..\\..\\Resources\\Icons\\Icono1_16.png");
		m_listBox.At(0).SetIcon(image1);
		m_listBox.At(4).SetIcon(image1);
		m_listBox.At(2).SetIcon(image1);

		Berta::Image image2("..\\..\\Resources\\Icons\\Icono5_2_16.png");
		m_listBox.At(6).SetIcon(image2);
		this->GetEvents().Resize.Connect([this](const Berta::ArgResize& args)
			{
				auto currentPosition = m_listBox.GetPosition();
				auto margin = m_listBox.Handle()->ToScale(10);
				m_listBox.SetSize({ args.NewSize.Width - currentPosition.X - margin, args.NewSize.Height - currentPosition.Y - margin });
			});
#endif
	}

	TabExample2() : Panel()
	{
	}

private:
	Berta::Button m_buttonClear;
	Berta::Button m_buttonErase;
	Berta::Button m_buttonIcon;
	Berta::ListBox m_listBox;
};

class TabExample3 : public Berta::Panel
{
public:
	TabExample3(Berta::Window* parent) : Panel(parent)
	{
#ifdef BT_DEBUG
		SetDebugName("Tab Input");
#endif

		m_buttonClear.Create(*this, true, { 15,10,75,25 });
#ifdef BT_DEBUG
		m_buttonClear.SetDebugName("buttonClear");
#endif
		m_buttonClear.SetCaption(L"Clear");
		m_buttonClear.GetEvents().Click.Connect([this](const Berta::ArgClick& args)
			{
				std::cout << "BUTTON > Clear Click" << std::endl;
				m_thumbListBox.Clear();
			});

		m_buttonErase.Create(*this, true, { 95,10,75,25 });
#ifdef BT_DEBUG
		m_buttonErase.SetDebugName("buttonErase");
#endif
		m_buttonErase.SetCaption(L"Erase");
		m_buttonErase.GetEvents().Click.Connect([this](const Berta::ArgClick& args)
			{
				m_thumbListBox.Erase(0);
			});


		m_buttonIcon.Create(*this, true, { 175,10,75,25 });
#ifdef BT_DEBUG
		m_buttonIcon.SetDebugName("buttonIcon");
#endif
		m_buttonIcon.SetCaption(L"Icon");
		m_buttonIcon.GetEvents().Click.Connect([this](const Berta::ArgClick& args)
			{
				auto selected = m_thumbListBox.GetSelected();
				if (selected.empty())
					return;

				Berta::Image newIcon("..\\..\\Resources\\Escudo.png");
				m_thumbListBox.At(selected[0]).SetIcon(newIcon);
			});

		m_thumbListBox.Create(*this, true, { 40, 45, 200, 200 });
#ifdef BT_DEBUG
		m_thumbListBox.SetDebugName("thummb list box");
#endif

		Berta::Image image1("..\\..\\Resources\\Icons\\Icono1_16.png");
		for (size_t i = 0; i < 20; i++)
		{
			std::wostringstream builder;
			builder << L"Text example " << i;
			m_thumbListBox.AddItem(builder.str(), image1);
		}

		m_slider.Create(*this, true, { 10,45,18,150 });
		m_slider.SetOrientation(true);
		m_slider.SetMinMax(0, 4);

		m_slider.GetEvents().ValueChanged.Connect([this](const Berta::ArgSlider& args)
			{
				BT_CORE_TRACE << " -- Slider = " << args.Value << std::endl;
				m_thumbListBox.SetThumbnailSize(m_thumbnailSizes[args.Value]);
			});

		this->GetEvents().Resize.Connect([this](const Berta::ArgResize& args)
			{
				auto currentPosition = m_thumbListBox.GetPosition();
				auto margin = m_thumbListBox.Handle()->ToScale(10);
				m_thumbListBox.SetSize({ args.NewSize.Width - currentPosition.X - margin, args.NewSize.Height - currentPosition.Y - margin });
			});

		m_checkBox.Create(*this, true, { 260,10,100,30 });
		m_checkBox.SetCaption(L"Multiselection");

		m_checkBox.GetEvents().CheckedChanged.Connect([this](const Berta::ArgCheckBox& args)
			{
				m_thumbListBox.EnableMultiselection(args.IsChecked);
			});

		m_checkBox.SetChecked(m_thumbListBox.IsEnabledMultiselection());
	}

private:
	Berta::Button m_buttonClear;
	Berta::Button m_buttonErase;
	Berta::Button m_buttonIcon;
	Berta::ThumbListBox m_thumbListBox;
	Berta::Slider m_slider;
	Berta::CheckBox m_checkBox;
	uint32_t m_thumbnailSizes[5]{ 32u, 64u, 96u, 128u, 256u };
};

class TabForm : public Berta::Panel
{
public:
	TabForm(Berta::Window* parent) : Panel(parent)
	{
		m_nestedForm= std::make_unique<Berta::NestedForm>(this->Handle(), Berta::Rectangle{0,0, 200, 200});
		m_nestedForm->GetAppearance().Background = Berta::Color{ 0xAB20CC };

		this->GetEvents().Resize.Connect([this](const Berta::ArgResize& args)
		{
			m_nestedForm->SetArea({ 0, 0, args.NewSize.Width, args.NewSize.Height });
		});

		m_button1.Create(m_nestedForm->Handle(), true, Berta::Rectangle{10,10,140,40});
		m_button1.SetCaption("Nested button");
#ifdef BT_DEBUG
		m_button1.SetDebugName("Nested button");
#endif
		m_nestedForm->Show();
	}

private:
	std::unique_ptr<Berta::NestedForm> m_nestedForm;
	Berta::Button m_button1;
};

int main()
{
	Berta::Form form(Berta::Size(700u, 550u), { true, true, true });
	form.SetCaption(L"Main Window - Example");

	Berta::MenuBar menuBar(form, { 0,0, 100, 25 });
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
//
//	form.GetEvents().Resize.Connect([&menuBar](const Berta::ArgResize& args)
//		{
//			auto currentSize = menuBar.GetSize();
//			menuBar.SetSize({ args.NewSize.Width, currentSize.Height });
//		});
//	menuBar.SetSize({ form.GetSize().Width, menuBar.GetSize().Height });
//
//	Berta::Label label(form, { 10,28,75,35 }, L"Hello world!");
//	label.GetAppearance().Background = Berta::Color{ 0x0000FF };
//
//	label.GetEvents().MouseMove.Connect([](const Berta::ArgMouse& args)
//		{
//			//std::cout << "LABEL>mouse move" << std::endl;
//		});
//
//	Berta::InputText inputText(form, { 110,28,200,25 });
//	inputText.SetCaption(L"Hola edgar como estas espero que estes muy bien vale. saludos");
//	inputText.GetEvents().ValueChanged.Connect([](const Berta::ArgTextChanged& args)
//		{
//			std::cout << "inputText > ValueChanged: " << std::string(args.NewValue.begin(), args.NewValue.end()) << std::endl;
//		});
//
//	Berta::ComboBox comboBox(form, { 110,55,200,25 });
//	for (size_t i = 0; i < 2; i++)
//	{
//		comboBox.PushItem(L"Ejemplo 1", image1);
//		comboBox.PushItem(L"Ejemplo 2");
//		comboBox.PushItem(L"Ejemplo 3", image1);
//		comboBox.PushItem(L"Ejemplo 4", image2);
//		comboBox.PushItem(L"Ejemplo 5", image3);
//		comboBox.PushItem(L"Ejemplo 6");
//		comboBox.PushItem(L"Ejemplo 7", image2);
//	}
//	comboBox.GetEvents().Selected.Connect([](const Berta::ArgComboBox& args)
//		{
//			std::cout << "ComboBox > Selected: " << args.SelectedIndex << std::endl;
//		});
//
//	Berta::ScrollBar scrollbar(form, { 10, 200, 20, 150 }, true);
//	scrollbar.SetMinMax(0, 10);
//	scrollbar.GetEvents().ValueChanged.Connect([](const Berta::ArgScrollBar& args)
//		{
//			std::cout << "scrollbar > ValueChanged: " << args.Value << std::endl;
//		});
//	Berta::ScrollBar scrollbar2(form, { 40,200, 20, 150 }, true);
//	scrollbar2.SetMinMax(0, 0);
//	scrollbar2.SetEnabled(false);
//
//	Berta::TabBar tabbar(form, { 70, 250, 400, 285 });
//
//	auto tabExample1 = tabbar.PushBack<TabExample1>("Apariencia");
//	auto tabExample2 = tabbar.PushBack<TabExample2>("Player");
//	auto tabExample3 = tabbar.Insert<TabExample3>(0, "Input");
//
//
//	Berta::Button button2(form, { 5,120,75,25 }, L"Disabled");
//#ifdef BT_DEBUG
//	button2.SetDebugName("button2");
//#endif
//	button2.SetEnabled(false);
//	button2.GetEvents().Click.Connect([&tabbar](const Berta::ArgClick& args)
//		{
//			std::cout << "BUTTON 2 >Click" << std::endl;
//			tabbar.Erase(1);
//		});
//
//	Berta::Button button(form, { 5,90,75,25 }, L"Click me!");
//#ifdef BT_DEBUG
//	button.SetDebugName("button1");
//#endif
//	button.GetEvents().Click.Connect([&button2 /*, &inputText, &comboBox, &menuBar*/](const Berta::ArgClick& args)
//		{
//			std::cout << "BUTTON > Click" << std::endl;
//			button2.SetEnabled(!button2.GetEnabled());
//			/*inputText.SetEnabled(!inputText.GetEnabled());
//			comboBox.SetEnabled(!comboBox.GetEnabled());
//			menuBar.SetEnabled(!menuBar.GetEnabled());*/
//		});
//	button.GetEvents().MouseLeave.Connect([](const Berta::ArgMouse& args)
//		{
//			std::cout << "BUTTON < mouse leave" << std::endl;
//		});
//	button.GetEvents().MouseEnter.Connect([](const Berta::ArgMouse& args)
//		{
//			std::cout << "BUTTON > mouse enter" << std::endl;
//		});
//
//	Berta::Button buttonClear(form, { 95,90,75,25 }, L"Clear!");
//#ifdef BT_DEBUG
//	buttonClear.SetDebugName("buttonClear");
//#endif
//	buttonClear.GetEvents().Click.Connect([&tabbar](const Berta::ArgClick& args)
//		{
//			std::cout << "BUTTON > Clear Click" << std::endl;
//			tabbar.Clear();
//		});
//
//	form.GetEvents().Visibility.Connect([](const Berta::ArgVisibility& args)
//		{
//			std::cout << "form > Visibility = " << args.IsVisible << std::endl;
//		});
//	tabExample1->GetEvents().Visibility.Connect([](const Berta::ArgVisibility& args)
//		{
//			std::cout << "Apariencia > Visibility = " << args.IsVisible << std::endl;
//		});
//	tabExample2->GetEvents().Visibility.Connect([](const Berta::ArgVisibility& args)
//		{
//			std::cout << "Tab Player > Visibility = " << args.IsVisible << std::endl;
//		});
//	tabExample3->GetEvents().Visibility.Connect([](const Berta::ArgVisibility& args)
//		{
//			std::cout << "Tab Input > Visibility = " << args.IsVisible << std::endl;
//		});
//
	Berta::Menu popupMenu;
	popupMenu.Append(L"Example", [](Berta::MenuItem& item)
		{
			std::cout << "Context menu click > Example" << std::endl;
		});
	popupMenu.Append(L"Example Submenu");
	auto subMenuContext = popupMenu.CreateSubMenu(1);
	subMenuContext->Append(L"hola", [](Berta::MenuItem& item)
		{
			std::cout << "Context sub menu click > hola" << std::endl;
		});
	subMenuContext->Append(L"hola 2");
	auto subsubMenu = subMenuContext->CreateSubMenu(1);
	subsubMenu->Append(L"hola 3", [](Berta::MenuItem& item)
		{
			std::cout << "Context sub menu click > hola 3" << std::endl;
		});
	
	form.GetEvents().MouseDown.Connect([&popupMenu, &form](const Berta::ArgMouse& args)
		{
			popupMenu.ShowPopup(form.Handle(), args);
		});
//
//	auto currentPosition = tabbar.GetPosition();
//	auto margin = tabbar.Handle()->ToScale(2);
//	tabbar.SetSize({ form.GetSize().Width - currentPosition.X - margin, form.GetSize().Height - currentPosition.Y - margin});

	//Berta::Button nestedForm(form, { 320,35, 200, 200 }, L"Button");
	//Berta::Button buttonLayout(form, { 320,250, 200, 200 }, L"Nested");

	/*form.GetEvents().Resize.Connect([&tabbar, &nestedForm](const Berta::ArgResize& args)
		{
			auto currentPosition = tabbar.GetPosition();
			auto margin = tabbar.Handle()->ToScale(2);
			tabbar.SetSize({ args.NewSize.Width - currentPosition.X - margin, args.NewSize.Height - currentPosition.Y - margin });

			currentPosition = nestedForm.GetPosition();
			auto currentSize = nestedForm.GetSize();
			Berta::Size newSize = { args.NewSize.Width - currentPosition.X - margin, currentSize.Height };
			std::cout << " - newSize " << newSize << std::endl;
			nestedForm.SetSize(newSize);
		});*/


//	Berta::Button buttonShowNested(form, { 90,120,75,25 }, L"Show");
//#ifdef BT_DEBUG
//	buttonShowNested.SetDebugName("buttonShowNested");
//#endif
//	buttonShowNested.GetEvents().Click.Connect([&nestedForm](const Berta::ArgClick& args)
//		{
//			nestedForm.Show();
//		});

	/*Berta::Button buttonHideNested(form, { 185,120,75,25 }, L"Hide");
#ifdef BT_DEBUG
	buttonHideNested.SetDebugName("buttonHideNested");
#endif
	buttonHideNested.GetEvents().Click.Connect([&nestedForm](const Berta::ArgClick& args)
		{
			nestedForm.Hide();
		});

	Berta::Button buttonInnerNested(nestedForm, { 10,15,75,25 }, L"Hello");
#ifdef BT_DEBUG
	buttonInnerNested.SetDebugName("buttonInnerNested");
#endif
	buttonInnerNested.GetEvents().Click.Connect([&nestedForm](const Berta::ArgClick& args)
		{
			std::cout << " -- inner nested button!" << std::endl;
		});

	Berta::Button buttonHideScrollbar(form, { 15,150,75,25 }, L"Show or Hide");
#ifdef BT_DEBUG
	buttonHideScrollbar.SetDebugName("buttonHideScrollbar");
#endif
	buttonHideScrollbar.GetEvents().Click.Connect([&scrollbar](const Berta::ArgClick& args)
		{
			if (scrollbar.IsVisible())
				scrollbar.Hide();
			else
				scrollbar.Show();
		});*/

	//form.SetLayout("{{menuBar Height=25}{b}}");
	form.SetLayout("{VerticalLayout {menuBar Height=24}{Dock dockRoot}}");
	
	form.GetLayout().Attach("menuBar", menuBar);

	//auto buttonPane = std::make_shared<ButtonPane>();
	Berta::Button buttonPaneTab(form, { 320,250, 200, 200 }, L"Scene");
	Berta::Button buttonPaneTab2(form, { 320,250, 200, 200 }, L"Properties");
	Berta::Button buttonPaneTab3(form, { 320,250, 200, 200 }, L"Explorer");
	
	//buttonPaneTab.Hide();
	//buttonPaneTab2.Hide();

	//Berta::Button nestedForm(form, { 15,150,75,25 }, L"Show or Hide");

	/*Berta::NestedForm nestedForm(form, { 320,35, 200, 200 });
	nestedForm.GetAppearance().Background = Berta::Color{ 0xAB20CC };*/

	TabForm tabForm(form);

	form.GetLayout().AddPaneTab("dockPane1", "tab-Scene", &buttonPaneTab, "", Berta::DockPosition::Tab);
	form.GetLayout().AddPaneTab("dockPane2", "tab-Properties", &buttonPaneTab2, "dockPane1", Berta::DockPosition::Right);
	form.GetLayout().AddPaneTab("dockPane2", "tab-Explorer", &buttonPaneTab3);
	form.GetLayout().AddPaneTab("dockPane3", "tab-D3D", &tabForm, "dockPane1", Berta::DockPosition::Down);

	//form.GetLayout().Attach("dockRoot", nestedForm);
	form.GetLayout().Apply();

	form.Show();
	//nestedForm.Show();
	form.Exec();

	return 0;
}