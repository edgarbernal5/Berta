/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_COLOR_HEADER
#define BT_PROPERTY_GRID_FIELD_COLOR_HEADER

#include "Berta/Controls/Properties/TypedPropertyField.h"
#include "Berta/Controls/Label.h"

#include <string>
#include <optional>

namespace Berta
{
	class PropertyGridFieldColor : public TypedPropertyField<Color>
	{
	public:
		using GetterFn = std::function<std::optional<Color>()>;
		using SetterFn = std::function<void(Color)>;
        
		using ClickCallback = std::function<std::optional<Color>(std::optional<Color> currentColor)>;

	public:
		PropertyGridFieldColor(std::string_view label, GetterFn getter, SetterFn setter, ClickCallback onButtonClick);

		void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override;
		
		void SetFocus() override;
		[[nodiscard]] bool HasFocus() const override;
		
		std::wstring GetValueAsString() const override;
		
		void SetButtonClick(ClickCallback callback);

	protected:
		void OnCreate(Window* parent) override;
		void OnVisibilityChanged(bool visible) override;
		void OnEnableChanged(bool enabled) override;
		
		void SetValueInternal(const Color& value) override;
		void SetMixedValuesInternal() override;
		
		ClickCallback m_clickCallback;
		Label m_colorRegion;

	private:
	};
}

#endif
