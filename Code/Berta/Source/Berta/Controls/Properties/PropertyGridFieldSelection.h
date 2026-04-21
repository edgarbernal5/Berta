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
	class PropertyGridFieldSelection : public PropertyGrid::PropertyGridFieldBase
	{
	public:
		using GetterFn = std::function<std::optional<size_t>()>;
		using SetterFn = std::function<void(std::optional<size_t>)>;
		
	public:
		PropertyGridFieldSelection(std::string_view label, GetterFn getter, SetterFn setter) :
			PropertyGridFieldBase(label), m_getter(std::move(getter)), m_setter(std::move(setter))
		{
		}

		void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override;
		
		void SetFocus() override;
		void Refresh() override;
		std::string GetValueAsString() const override;
		
		virtual void SetOption(std::optional<size_t> index);

		virtual void PushItem(const std::string& optionText);
		virtual void Set(const std::vector<std::string> & options, bool clear = true);

	protected:
		void OnCreate(Window* parent) override;
		void OnVisibilityChanged(bool visible) override;
		void OnEnableChanged(bool enabled) override;
		
		GetterFn m_getter;
		SetterFn m_setter;
		ComboBox m_comboBox;
	private:
	};
}

#endif
