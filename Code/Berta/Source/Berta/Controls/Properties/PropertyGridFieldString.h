/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_STRING_HEADER
#define BT_PROPERTY_GRID_FIELD_STRING_HEADER

#include "Berta/Controls/PropertyGrid.h"
#include "Berta/Controls/InputText.h"

#include <string>
#include <vector>

namespace Berta
{
	class PropertyGridFieldString : public PropertyGridField
	{
	public:
		PropertyGridFieldString(const std::string& label, const std::string& value = "") :
			Berta::PropertyGridField(label, value)
		{
		}

		virtual void Draw(Berta::Graphics& graphics, const Berta::Rectangle& area, uint32_t labelWidth, const Berta::Color& textColor) override;
		
		virtual void SetEnabled(bool enabled) override;
		virtual void SetValue(const std::string& value) override;

		virtual void SetEditable(bool isEditable);
		virtual bool IsEditable() const;

		virtual void SetCharFilter(std::function<bool(wchar_t)> predicate);

	protected:
		void Create(Berta::Window* parent) override;

		Berta::InputText m_inputText;
	private:
	};
}

#endif
