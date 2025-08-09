/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_SELECTION_HEADER
#define BT_PROPERTY_GRID_FIELD_SELECTION_HEADER

#include "Berta/Controls/PropertyGrid.h"
#include "Berta/Controls/ComboBox.h"

#include <string>
#include <vector>

namespace Berta
{
	class PropertyGridFieldSelection : public PropertyGridField
	{
	public:
		PropertyGridFieldSelection(const std::string& label) :
			PropertyGridField(label)
		{
		}

		virtual void Draw(Berta::Graphics& graphics, const Berta::Rectangle& area, uint32_t labelWidth, const Berta::Color& textColor) override;
		
		virtual void SetEnabled(bool enabled) override;
		virtual void SetValue(const std::string& value) override;

		virtual void PushItem(const std::string& optionText);
		virtual void Set(const std::vector<std::string> & options, bool clear = true);

	protected:
		void Create(Berta::Window* parent) override;

		ComboBox m_comboBox;
	private:
	};
}

#endif
