/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_CHECK_HEADER
#define BT_PROPERTY_GRID_FIELD_CHECK_HEADER

#include <optional>

#include "Berta/Controls/Properties/TypedPropertyField.h"
#include "Berta/Controls/CheckBox.h"

#include <string>

namespace Berta
{
	class PropertyGridFieldCheck : public TypedPropertyField<bool>
	{
	public:
		PropertyGridFieldCheck(std::string_view label, GetterFn getter, SetterFn setter);
		~PropertyGridFieldCheck() override = default;
		
		void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override;
		
		void SetFocus() override;
		[[nodiscard]] bool HasFocus() const override;
		
		std::wstring GetValueAsString() const override;
		
	protected:
		void OnCreate(Window* parent) override;
		void OnVisibilityChanged(bool visible) override;
		void OnEnableChanged(bool enabled) override;
		
		void SetValueInternal(const bool& value) override;
		void SetMixedValuesInternal() override;
		
		CheckBox m_checkBox;
	};
}

#endif
