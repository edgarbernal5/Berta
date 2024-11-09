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
#include <iostream>

class TabExample1 : public Berta::Panel
{
public:
	TabExample1(Berta::Window* parent) : Panel(parent)
	{
#ifdef BT_DEBUG
		SetDebugName("Tab Apariencia");
#endif

		m_button.Create(*this, true, { 5,35,100,40 });
		m_button.SetCaption(L"Click me on tab!");
#ifdef BT_DEBUG
		m_button.SetDebugName("button tab panel 1");
#endif
		m_button.GetEvents().Click.Connect([](const Berta::ArgClick& args)
		{
			std::cout << "CLICK button tab panel 1" << std::endl;
		});
	}

	TabExample1() : Panel()
	{
	}

private:
	Berta::Button m_button;
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
				m_listBox.Erase(0);
			});

		m_listBox.Create(*this, true, { 15, 38, 200, 200 });

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
				m_listBox.SetSize({ args.NewSize.Width - currentPosition.X - 10, args.NewSize.Height - currentPosition.Y - 10 });
			});
#endif
	}

	TabExample2() : Panel()
	{
	}

private:
	Berta::Button m_buttonClear;
	Berta::Button m_buttonErase;
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

		m_thumbListBox.Create(*this, true, { 15, 45, 200, 200 });
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

		this->GetEvents().Resize.Connect([this](const Berta::ArgResize& args)
			{
				auto currentPosition = m_thumbListBox.GetPosition();
				m_thumbListBox.SetSize({ args.NewSize.Width - currentPosition.X - 10, args.NewSize.Height - currentPosition.Y - 10 });
			});
	}

private:
	Berta::Button m_buttonClear;
	Berta::Button m_buttonErase;
	Berta::ThumbListBox m_thumbListBox;
};

int main()
{
	Berta::Form form(Berta::Size(500u, 450u), { true, true, true });
	form.SetCaption(L"Window");

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

	form.GetEvents().Resize.Connect([&menuBar](const Berta::ArgResize& args)
		{
			auto currentSize = menuBar.GetSize();
			menuBar.SetSize({ args.NewSize.Width, currentSize.Height });
		});
	menuBar.SetSize({ form.GetSize().Width, menuBar.GetSize().Height });

	Berta::Label label(form, { 50,35,105,45 }, L"Hello world!");
	label.GetAppearance().Background = Berta::Color{ 0x0000FF };

	label.GetEvents().MouseMove.Connect([](const Berta::ArgMouse& args)
		{
			//std::cout << "LABEL>mouse move" << std::endl;
		});

	Berta::InputText inputText(form, { 190,35,200,25 });
	inputText.SetCaption(L"Hola edgar como estas espero que estes muy bien vale. saludos");
	inputText.GetEvents().ValueChanged.Connect([](const Berta::ArgTextChanged& args)
		{
			std::cout << "inputText > ValueChanged: " << std::string(args.NewValue.begin(), args.NewValue.end()) << std::endl;
		});

	Berta::ComboBox comboBox(form, { 190,65,200,25 });
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
	comboBox.GetEvents().Selected.Connect([](const Berta::ArgComboBox& args)
		{
			std::cout << "ComboBox > Selected: " << args.SelectedIndex << std::endl;
		});

	Berta::ScrollBar scrollbar(form, { 10, 200, 20, 150 }, true);
	scrollbar.SetMinMax(0, 10);
	scrollbar.GetEvents().ValueChanged.Connect([](const Berta::ArgScrollBar& args)
		{
			std::cout << "scrollbar > ValueChanged: " << args.Value << std::endl;
		});
	Berta::ScrollBar scrollbar2(form, { 40,200, 20, 150 }, true);
	scrollbar2.SetMinMax(0, 0);
	scrollbar2.SetEnabled(false);

	Berta::TabBar tabbar(form, { 70, 150, 400, 285 });

	auto tabExample1 = tabbar.PushBack<TabExample1>("Apariencia");
	auto tabExample2 = tabbar.PushBack<TabExample2>("Player");
	auto tabExample3 = tabbar.Insert<TabExample3>(0, "Input");

	form.GetEvents().Resize.Connect([&tabbar](const Berta::ArgResize& args)
		{
			auto currentPosition = tabbar.GetPosition();
			tabbar.SetSize({ args.NewSize.Width - currentPosition.X - 2, args.NewSize.Height - currentPosition.Y - 2 });
		});

	Berta::Button button2(form, { 5,120,75,25 }, L"Disabled");
#ifdef BT_DEBUG
	button2.SetDebugName("button2");
#endif
	button2.SetEnabled(false);
	button2.GetEvents().Click.Connect([&tabbar](const Berta::ArgClick& args)
		{
			std::cout << "BUTTON 2 >Click" << std::endl;
			tabbar.Erase(1);
		});

	Berta::Button button(form, { 5,90,75,25 }, L"Click me!");
#ifdef BT_DEBUG
	button.SetDebugName("button1");
#endif
	button.GetEvents().Click.Connect([&button2 /*, &inputText, &comboBox, &menuBar*/](const Berta::ArgClick& args)
		{
			std::cout << "BUTTON > Click" << std::endl;
			button2.SetEnabled(!button2.GetEnabled());
			/*inputText.SetEnabled(!inputText.GetEnabled());
			comboBox.SetEnabled(!comboBox.GetEnabled());
			menuBar.SetEnabled(!menuBar.GetEnabled());*/
		});
	button.GetEvents().MouseLeave.Connect([](const Berta::ArgMouse& args)
		{
			std::cout << "BUTTON < mouse leave" << std::endl;
		});
	button.GetEvents().MouseEnter.Connect([](const Berta::ArgMouse& args)
		{
			std::cout << "BUTTON > mouse enter" << std::endl;
		});

	Berta::Button buttonClear(form, { 95,90,75,25 }, L"Clear!");
#ifdef BT_DEBUG
	buttonClear.SetDebugName("buttonClear");
#endif
	buttonClear.GetEvents().Click.Connect([&tabbar](const Berta::ArgClick& args)
		{
			std::cout << "BUTTON > Clear Click" << std::endl;
			tabbar.Clear();
		});

	form.GetEvents().Visibility.Connect([](const Berta::ArgVisibility& args)
		{
			std::cout << "form > Visibility = " << args.IsVisible << std::endl;
		});
	tabExample1->GetEvents().Visibility.Connect([](const Berta::ArgVisibility& args)
		{
			std::cout << "Apariencia > Visibility = " << args.IsVisible << std::endl;
		});
	tabExample2->GetEvents().Visibility.Connect([](const Berta::ArgVisibility& args)
		{
			std::cout << "Tab Player > Visibility = " << args.IsVisible << std::endl;
		});
	tabExample3->GetEvents().Visibility.Connect([](const Berta::ArgVisibility& args)
		{
			std::cout << "Tab Input > Visibility = " << args.IsVisible << std::endl;
		});

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

	form.Show();
	form.Exec();

	return 0;
}