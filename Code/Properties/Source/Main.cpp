/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <Berta/Controls/PropertyGrid.h>
#include <Berta/Controls/InputText.h>

class PropertyGridFieldVector3 : public Berta::PropertyGridField
{
public:

protected:
	void Create(Berta::Window* parent) override;

private:
	Berta::InputText m_inputText;
};

void PropertyGridFieldVector3::Create(Berta::Window* parent)
{
	m_inputText.Create(parent);

}

int main()
{
	Berta::Form form(Berta::Size(850u, 750u), { true, true, true });
	form.SetCaption("Property Grid - Example");

	Berta::PropertyGrid propertyGrid(form, { 15,15,280,600 });

	auto categoryTransform = propertyGrid.Append("Transform");
	categoryTransform.Append(Berta::PropertyGridFieldPtr(new PropertyGridFieldVector3()));

	auto categoryMesh = propertyGrid.Append("Mesh");

	form.Show();
	form.Exec();

	return 0;
}
