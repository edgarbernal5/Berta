/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_VECTOR3_HEADER
#define BT_PROPERTY_GRID_FIELD_VECTOR3_HEADER

#include "Berta/Controls/Properties/TypedPropertyField.h"
#include "Berta/Controls/Properties/PropertyGridTypes.h"

namespace Berta
{
    class PropertyGridFieldVector3 : public TypedPropertyField<OptionalVector3>
    {
    public:
        PropertyGridFieldVector3(std::string_view label, GetterFn getter, SetterFn setter);
        ~PropertyGridFieldVector3() override = default;
		
        void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override;
		
        void SetFocus() override;
        [[nodiscard]] bool HasFocus() const override;
		
        std::wstring GetValueAsString() const override;

    protected:
        void OnCreate(Window* parent) override;
        void OnVisibilityChanged(bool visible) override;
        void OnEnableChanged(bool enabled) override;
		
        void SetValueInternal(const OptionalVector3& value) override;
        void SetMixedValuesInternal() override;
        
    private:
        std::wstring FormatSummary(const OptionalVector3& v) const;
        std::wstring FormatFloat(float val) const;
        
        std::wstring m_summaryText{ L"(0.0, 0.0, 0.0)" };
    };
}

#endif