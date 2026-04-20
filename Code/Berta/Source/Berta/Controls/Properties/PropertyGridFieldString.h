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
	class PropertyGridFieldString : public PropertyGrid::PropertyGridFieldBase
	{
	public:
		using GetterFn = std::function<std::string()>;
		using SetterFn = std::function<void(const std::string&)>;
		
	public:
		PropertyGridFieldString(std::string_view label, GetterFn getter, SetterFn setter) :
			PropertyGridFieldBase(label), m_getter(std::move(getter)), m_setter(std::move(setter))
		{
		}

		void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override;
		
		void SetFocus() override;
		void Refresh() override;
		std::string GetValueAsString() const override;

		virtual void SetEditable(bool isEditable);
		virtual bool IsEditable() const;

		virtual void SetCharFilter(std::function<bool(wchar_t)> predicate);

	protected:
		virtual void OnCreate(Window* parent) override;
		void OnVisibilityChanged(bool visible) override;
		void OnEnableChanged(bool enabled) override;
		
	private:
		GetterFn m_getter;
		SetterFn m_setter;
		InputText m_inputText;
	};
}

#endif
