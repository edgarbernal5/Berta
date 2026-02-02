/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ResourceCache.h"

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
}
