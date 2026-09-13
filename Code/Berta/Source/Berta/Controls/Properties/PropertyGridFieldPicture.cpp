#include "btpch.h"
#include "PropertyGridFieldPicture.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
    PropertyGridFieldPicture::PropertyGridFieldPicture(std::string_view label, GetterFn getter, SetterFn setter)
        : TypedPropertyField(label, std::move(getter), std::move(setter))
    {
        m_height = 128;
    }

    void PropertyGridFieldPicture::Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config)
    {
        auto frameSize = area.Height;
        Rectangle backgroundArea = area;
        backgroundArea.Width = frameSize;
        m_previewRect = backgroundArea;
        if (!m_previewImage)
        {
            graphics.DrawRoundRectBox(backgroundArea, config.Background,config.BoxBorderColor, true);
            return;
        }
        
        auto imageSize = m_previewImage.GetSize();
       
        float scaleX = static_cast<float>(frameSize) / imageSize.Width;
        float scaleY = static_cast<float>(frameSize) / imageSize.Height;

        float scale = std::min<float>({1.0f, scaleY, scaleX});

        uint32_t targetWidth = static_cast<uint32_t>(static_cast<float>(imageSize.Width) * scale);
        uint32_t targetHeight = static_cast<uint32_t>(static_cast<float>(imageSize.Height) * scale);

        int offsetX = static_cast<int>(frameSize - targetWidth) >> 1;
        int offsetY = static_cast<int>(frameSize - targetHeight) >> 1;

        Rectangle imageRect = { area.X + offsetX, area.Y + offsetY, targetWidth, targetHeight };
        
        graphics.DrawRoundRectBox(backgroundArea, config.Background, config.BoxBorderColor, true);
        
        m_previewImage.Paste(graphics, imageRect);
    }

    void PropertyGridFieldPicture::SetFocus()
    {
    }

    bool PropertyGridFieldPicture::HasFocus() const
    {
        return false;
    }

    std::wstring PropertyGridFieldPicture::GetValueAsString() const
    {
        return L"";
    }

    void PropertyGridFieldPicture::SetPictureFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels)
    {
        m_previewImage.OpenFromMemory(pixels, width, height, channels);
        
        GUI::MarkAsNeedUpdate(m_parent, &m_previewRect);
    }

    void PropertyGridFieldPicture::OnCreate(Window* parent)
    {
        Refresh();
    }

    void PropertyGridFieldPicture::OnVisibilityChanged(bool visible)
    {

    }

    void PropertyGridFieldPicture::OnEnableChanged(bool enabled)
    {

    }

    void PropertyGridFieldPicture::OnReadOnlyChanged(bool readOnly)
    {

    }

    void PropertyGridFieldPicture::SetValueInternal(const std::wstring& value)
    {
        m_previewImage.Open(value);
        if (!m_previewImage)
        {
            
        }
    }

    void PropertyGridFieldPicture::SetMixedValuesInternal()
    {
    }
}
