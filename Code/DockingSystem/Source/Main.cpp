/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/Button.h>
#include <Berta/Controls/MenuBar.h>
#include <Berta/Controls/Panel.h>

#include <D3D12Lite.h>
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
		m_nestedForm = std::make_unique<Berta::NestedForm>(this->Handle(), Berta::Rectangle{ 0,60, 200, 200 }, Berta::FormStyle::Flat(), true);
		m_nestedForm->SetCustomPaintCallback([this]()
			{
				m_device->BeginFrame();
				auto& backBuffer = m_device->GetCurrentBackBuffer();

				m_graphicsContext->Reset();
				m_graphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
				m_graphicsContext->FlushBarriers();

				m_graphicsContext->ClearRenderTarget(backBuffer, Color(0.3f, 0.3f, 0.8f));

				m_graphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
				m_graphicsContext->FlushBarriers();

				m_device->SubmitContextWork(*m_graphicsContext);

				m_device->EndFrame();
				m_device->Present();
			});

		m_nestedForm->GetEvents().Resize.Connect([this](const Berta::ArgResize& args)
			{
				m_device->Resize(D3D12Lite::Uint2{ args.NewSize.Width, args.NewSize.Height });

				/*D3D12_VIEWPORT viewport;
				viewport.TopLeftX = 0.0f;
				viewport.TopLeftY = 0.0f;
				viewport.Width = args.NewSize.Width;
				viewport.Height = args.NewSize.Height;
				viewport.MinDepth = D3D12_MIN_DEPTH;
				viewport.MaxDepth = D3D12_MAX_DEPTH;

				m_graphicsContext->SetViewport(viewport);*/
			});

		auto formSize = m_nestedForm->GetSize();
		m_device = std::make_unique<D3D12Lite::Device>(m_nestedForm->Handle()->RootHandle.Handle, D3D12Lite::Uint2{ formSize.Width, formSize.Height });
		m_graphicsContext = m_device->CreateGraphicsContext();

		this->GetEvents().Resize.Connect([this](const Berta::ArgResize& args)
		{
			m_nestedForm->SetArea({ 0, 0, args.NewSize.Width, args.NewSize.Height });
		});

		m_nestedForm->Show();
	}

private:
	std::unique_ptr<Berta::NestedForm> m_nestedForm;
	std::unique_ptr<D3D12Lite::Device> m_device;
	std::unique_ptr<D3D12Lite::GraphicsContext> m_graphicsContext;
};

int main()
{
	Berta::Form form(Berta::Size(700u, 450u), { true, true, true });
	form.SetCaption("Docking system - Example");

	Berta::MenuBar menuBar(form, { 0,0, 100, 25 });
	auto& menuFile = menuBar.PushBack(L"File");

	menuFile.Append("New");
	auto newSubmenu = menuFile.CreateSubMenu(0);
	newSubmenu->Append("Tab");

	menuFile.Append("Exit", [](Berta::MenuItem& item)
	{
		Berta::GUI::Exit();
	});

	auto& menuWindow = menuBar.PushBack(L"Window");
	menuWindow.Append("Load layout");
	menuWindow.Append("Reset layout");
	menuWindow.Append("Custom");
	auto customSubmenu = menuWindow.CreateSubMenu(2);
	customSubmenu->Append("One");
	customSubmenu->Append("Two");
	customSubmenu->AppendSeparator();
	customSubmenu->Append("More");
	

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