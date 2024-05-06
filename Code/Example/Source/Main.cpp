/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Widgets/Form.h>
#include <Berta/Widgets/Label.h>

int main()
{
	Berta::Form form({ 0,0,800,600 }, { true, false, true });
	form.Caption(L"Window");
	
	Berta::Label label(form, { 0,0,800,600 }, L"Hello world!");
	
	form.Show();
	
	form.Exec();

	return 0;
}