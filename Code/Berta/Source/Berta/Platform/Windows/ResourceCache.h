/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_RESOURCE_CACHE_HEADER
#define BT_RESOURCE_CACHE_HEADER

#ifdef BT_PLATFORM_WINDOWS
#include <unordered_map>
#include <wrl/client.h>

#include "Berta/Core/Colors.h"

namespace Berta
{
    class Graphics;

    struct ColorHasher
    {
        size_t operator()(const Color& c) const
        {
            return static_cast<size_t>(c.GetR()) << 24 | static_cast<size_t>(c.GetG()) << 16 | static_cast<size_t>(c.GetB()) << 8 | static_cast<size_t>(c.GetA());
        }
    };
    
    class ResourceCache
    {
    public:
        ResourceCache(ID2D1RenderTarget* targetRT);
        ResourceCache() = default;
        
        ID2D1SolidColorBrush* GetBrush(const Color& color);
        ID2D1StrokeStyle* GetStrokeStyle(LineStyle style);
        
        ID2D1PathGeometry* GetOrCreateGeometry(const Graphics* graphics, int width, int height, const CornerRadii& radii);
        void InvalidateGeometry();
    private:
        ID2D1RenderTarget* m_targetRT{ nullptr };
        std::unordered_map<Color, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>, ColorHasher> m_brushCache;
        std::unordered_map<LineStyle, Microsoft::WRL::ComPtr<ID2D1StrokeStyle>> m_strokeStyleCache;
        
        Microsoft::WRL::ComPtr<ID2D1PathGeometry> m_geometry;
        int m_lastWidth = -1;
        int m_lastHeight = -1;
        CornerRadii m_lastRadii;
    };
}

#endif
#endif