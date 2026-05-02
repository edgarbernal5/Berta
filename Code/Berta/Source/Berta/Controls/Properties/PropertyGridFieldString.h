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

#include <string>

namespace Berta
{
	class PropertyGridFieldString : public Internal::PropertyGrid::PropertyGridFieldBase
	{
	public:
		using GetterFn = std::function<std::optional<std::string>()>;
		using SetterFn = std::function<void(const std::string&)>;
		
	public:
		PropertyGridFieldString(std::string_view label, GetterFn getter, SetterFn setter) :
			PropertyGridFieldBase(label), m_getter(std::move(getter)), m_setter(std::move(setter))
		{
		}

		virtual void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override;
		
		virtual void SetFocus() override;
		virtual void Refresh() override;
		std::string GetValueAsString() const override;

		virtual void SetEditable(bool isEditable);
		virtual bool IsEditable() const;

		virtual void SetCharFilter(std::function<bool(wchar_t)> predicate);

	protected:
		virtual void OnCreate(Window* parent) override;
		virtual void OnVisibilityChanged(bool visible) override;
		virtual void OnEnableChanged(bool enabled) override;
		
		GetterFn m_getter;
		SetterFn m_setter;
		TextBox m_textBox;
		
	private:
		void ApplyValue();
	};
}

#endif
