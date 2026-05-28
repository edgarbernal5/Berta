/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/Button.h>
#include <Berta/Controls/MenuBar.h>
#include <Berta/Controls/Panel.h>

#include <iostream>

class TabProperties : public Berta::Panel
{
public:
	TabProperties(Berta::Window* parent) :
		Panel(parent)
	{
		m_buttonGrid.Create(*this, true, Berta::Rectangle{ 10,10,140,40 });
		m_buttonGrid.SetCaption("Grid");
#ifdef BT_DEBUG
		m_buttonGrid.SetDebugName("Button Grid");
#endif

		m_buttonValues.Create(*this, true, Berta::Rectangle{ 10,10,140,40 });
		m_buttonValues.SetCaption("Values");
#ifdef BT_DEBUG
		m_buttonValues.SetDebugName("Button Values");
#endif

		m_layout.Create(*this);
		m_layout.Parse("{{grid}{values}}");

		m_layout.Attach("grid", m_buttonGrid);
		m_layout.Attach("values", m_buttonValues);
		m_layout.Apply();
	}
	~TabProperties() override
	{
	}

private:
	Berta::Layout m_layout;
	Berta::Button m_buttonGrid;
	Berta::Button m_buttonValues;
};

class TabForm : public Berta::Panel
{
public:
	TabForm(Berta::Window* parent) :
		Panel(parent)
	{
		m_nestedForm = std::make_unique<Berta::NestedForm>(this->Handle(), Berta::Rectangle{ 0,0, 200, 200 }, Berta::FormStyle::Flat());
		m_nestedForm->GetAppearance().Background = Berta::Color(0xFFFF0000);

		this->GetEvents().Resize.Connect([this](const Berta::ArgResize& args)
			{
				m_nestedForm->SetSize(args.NewSize);
			});

		m_nestedButton.Create(*m_nestedForm, true, { 10,10,150,70 });
		m_nestedButton.SetCaption("Nested button");
		m_nestedForm->Show();
	}

	~TabForm() override
	{
	}
private:
	std::unique_ptr<Berta::NestedForm> m_nestedForm;
	Berta::Button m_nestedButton;
};

int main()
{
	Berta::Form form(Berta::Size(700u, 450u), { true, true, true });
	form.SetCaption("Docking system - Example");

	Berta::MenuBar menuBar(form, { 0,0, 100, 25 });
	auto& menuFile = menuBar.PushBack(L"File");

	auto newSubmenu = std::make_unique<Berta::Menu>();
	newSubmenu->Append("Tab");
	menuFile.AppendSubMenu(L"New", std::move(newSubmenu));
	menuFile.Append("Exit", [](Berta::MenuItem item)
		{
			Berta::GUI::Exit();
		});

	auto& menuWindow = menuBar.PushBack(L"Window");
	menuWindow.Append("Load layout");
	menuWindow.Append("Reset layout");
	
	auto customSubmenu = std::make_unique<Berta::Menu>();
	customSubmenu->Append("One");
	customSubmenu->Append("Two");
	customSubmenu->AppendSeparator();
	customSubmenu->Append("More");
	menuWindow.AppendSubMenu(L"Custom", std::move(customSubmenu));

	auto buttonPaneScene = std::make_unique<Berta::Button>(form, Berta::Rectangle{ 320,250, 200, 200 }, "Scene");
	auto buttonPaneExplorer = std::make_unique<Berta::Button>(form, Berta::Rectangle{ 320,250, 200, 200 }, "Explorer");

	auto tabForm = std::make_unique<TabForm>(form);
	auto tabProperties = std::make_unique<TabProperties>(form);

	form.SetLayout("{VerticalLayout {menuBar Height=24}{Dock dockRoot}}");

	auto& layout = form.GetLayout();
	layout.Attach("menuBar", menuBar);

	layout.AddPaneTab("dockScene", "tab-Scene", std::move(buttonPaneScene), "", Berta::DockPosition::Tab);
	layout.AddPaneTab("dockProp", "tab-Properties", std::move(tabProperties), "dockScene", Berta::DockPosition::Right);
	layout.AddPaneTab("dockProp", "tab-Explorer", std::move(buttonPaneExplorer));
	layout.AddPaneTab("dockNested", "tab-Nested", std::move(tabForm), "dockScene", Berta::DockPosition::Down);

	layout.Apply();

	form.Show();
	form.Exec();

	return 0;
}