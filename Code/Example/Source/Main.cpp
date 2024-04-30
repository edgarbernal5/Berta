/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Core/Form.h>

int main()
{
	Berta::Form form({ 0,0,800,800 }, { true, false, true });
	form.Show();
	form.Exec();

	return 0;
}