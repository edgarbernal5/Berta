/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/Button.h>
#include <Berta/Controls/MenuBar.h>
#include <Berta/Controls/Panel.h>
#include <Berta/Controls/TreeBox.h>
#include <Berta/Controls/ListBox.h>

#include <D3D12Lite.h>
#include <iostream>

struct TreeItemData
{
	std::wstring path;
};
class TabExplorer : public Berta::Panel
{
public:
	TabExplorer(Berta::Window* parent) :
		Panel(parent)
	{
		m_listBox.AppendHeader("Name", 200);
		m_listBox.AppendHeader("Type", 120);

		DWORD drives = ::GetLogicalDrives();

		for (char i = 0; i < 26; ++i)
		{
			if (drives & (1 << i))
			{
				std::wstring letter = std::wstring(1, 'A' + i) + L":/";
				std::wstring text = std::wstring(1, 'A' + i) + L":";

				auto newItem = m_treeBox.Insert(letter, text);
				newItem.SetIcon(m_hardDriveImg);
				TreeItemData nodeData{ letter };
				newItem.SetUserData(nodeData);

				m_treeBox.Insert(letter + L".../", L"...");
			}
		}

		m_treeBox.GetEvents().Selected.Connect([this](const Berta::ArgTreeBoxSelection& args)
			{
				m_listBox.Clear();

				if (args.Items.size() > 1)
					return;

				auto& treeItem = args.Items[0];
				auto userData = treeItem.GetUserData<TreeItemData>();
				auto path = m_treeBox.GetKeyPath(treeItem, '/') + L"/";

				try
				{
					for (const auto& entry : std::filesystem::directory_iterator(path))
					{
						try
						{
							if (std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry))
							{
								auto newItem = m_listBox.Append(entry.path().filename().string());
								newItem.SetIcon(m_folderImg);
							}
							else if (!std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry))
							{
								auto newItem = m_listBox.Append(entry.path().filename().string());
								newItem.SetIcon(m_fileImg);
							}
						}
						catch (...)
						{

						}
					}
				}
				catch (...)
				{

				}
			});

		m_treeBox.GetEvents().Expanded.Connect([this](const Berta::ArgTreeBox& args)
			{
				if (!args.IsExpanded)
					return;

				if (args.Item.FirstChild() && args.Item.FirstChild().GetText() == L"...")
				{
					auto path = m_treeBox.GetKeyPath(args.Item, '/') + L"/";

					auto child = args.Item.FirstChild();
					m_treeBox.Erase(child);
					for (const auto& entry : std::filesystem::directory_iterator(path))
					{
						try
						{
							if (std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry))
							{
								auto newItem = m_treeBox.Insert(entry.path().wstring(), entry.path().filename().wstring());
								newItem.SetIcon(m_folderImg);
								TreeItemData itemData{ entry.path().wstring() };
								newItem.SetUserData(itemData);
								auto subEntryPath = entry.path().string() + "/";
								for (const auto& subEntry : std::filesystem::directory_iterator(subEntryPath))
								{
									try
									{
										if (std::filesystem::is_directory(subEntry.symlink_status()) && !std::filesystem::is_symlink(subEntry))
										{
											m_treeBox.Insert(entry.path().wstring() + L"/...", L"...");
											break;
										}
									}
									catch (...)
									{

									}
								}
							}
						}
						catch (...)
						{

						}
					}
				}
			});

		m_listBox.GetEvents().DblClick.Connect([this](const Berta::ArgMouse& args)
			{
				if (m_listBox.GetSelected().empty())
					return;

				auto selected = m_listBox.GetSelected();
				auto& first = selected.at(0);

				auto treeItemSelected = m_treeBox.GetSelected().at(0);
				auto pathTreeItemSelected = m_treeBox.GetKeyPath(treeItemSelected, '/');
				auto newSelected = m_treeBox.Find(pathTreeItemSelected + L"/" +  Berta::StringUtils::UTF8ToWide(first.GetText(0)));
				if (newSelected)
				{
					treeItemSelected.Expand();
					newSelected.Select();
					return;
				}

				if (treeItemSelected.FirstChild() && treeItemSelected.FirstChild().GetText() == L"...")
				{
					auto path = m_treeBox.GetKeyPath(treeItemSelected, '/') + L"/";

					auto child = treeItemSelected.FirstChild();
					m_treeBox.Erase(child);
					for (const auto& entry : std::filesystem::directory_iterator(path))
					{
						try
						{
							if (std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry))
							{
								auto newItem = m_treeBox.Insert(entry.path().wstring(), entry.path().filename().wstring());
								newItem.SetIcon(m_folderImg);
								TreeItemData itemData{ entry.path().wstring() };
								newItem.SetUserData(itemData);

								auto subEntryPath = entry.path().string() + "/";
								for (const auto& subEntry : std::filesystem::directory_iterator(subEntryPath))
								{
									try
									{
										if (std::filesystem::is_directory(subEntry.symlink_status()) && !std::filesystem::is_symlink(subEntry))
										{
											m_treeBox.Insert(entry.path().wstring() + L"/...", L"...");
											break;
										}
									}
									catch (...)
									{

									}
								}
							}
						}
						catch (...)
						{

						}
					}

					newSelected = m_treeBox.Find(pathTreeItemSelected + L"/" + Berta::StringUtils::UTF8ToWide(first.GetText(0)));
					if (newSelected)
					{
						treeItemSelected.Expand();
						newSelected.Select();
						return;
					}
				}
			});

		m_layout.Create(*this);
		m_layout.Parse("{{treeBox Width=40%}|{listBox}}");

		m_layout.Attach("treeBox", m_treeBox);
		m_layout.Attach("listBox", m_listBox);
		m_layout.Apply();
	}

private:
	Berta::TreeBox m_treeBox{ *this };
	Berta::ListBox m_listBox{ *this };

	Berta::Image m_folderImg{ "..\\..\\Resources\\Icons\\Folder 2 128.png" };
	Berta::Image m_folderOpenImg{ "..\\..\\Resources\\Icons\\Folder 128.png" };
	Berta::Image m_fileImg{ "..\\..\\Resources\\Icons\\File 128.png" };
	Berta::Image m_hardDriveImg{ "..\\..\\Resources\\Icons\\Hard drive 3 128.png" };

	Berta::Layout m_layout;
};

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
				OnDraw();
				std::cout << " .... END RENDERING ////***/**/" << std::endl;
			});

		m_nestedForm->GetEvents().Resize.Connect([this](const Berta::ArgResize& args)
			{
				m_isResizing = true;
				m_device->Resize(D3D12Lite::Uint2{ args.NewSize.Width, args.NewSize.Height });

				m_viewport.TopLeftX = 0.0f;
				m_viewport.TopLeftY = 0.0f;
				m_viewport.Width = args.NewSize.Width;
				m_viewport.Height = args.NewSize.Height;
				m_viewport.MinDepth = D3D12_MIN_DEPTH;
				m_viewport.MaxDepth = D3D12_MAX_DEPTH;
				m_isResizing = false;

				//OnDraw();
			});

		auto formSize = m_nestedForm->GetSize();
		m_device = std::make_unique<D3D12Lite::Device>(m_nestedForm->Handle()->RootHandle.Handle, D3D12Lite::Uint2{ formSize.Width, formSize.Height });
		m_graphicsContext = m_device->CreateGraphicsContext();

		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;
		m_viewport.Width = formSize.Width;
		m_viewport.Height = formSize.Height;
		m_viewport.MinDepth = D3D12_MIN_DEPTH;
		m_viewport.MaxDepth = D3D12_MAX_DEPTH;
		this->GetEvents().Resize.Connect([this](const Berta::ArgResize& args)
			{
				m_nestedForm->SetArea({ 0, 0, args.NewSize.Width, args.NewSize.Height });
			});

		m_nestedForm->Show();
	}

private:
	void OnDraw()
	{
		//auto formSize = m_nestedForm->GetSize();
				//if ((float)formSize.Width != m_viewport.Width || (float)formSize.Height != m_viewport.Height)
				//{
				//	//m_device->Resize(D3D12Lite::Uint2{ formSize.Width, formSize.Height });

				//	m_viewport.Width = formSize.Width;
				//	m_viewport.Height = formSize.Height;
				//	std::cout << "////***/**/ CAMBIOOOOOO...." << std::endl;
				//}

				//std::cout << "////***/**/ RENDERING...." << std::endl;
		m_device->BeginFrame();
		auto& backBuffer = m_device->GetCurrentBackBuffer();

		m_graphicsContext->Reset();
		m_graphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_graphicsContext->FlushBarriers();
		auto rtvHandle = backBuffer.mRTVDescriptor.mCPUHandle;
		m_graphicsContext->SetTargets(1, &rtvHandle, {});
		m_graphicsContext->SetViewport(m_viewport);
		m_graphicsContext->ClearRenderTarget(backBuffer, Color(0.3f, 0.3f, 0.8f));
		//m_graphicsContext->ClearDepthStencilTarget(depthBuffer, 1.0f, 0);

		m_graphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
		m_graphicsContext->FlushBarriers();

		m_device->SubmitContextWork(*m_graphicsContext);

		m_device->EndFrame();
		m_device->Present();
	}

	std::unique_ptr<Berta::NestedForm> m_nestedForm;
	std::unique_ptr<D3D12Lite::Device> m_device;
	std::unique_ptr<D3D12Lite::GraphicsContext> m_graphicsContext;

	D3D12_VIEWPORT m_viewport;
	bool m_isResizing{ false };
};

class TabScene : public Berta::Panel
{
public:
	TabScene(Berta::Window* parent) :
		Panel(parent)
	{
		m_layout.Create(*this);
		m_layout.Parse("{VerticalLayout {Dock dockRoot}}");

		m_tabProperties = std::make_unique<TabProperties>(this->Handle());
		m_tabForm = std::make_unique<TabForm>(this->Handle());

		m_layout.AddPaneTab("panel-properties-pane", "tab-properties", *m_tabProperties, "", Berta::DockPosition::Tab);
		m_layout.AddPaneTab("panel-pane", "tab-scene", *m_tabForm, "panel-properties-pane", Berta::DockPosition::Right);

		m_layout.Apply();
	}

private:
	Berta::Layout m_layout;
	std::unique_ptr<TabProperties> m_tabProperties;
	std::unique_ptr<TabForm> m_tabForm;
};

int main()
{
	Berta::Form form(Berta::Size(700u, 450u), { true, true, true });
	form.SetCaption("Docking system - Example");

	Berta::MenuBar menuBar(form, { 0,0, 100, 25 });
	auto& menuFile = menuBar.PushBack(L"&File");

	menuFile.Append("New");
	/*auto newSubmenu = menuFile.CreateSubMenu(0);
	newSubmenu->Append("Tab");

	menuFile.Append("Exit", [](Berta::MenuItem& item)
		{
			Berta::GUI::Exit();
		});

	auto& menuWindow = menuBar.PushBack(L"W&indow");
	menuWindow.Append("Load layout");
	menuWindow.Append("Reset layout");
	menuWindow.Append("Custom");
	auto customSubmenu = menuWindow.CreateSubMenu(2);
	customSubmenu->Append("O&ne");
	customSubmenu->Append("Tw&o");
	customSubmenu->AppendSeparator();
	customSubmenu->Append("More");*/

	auto& helpMenu = menuBar.PushBack(L"Help");
	helpMenu.Append("About");

	TabScene buttonPaneScene(form);
	TabExplorer buttonPaneExplorer(form);

	form.SetLayout("{VerticalLayout {menuBar Height=24}{Dock dockRoot}}");

	auto& layout = form.GetLayout();
	layout.Attach("menuBar", menuBar);

	layout.AddPaneTab("dockScene", "tab-Scene-Document", buttonPaneScene, "", Berta::DockPosition::Tab);
	//layout.AddPaneTab("dockProp", "tab-Properties", tabProperties, "dockScene", Berta::DockPosition::Right);
	layout.AddPaneTab("dockProp", "tab-Explorer", buttonPaneExplorer, "dockScene", Berta::DockPosition::Down);
	//layout.AddPaneTab("dockD3D", "tab-D3D", tabForm, "dockScene", Berta::DockPosition::Down);

	layout.Apply();

	form.Show();
	form.Exec();

	return 0;
}