/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/PropertyGrid.h>

class PropertyGridFieldVector3 : public Berta::PropertyGridField
{

};

int main()
{
	Berta::Form form(Berta::Size(950u, 850u), { true, true, true });
	form.SetCaption("Property Grid - Example");

	Berta::PropertyGrid propertyGrid(form, { 15,15,280,600 });

	auto categoryTransform = propertyGrid.Append("Transform");
	categoryTransform.Append(Berta::PropertyGridFieldPtr(new PropertyGridFieldVector3()));

	auto categoryMesh = propertyGrid.Append("Mesh");

	form.Show();
	form.Exec();

	return 0;
}