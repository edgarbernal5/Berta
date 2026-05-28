/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_STRING_BUTTON_HEADER
#define BT_PROPERTY_GRID_FIELD_STRING_BUTTON_HEADER

#include "Berta/Controls/Properties/PropertyGridFieldString.h"
#include "Berta/Controls/Button.h"

#include <string>

namespace Berta
{
	class PropertyGridFieldStringButton : public PropertyGridFieldString
	{
	public:
		using ClickCallback = std::function<void(std::optional<std::wstring> currentValue)>;
		
	public:
		PropertyGridFieldStringButton(std::string_view label, GetterFn getter, SetterFn setter, ClickCallback onButtonClick)
			: PropertyGridFieldString(label, std::move(getter), std::move(setter)), m_clickCallback(std::move(onButtonClick))
		{
			m_buttonText = "...";
		}

		virtual void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override;
		
		void SetFocus() override;
		[[nodiscard]] bool HasFocus() const override;
		
		void SetButtonClick(ClickCallback callback);

	protected:
		virtual void OnCreate(Window* parent) override;
		virtual void OnVisibilityChanged(bool visible) override;
		virtual void OnEnableChanged(bool enabled) override;

	private:
		std::string m_buttonText;
		Button m_button;
		ClickCallback m_clickCallback;

	private:
	};
}

#endif
