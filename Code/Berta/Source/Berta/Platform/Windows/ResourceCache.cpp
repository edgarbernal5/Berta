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

    ID2D1PathGeometry* ResourceCache::GetOrCreateGeometry(const Graphics* graphics, int width, int height,const CornerRadii& radii)
    {
        // Hit del caché: Si nada cambió, devolvemos el puntero existente
        if (m_geometry && m_lastWidth == width && m_lastHeight == height && m_lastRadii == radii) 
        {
            return m_geometry.Get();
        }

        // Miss del caché: Liberamos la anterior
        m_geometry.Reset();

        // Creamos la nueva geometría en el origen local (0, 0)
        Rectangle localRect{ 0, 0, width, height };
        
        // Asumiendo que CreateCustomRoundedGeometry devuelve un puntero crudo recién instanciado
        ID2D1PathGeometry* newGeometry = graphics->CreateCustomRoundedGeometry(localRect, radii);
        
        if (newGeometry) 
        {
            // Usamos Attach porque la función de creación ya deja el RefCount en 1.
            // Si la función devolviera un ComPtr, simplemente lo asignaríamos.
            m_geometry.Attach(newGeometry);
            
            // Actualizamos los parámetros de validación
            m_lastWidth = width;
            m_lastHeight = height;
            m_lastRadii = radii;
        }

        return m_geometry.Get();
    }
}
