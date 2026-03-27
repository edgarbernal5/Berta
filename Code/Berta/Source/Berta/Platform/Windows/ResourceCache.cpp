/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ResourceCache.h"

#include "Berta/Platform/Windows/D2D.h"

namespace Berta
{
    ResourceCache::ResourceCache(ID2D1RenderTarget* targetRT) :
        m_targetRT(targetRT)
    {
    }

    ID2D1SolidColorBrush* ResourceCache::GetBrush(const Color& color)
    {
        auto it = m_brushCache.find(color);
        if (it != m_brushCache.end())
        {
            return it->second.Get();
        }

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
        auto hr = m_targetRT->CreateSolidColorBrush
        (
            D2D1::ColorF(color.GetR() / 255.0f, color.GetG() / 255.0f, color.GetB() / 255.0f, color.GetA() / 255.0f),
            &brush
        );
        
        if (FAILED(hr))
        {
            return nullptr;
        }
    
        m_brushCache[color] = brush;
        return brush.Get();
    }

    ID2D1StrokeStyle* ResourceCache::GetStrokeStyle(LineStyle style)
    {
        if (style == LineStyle::Solid)
        {
            return nullptr;
        }

        auto it = m_strokeStyleCache.find(style);
        if (it != m_strokeStyleCache.end())
        {
            return it->second.Get();
        }

        Microsoft::WRL::ComPtr<ID2D1StrokeStyle> newStyle;

        std::vector<float> dashes;
        if (style == LineStyle::Dotted)
        {
            dashes = { 1.0f, 1.0f }; // 1 pixel pintado, 1 pixel vacío
        }
        else if (style == LineStyle::Dash)
        {
            dashes = { 3.0f, 2.0f }; // 3 pixeles pintados, 2 pixeles vacíos (Rayas)
        }

        D2D1_STROKE_STYLE_PROPERTIES props = D2D1::StrokeStyleProperties(
            D2D1_CAP_STYLE_FLAT,  
            D2D1_CAP_STYLE_FLAT,  
            D2D1_CAP_STYLE_FLAT,  
            D2D1_LINE_JOIN_MITER, 
            10.0f,                
            D2D1_DASH_STYLE_CUSTOM, 
            0.0f                  
        );

        DirectX::D2DModule::GetInstance().GetFactory()->CreateStrokeStyle
        (
            &props, 
            dashes.data(), 
            static_cast<UINT32>(dashes.size()), 
            &newStyle
        );

        m_strokeStyleCache[style] = newStyle;
        
        return newStyle.Get();
    }
}
