/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/MenuBar.h>
#include <Berta/Controls/TreeBox.h>
#include <Berta/Controls/ListBox.h>
#include <Berta/Controls/TabBar.h>
#include <Berta/Controls/ThumbListBox.h>
#include <Berta/Controls/ComboBox.h>
#include <Berta/Controls/Slider.h>

#include <iostream>
#include <filesystem>
#include <Berta/GUI/ControlDrawBatch.h>

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
				std::string letter = std::string(1, 'A' + i) + ":/";
				std::string text = std::string(1, 'A' + i) + ":";

				auto newItem = m_treeBox.Insert(letter, text);
				newItem.SetIcon(m_hardDriveImg);

				m_treeBox.Insert(letter + ".../", "...");
			}
		}

		m_treeBox.GetEvents().Selected.Connect([this](const Berta::ArgTreeBoxSelection& args)
		{
			m_listBox.Clear();

			if (args.Items.size() > 1)
				return;

			auto& treeItem = args.Items[0];

			auto path = m_treeBox.GetKeyPath(treeItem, '/') + "/";

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
						else if (!std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry)) {
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

			if (args.Item.FirstChild() && args.Item.FirstChild().GetText() == "...")
			{
				auto path = m_treeBox.GetKeyPath(args.Item, '/') + "/";

				auto child = args.Item.FirstChild();
				m_treeBox.Erase(child);
				for (const auto& entry : std::filesystem::directory_iterator(path))
				{
					try
					{
						if (std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry))
						{
							auto newItem = m_treeBox.Insert(entry.path().string(), entry.path().filename().string());
							newItem.SetIcon(m_folderImg);

							auto subEntryPath = entry.path().string() + "/";
							for (const auto& subEntry : std::filesystem::directory_iterator(subEntryPath))
							{
								try
								{
									if (std::filesystem::is_directory(subEntry.symlink_status()) && !std::filesystem::is_symlink(subEntry))
									{
										m_treeBox.Insert(entry.path().string() + "/...", "...");
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
			auto newSelected = m_treeBox.Find(pathTreeItemSelected + "/" + first.GetText(0));
			if (newSelected)
			{
				treeItemSelected.Expand();
				newSelected.Select();
				return;
			}

			if (treeItemSelected.FirstChild() && treeItemSelected.FirstChild().GetText() == "...")
			{
				auto path = m_treeBox.GetKeyPath(treeItemSelected, '/') + "/";

				auto child = treeItemSelected.FirstChild();
				m_treeBox.Erase(child);
				for (const auto& entry : std::filesystem::directory_iterator(path))
				{
					try
					{
						if (std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry))
						{
							auto newItem = m_treeBox.Insert(entry.path().string(), entry.path().filename().string());
							newItem.SetIcon(m_folderImg);

							auto subEntryPath = entry.path().string() + "/";
							for (const auto& subEntry : std::filesystem::directory_iterator(subEntryPath))
							{
								try
								{
									if (std::filesystem::is_directory(subEntry.symlink_status()) && !std::filesystem::is_symlink(subEntry))
									{
										m_treeBox.Insert(entry.path().string() + "/...", "...");
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

				newSelected = m_treeBox.Find(pathTreeItemSelected + "/" + first.GetText(0));
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

class TabImages : public Berta::Panel
{
public:
	TabImages(Berta::Window* parent) :
		Panel(parent)
	{
		m_slider.SetOrientation(false);
		m_slider.SetMinMax(0, 4);

		m_slider.GetEvents().ValueChanged.Connect([this](const Berta::ArgSlider& args)
		{
			uint32_t thumbnailSizes[5]{ 32u, 64u, 96u, 128u, 256u };

			m_thumbListBox.SetThumbnailSize(thumbnailSizes[args.Value]);
		});

		m_comboBox.GetEvents().Selected.Connect([this](const Berta::ArgComboBox& args)
		{
			m_currentPath = m_comboBox.GetText(args.SelectedIndex);
			Berta::ControlDrawBatch controlBatch(m_thumbListBox);
			for (const auto& entry : std::filesystem::directory_iterator(m_currentPath))
			{
				try
				{
					if (std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry))
					{
						//auto newItem = m_treeBox.Insert(entry.path().string(), entry.path().filename().string());
						//m_thumbListBox.AddItem(entry.path().filename(), m_hardDriveImg);
						m_thumbListBox.AddItem(entry.path().filename(), m_folderImg);

					}
				}
				catch (...)
				{

				}
			}
		});

		DWORD drives = ::GetLogicalDrives();

		for (char i = 0; i < 26; ++i)
		{
			if (drives & (1 << i))
			{
				std::string letter = std::string(1, 'A' + i) + ":/";
				std::string text = std::string(1, 'A' + i) + ":";

				m_comboBox.PushItem(letter, m_hardDriveImg);
			}
		}

		m_layout.Create(*this);
		m_layout.Parse("{VerticalLayout {HorizontalLayout Height=25 {{comboBox Width=120}{slider Width=180}}{thumbBox}}");

		m_layout.Attach("comboBox", m_comboBox);
		m_layout.Attach("slider", m_slider);
		m_layout.Attach("thumbBox", m_thumbListBox);
		m_layout.Apply();
	}

private:
	Berta::ThumbListBox m_thumbListBox{ *this };
	Berta::ComboBox m_comboBox{ *this };
	Berta::Slider m_slider{ *this };

	std::wstring m_currentPath;
	Berta::Layout m_layout;

	Berta::Image m_hardDriveImg{ "..\\..\\Resources\\Icons\\Hard drive 3 128.png" };
	Berta::Image m_folderImg{ "..\\..\\Resources\\Icons\\Folder 2 128.png" };
};

int main()
{
	Berta::Form form(Berta::Size(700u, 550u), { true, true, true });
	form.SetCaption("File explorer - Example");

	Berta::MenuBar menuBar(form, { 0,0, 100, 25 });
	auto& menuFile = menuBar.PushBack(L"File");
	menuFile.Append("Exit", [](Berta::MenuItem& item)
	{
		Berta::GUI::Exit();
	});

	Berta::TabBar tabbar(form, { 70, 250, 400, 285 });

	tabbar.PushBack<TabExplorer>("Explorer");
	tabbar.PushBack<TabImages>("Images");
	form.SetLayout("{VerticalLayout {menuBar Height=24}{tabBar}");

	auto& layout = form.GetLayout();
	layout.Attach("menuBar", menuBar);
	layout.Attach("tabBar", tabbar);
	layout.Apply();

	form.Show();
	form.Exec();

	return 0;
}