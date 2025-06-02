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
		m_nestedForm = std::make_unique<Berta::NestedForm>(this->Handle(), Berta::Rectangle{ 0,0, 200, 200 });
		m_nestedForm->GetAppearance().Background = Berta::Color{ 0xFFAB20CC };

		this->GetEvents().Resize.Connect([this](const Berta::ArgResize& args)
		{
			m_nestedForm->SetArea({ 0, 0, args.NewSize.Width, args.NewSize.Height });
		});

		m_button.Create(m_nestedForm->Handle(), true, Berta::Rectangle{ 10,10,140,40 });
		m_button.SetCaption("Nested button");
#ifdef BT_DEBUG
		m_button.SetDebugName("Nested button");
#endif
		m_nestedForm->Show();
	}

private:
	std::unique_ptr<Berta::NestedForm> m_nestedForm;
	Berta::Button m_button;
};

int main()
{
	Berta::Form form(Berta::Size(700u, 450u), { true, true, true });
	form.SetCaption("Docking system - Example");

	Berta::MenuBar menuBar(form, { 0,0, 100, 25 });
	auto& menuFile = menuBar.PushBack(L"File");
	menuFile.Append("Exit", [](Berta::MenuItem& item)
	{
		Berta::GUI::Exit();
	});

	Berta::Button buttonPaneScene(form, { 320,250, 200, 200 }, "Scene");
	Berta::Button buttonPaneExplorer(form, { 320,250, 200, 200 }, "Explorer");
	
	TabForm tabForm(form);
	TabProperties tabProperties(form);

	form.SetLayout("{VerticalLayout {menuBar Height=24}{Dock dockRoot}}");

	auto& layout = form.GetLayout();
	layout.Attach("menuBar", menuBar);

	layout.AddPaneTab("dockScene", "tab-Scene", &buttonPaneScene, "", Berta::DockPosition::Tab);
	layout.AddPaneTab("dockProp", "tab-Properties", &tabProperties, "dockScene", Berta::DockPosition::Right);
	layout.AddPaneTab("dockProp", "tab-Explorer", &buttonPaneExplorer);
	layout.AddPaneTab("dockD3D", "tab-D3D", &tabForm, "dockScene", Berta::DockPosition::Down);

	layout.Apply();

	form.Show();
	form.Exec();

	return 0;
}