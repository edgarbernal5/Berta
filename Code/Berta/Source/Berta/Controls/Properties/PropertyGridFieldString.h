/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_STRING_HEADER
#define BT_PROPERTY_GRID_FIELD_STRING_HEADER

#include <optional>

#include "Berta/Controls/Properties/PropertyGridFieldBase.h"
#include "Berta/Controls/TextBox.h"
#include "Berta/Controls/Properties/TypedPropertyField.h"

#include <string>

namespace Berta
{
	class PropertyGridFieldString : public TypedPropertyField<std::wstring>
	{
	public:
		PropertyGridFieldString(std::string_view label, GetterFn getter, SetterFn setter);
		~PropertyGridFieldString() override = default;
		
		virtual void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override;
		
		virtual void SetFocus() override;
		[[nodiscard]] bool HasFocus() const override;
		
		std::wstring GetValueAsString() const override;

		virtual void SetEditable(bool isEditable);
		virtual bool IsEditable() const;

		virtual void SetCharFilter(std::function<bool(wchar_t)> predicate);

	protected:
		void OnCreate(Window* parent) override;
		void OnVisibilityChanged(bool visible) override;
		void OnEnableChanged(bool enabled) override;
		void OnReadOnlyChanged(bool readOnly) override;
		
		void SetValueInternal(const std::wstring& value) override;
		void SetMixedValuesInternal() override;
		
		TextBox m_textBox;
		
	private:
		void ApplyValue();
	};
}

#endif
