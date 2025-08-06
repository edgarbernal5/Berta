/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_VECTOR_3_HEADER
#define BT_PROPERTY_GRID_FIELD_VECTOR_3_HEADER

#include "Berta/Controls/PropertyGrid.h"
#include "Berta/Controls/InputText.h"

#include <string>
#include <vector>

namespace Berta
{
	class PropertyGridFieldVector3 : public PropertyGridField
	{
	public:
		PropertyGridFieldVector3(const std::string& label, const std::string& value) :
			Berta::PropertyGridField(label, value)
		{
		}

		virtual void Draw(Berta::Graphics& graphics, const Berta::Rectangle& area, uint32_t labelWidth, const Berta::Color& textColor) override;

		void SetEnabled(bool enabled) override;
		void SetValue(const std::string& value) override;

	protected:
		void Create(Berta::Window* parent) override;

	private:
		Berta::InputText m_inputTexts[3];
		std::string m_inputTextLabels[3]{ "X", "Y", "Z" };
	};
}

#endif
