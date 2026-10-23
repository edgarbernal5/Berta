/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_COLOR_HEADER
#define BT_PROPERTY_GRID_FIELD_COLOR_HEADER

#include "Berta/Controls/Properties/TypedPropertyField.h"
#include "Berta/Controls/ColorButtonPicker.h"

#include <string>
#include <optional>

#include "EnumTypes.h"
#include "Berta/Controls/TextBox.h"

namespace Berta
{
	class PropertyGridFieldColor : public TypedPropertyField<OptionalColor>
	{
	public:
		using ClickCallback = std::function<std::optional<OptionalColor>(std::optional<OptionalColor> currentColor)>;

	public:
		PropertyGridFieldColor(std::string_view label, GetterFn getter, SetterFn setter, ClickCallback onButtonClick, bool hasAlphaChannel = true);

		void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override;
		
		void SetFocus() override;
		[[nodiscard]] bool HasFocus() const override;
		
		std::wstring GetValueAsString() const override;
		
		void SetButtonClick(ClickCallback callback);

	protected:
		void OnCreate(Window* parent) override;
		void OnVisibilityChanged(bool visible) override;
		void OnEnableChanged(bool enabled) override;
		
		void SetValueInternal(const OptionalColor& value) override;
		void SetMixedValuesInternal() override;
		
		void ApplyValue(int index);
		
		ClickCallback m_clickCallback;
		ColorButtonPicker m_colorRegion;
		
		TextBox m_textBoxes[4];
		std::wstring m_textBoxLabels[4] = { L"R", L"G", L"B", L"A"};
        std::wstring m_summaryText{ L"(0, 0, 0, 0)" };
		bool m_hasAlphaChannel;

	private:
		std::wstring FormatSummary(const OptionalColor& v) const;
	};
}

#endif
