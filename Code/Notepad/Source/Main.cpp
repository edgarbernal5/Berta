/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/MenuBar.h>
#include <Berta/Controls/TextBox.h>

int main()
{
	Berta::Form form(Berta::Size(450u, 350u), { true, true, true });
	form.SetCaption("Notepad - Example");

	Berta::MenuBar menuBar(form, { 0,0, 100, 25 });
	auto& menuFile = menuBar.PushBack("File");
	menuFile.Append("Exit", [](Berta::MenuItem item)
		{
			Berta::GUI::Exit();
		});

	Berta::TextBox multiLineInputText(form, { 0,0, 100, 25 });
	multiLineInputText.SetMultiLine(true);
	multiLineInputText.SetWordWrap(false);
	multiLineInputText.SetCaption("Primera línea\nSegunda línea\nBajada de línea");
	
	Berta::TextBox wordWrapSingleLine(form, { 0,0, 100, 25 });
	wordWrapSingleLine.SetMultiLine(false);
	wordWrapSingleLine.SetWordWrap(true);
	wordWrapSingleLine.SetCaption("Primera línea. Segunda línea. Bajada de línea");
	
	Berta::TextBox oneLineInputText(form, { 0,0, 100, 25 });
	oneLineInputText.SetMultiLine(false);
	oneLineInputText.SetWordWrap(false);
	oneLineInputText.SetCaption("Primera línea. Segunda línea. Bajada de línea");
	oneLineInputText.SetScrollBarVisibility(Berta::ScrollBarVisibility::Hidden, Berta::ScrollBarVisibility::Hidden);
	
	form.SetLayout("{VerticalLayout {menuBar Height=24}{{a}{VerticalLayout {b}{{}{c}}}}}");

	auto& layout = form.GetLayout();
	layout.Attach("menuBar", menuBar);
	layout.Attach("a", multiLineInputText);
	layout.Attach("b", wordWrapSingleLine);
	layout.Attach("c", oneLineInputText);
	layout.Apply();
	
	form.Show();
	form.Exec();

	return 0;
}