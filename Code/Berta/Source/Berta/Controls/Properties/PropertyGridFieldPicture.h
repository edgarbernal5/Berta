/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_PICTURE_HEADER
#define BT_PROPERTY_GRID_FIELD_PICTURE_HEADER

#include "Berta/Controls/Properties/TypedPropertyField.h"
#include "Berta/Paint/Image.h"

namespace Berta
{
    class PropertyGridFieldPicture : public TypedPropertyField<std::wstring>
    {
    public:
        PropertyGridFieldPicture(std::string_view label, GetterFn getter, SetterFn setter);
        ~PropertyGridFieldPicture() override = default;
		
        void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override;
		
        void SetFocus() override;
        [[nodiscard]] bool HasFocus() const override;
		
        std::wstring GetValueAsString() const override;

        void SetPictureFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels);
        
    protected:
        void OnCreate(Window* parent) override;
        void OnVisibilityChanged(bool visible) override;
        void OnEnableChanged(bool enabled) override;
        void OnReadOnlyChanged(bool readOnly) override;
		
        void SetValueInternal(const std::wstring& value) override;
        void SetMixedValuesInternal() override;
    
    private:
        Image m_previewImage;
        Rectangle m_previewRect;
    };
}
#endif