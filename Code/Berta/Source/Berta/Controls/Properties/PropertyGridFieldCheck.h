/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_CHECK_HEADER
#define BT_PROPERTY_GRID_FIELD_CHECK_HEADER

#include "Berta/Controls/PropertyGrid.h"
#include "Berta/Controls/CheckBox.h"

#include <string>
#include <vector>

namespace Berta
{
	class PropertyGridFieldCheck : public PropertyGridField
	{
	public:
		PropertyGridFieldCheck(const std::string& label, const std::string& value) :
			PropertyGridField(label, value)
		{
		}

		virtual void Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor) override;

		virtual bool IsChecked() const;
		virtual void SetCheck(bool checked);

		virtual void SetEnabled(bool enabled) override;
		virtual void SetValue(const std::string& value) override;

	protected:
		void Create(Window* parent) override;

		CheckBox m_checkBox;
	private:
	};
}

#endif
