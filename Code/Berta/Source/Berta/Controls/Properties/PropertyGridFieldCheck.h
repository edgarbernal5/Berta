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
	class PropertyGridFieldCheck : public PropertyGrid::PropertyGridFieldBase
	{
	public:
		using GetterFn = std::function<bool()>;
		using SetterFn = std::function<void(const bool&)>;
		
	public:
		PropertyGridFieldCheck(std::string_view label, GetterFn getter, SetterFn setter) :
			PropertyGridFieldBase(label), m_getter(std::move(getter)), m_setter(std::move(setter))
		{
		}

		void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override;
		
		void SetFocus() override;
		void Refresh() override;
		std::string GetValueAsString() const override;
		
	protected:
		void OnCreate(Window* parent) override;
		void OnVisibilityChanged(bool visible) override;
		void OnEnableChanged(bool enabled) override;
		
		CheckBox m_checkBox;
	
	private:
		GetterFn m_getter;
		SetterFn m_setter;
	};
}

#endif
