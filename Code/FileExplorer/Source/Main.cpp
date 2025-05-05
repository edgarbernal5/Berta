/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/MenuBar.h>
#include <Berta/Controls/TreeBox.h>
#include <Berta/Controls/ListBox.h>

#include <iostream>
#include <filesystem>

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

	Berta::Image folderImg("..\\..\\Resources\\Icons\\Folder 2 128.png");
	Berta::Image folderOpenImg("..\\..\\Resources\\Icons\\Folder 128.png");
	Berta::Image fileImg("..\\..\\Resources\\Icons\\File 128.png");
	Berta::Image hardDriveImg("..\\..\\Resources\\Icons\\Hard drive 3 128.png");

	Berta::TreeBox treeBox(form, {});
	Berta::ListBox listBox(form, {});

	listBox.AppendHeader("Name", 200);

	treeBox.GetEvents().Selected.Connect([&listBox, &folderImg, &fileImg](const Berta::ArgTreeBoxSelection& args)
	{
		listBox.Clear();

		if (args.Items.size() > 1)
			return;

		auto& treeItem = args.Items[0];

		auto path = treeItem.GetHandle() + "/";

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			try
			{
				if (std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry))
				{
					auto newItem = listBox.Append(entry.path().filename().string());
					newItem.SetIcon(folderImg);
				}
				else if (!std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry)) {
					auto newItem = listBox.Append(entry.path().filename().string());
					newItem.SetIcon(fileImg);
				}
			}
			catch (...)
			{

			}
		}
	});

	treeBox.GetEvents().Expanded.Connect([&listBox, &treeBox, &folderImg](const Berta::ArgTreeBox& args)
	{
		if (!args.IsExpanded)
			return;

		if (args.Item.FirstChild() && args.Item.FirstChild().GetText() == "...")
		{
			auto path = args.Item.GetHandle() + "/";

			auto child = args.Item.FirstChild();
			treeBox.Erase(child);
			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				try
				{
					if (std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry))
					{
						auto newItem = treeBox.Insert(entry.path().string(), entry.path().filename().string());
						newItem.SetIcon(folderImg);

						auto subEntryPath = entry.path().string() + "/";
						for (const auto& subEntry : std::filesystem::directory_iterator(subEntryPath))
						{
							try
							{
								if (std::filesystem::is_directory(subEntry.symlink_status()) && !std::filesystem::is_symlink(subEntry))
								{
									treeBox.Insert(entry.path().string() + "/...", "...");
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

	listBox.GetEvents().DblClick.Connect([&listBox, &treeBox, &folderImg](const Berta::ArgMouse& args)
	{
		if (listBox.GetSelected().empty())
			return;

		auto selected = listBox.GetSelected();
		auto& first = selected.at(0);

		auto treeItemSelected = treeBox.GetSelected().at(0);
		auto newSelected = treeBox.Find(treeItemSelected.GetHandle() + "/" + first.GetText(0));
		if (newSelected)
		{
			treeItemSelected.Expand();
			newSelected.Select();
			return;
		}
		
		if (treeItemSelected.FirstChild() && treeItemSelected.FirstChild().GetText() == "...")
		{
			auto path = treeItemSelected.GetHandle() + "/";

			auto child = treeItemSelected.FirstChild();
			treeBox.Erase(child);
			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				try
				{
					if (std::filesystem::is_directory(entry.symlink_status()) && !std::filesystem::is_symlink(entry))
					{
						auto newItem = treeBox.Insert(entry.path().string(), entry.path().filename().string());
						newItem.SetIcon(folderImg);

						auto subEntryPath = entry.path().string() + "/";
						for (const auto& subEntry : std::filesystem::directory_iterator(subEntryPath))
						{
							try
							{
								if (std::filesystem::is_directory(subEntry.symlink_status()) && !std::filesystem::is_symlink(subEntry))
								{
									treeBox.Insert(entry.path().string() + "/...", "...");
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

			newSelected = treeBox.Find(treeBox.GetSelected().at(0).GetHandle() + "/" + first.GetText(0));
			if (newSelected)
			{
				treeItemSelected.Expand();
				newSelected.Select();
				return;
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

			auto newItem = treeBox.Insert(letter, text);
			newItem.SetIcon(hardDriveImg);

			treeBox.Insert(letter + ".../", "...");
		}
	}

	form.SetLayout("{VerticalLayout {menuBar Height=24}{{treeBox Width=40%}|{listBox}}");

	auto& layout = form.GetLayout();
	layout.Attach("menuBar", menuBar);
	layout.Attach("treeBox", treeBox);
	layout.Attach("listBox", listBox);
	layout.Apply();

	form.Show();
	form.Exec();

	return 0;
}