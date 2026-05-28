/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/MenuBar.h>

#include <Berta/Controls/Button.h>

class DummyDialog : public Berta::Form
{
public:
	explicit DummyDialog(Berta::Window* owner, const Berta::Size& size, const Berta::FormStyle& windowStyle = { true, true, true });
	DummyDialog(Berta::Window* owner, const Berta::Rectangle& rectangle, const Berta::FormStyle& windowStyle = { true, true, true });

private:
	void InitControls();
	
	Berta::Button m_okButton;
	Berta::Button m_cancelButton;
};

DummyDialog::DummyDialog(Berta::Window* owner, const Berta::Size& size, const Berta::FormStyle& windowStyle) :
Berta::Form(owner, size, windowStyle)
{
	SetCaption("Dialog");
	InitControls();
}

DummyDialog::DummyDialog(Berta::Window* owner, const Berta::Rectangle& rectangle, const Berta::FormStyle& windowStyle) :
Berta::Form(owner, rectangle, windowStyle)
{
	SetCaption("Dialog");
	InitControls();
}

void DummyDialog::InitControls()
{
	m_okButton.Create(this->Handle(), false, Berta::Rectangle{10,10,120,30});
	m_okButton.SetCaption("Ok");
	m_okButton.GetEvents().Click.Connect([this](const Berta::ArgClick& args)
	{
		this->Close(Berta::DialogResult::OK);
	});
	m_cancelButton.Create(this->Handle(), false, Berta::Rectangle{140,10,120,30});
	m_cancelButton.SetCaption("Cancel");
	m_cancelButton.GetEvents().Click.Connect([this](const Berta::ArgClick& args)
	{
		this->Close(Berta::DialogResult::Cancel);
	});
}

int main()
{
	Berta::Form form(Berta::Size(450u, 350u), { true, true, true });
	form.SetCaption("Menu - Example");

	Berta::MenuBar menuBar(form, { 15,15,200,45 });

	Berta::Image cutImage("..\\..\\Resources\\Icons\\Icono9_16.png");
	Berta::Image imageImage("..\\..\\Resources\\Icons\\Image 128.png");
	Berta::Image hddImage("..\\..\\Resources\\Icons\\Hard drive 3 128.png");

	auto& fileMenu = menuBar.PushBack("&File");
	auto newSubMenu = std::make_unique<Berta::Menu>();
	newSubMenu->Append("Scene");
	newSubMenu->Append("Texture");
	auto prefabSubMenu = std::make_unique<Berta::Menu>();
	prefabSubMenu->Append(L"Variant");
	newSubMenu->AppendSeparator();
	newSubMenu->AppendSubMenu(L"Prefab", std::move(prefabSubMenu));
	
	newSubMenu->SetImage(0, hddImage);
	
	fileMenu.AppendSubMenu(L"New", std::move(newSubMenu));
	fileMenu.AppendSeparator();
	
	auto openSubMenu = std::make_unique<Berta::Menu>();
	openSubMenu->Append("Berta");
	openSubMenu->Append("Bruno");
	openSubMenu->AppendSeparator();
	openSubMenu->AppendCheckbox(L"Save recent projects", true);
	fileMenu.AppendSubMenu(L"Open", std::move(openSubMenu));
	fileMenu.Append("Exit", [](Berta::MenuItem item)
		{
			Berta::GUI::Exit();
		});
	fileMenu.SetImage(0, imageImage);

	auto& editMenu = menuBar.PushBack("&Edit");
	editMenu.Append("Undo");
	editMenu.Append("Redo");
	editMenu.AppendSeparator();
	editMenu.Append("Cut").SetImage(cutImage).SetEnabled(false);
	editMenu.Append("Copy").SetImage(cutImage).SetEnabled(false);
	
	auto& viewMenu = menuBar.PushBack("View");
	viewMenu.AppendCheckbox(L"Side bar", true);

	auto& helpMenu = menuBar.PushBack("Help");
	helpMenu.Append("About", [&form](Berta::MenuItem item)
	{
		std::cout << "Context menu click > About" << std::endl;
		
		DummyDialog dialog(form, Berta::Size(350,200));
		auto dialogResult = dialog.Exec(form.Handle());
		if (dialogResult == Berta::DialogResult::OK)
		{
			std::cout << "OK" << std::endl;
		}
		else if (dialogResult == Berta::DialogResult::Cancel)
		{
			std::cout << "Cancel" << std::endl;
		}
		else
		{
			std::cout << "Other" << std::endl;
		}
	});

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
			if (!args.ButtonState.RightButton)
			{
				return;
			}
			
			Berta::GUI::ShowContextMenu(popupMenu, form.Handle(), args.Position);
		});

	form.Show();
	form.Exec();

	return 0;
}
