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
	class PropertyGridFieldVector3 : public PropertyGrid::PropertyGridFieldBase
	{
	public:
		using GetterFn = std::function<std::string()>;
		using SetterFn = std::function<void(const std::string&)>;
		
	public:
		PropertyGridFieldVector3(std::string_view label, GetterFn getter, SetterFn setter) :
			PropertyGridFieldBase(label), m_getter(getter), m_setter(setter) 
		{
		}

		virtual void Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const LayoutConfig& config) override;
		
		std::string GetValueAsString() const override;
		
		void SetEnabled(bool enabled) override;

	protected:
		void Create(Window* parent) override;
		void OnVisibilityChanged(bool visible) override;

	private:
		GetterFn m_getter;
		SetterFn m_setter;
		
		InputText m_inputTexts[3];
		std::string m_inputTextLabels[3]{ "X", "Y", "Z" };
	};
}

#endif
