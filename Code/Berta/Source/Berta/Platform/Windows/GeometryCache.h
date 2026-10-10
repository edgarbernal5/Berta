/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_GEOMETRY_CACHE_HEADER
#define BT_GEOMETRY_CACHE_HEADER

#ifdef BT_PLATFORM_WINDOWS
#include <unordered_map>
#include <wrl/client.h>

#include "Berta/Core/Colors.h"

namespace Berta
{
    class Graphics;

    struct GeometryKey
    {
        uint32_t Width;
        uint32_t Height;
        CornerRadii Radii;
        float Inset;
        
        bool operator==(const GeometryKey& other) const 
        {
            return Width == other.Width && Height == other.Height && 
                   Radii == other.Radii && Inset == other.Inset;
        }
    };
    
    struct GeometryKeyHasher 
    {
        std::size_t operator()(const GeometryKey& k) const 
        {
            std::size_t seed = 0;
            auto hash_combine = [&seed](std::size_t hashValue) {
                seed ^= hashValue + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            };

            std::hash<float> hasher;
            hash_combine(hasher(static_cast<float>(k.Width)));
            hash_combine(hasher(static_cast<float>(k.Height)));
            hash_combine(hasher(k.Radii.TopLeft));
            hash_combine(hasher(k.Radii.TopRight));
            hash_combine(hasher(k.Radii.BottomRight));
            hash_combine(hasher(k.Radii.BottomLeft));
            hash_combine(hasher(k.Inset));
            
            return seed;
        }
    };
    
    class GeometryCache
    {
    public:
        GeometryCache(ID2D1Factory* factory) : m_factory(factory) {}
        GeometryCache();
        
        void Clear();
        void Initialize(ID2D1Factory* factory);
        ID2D1PathGeometry* GetGeometry(uint32_t width, uint32_t height, const CornerRadii& radii, float inset);
        
    private:
        void BuildRoundedPath(ID2D1GeometrySink* sink, float W, float H, const CornerRadii& radii, float inset);
        
        ID2D1Factory* m_factory{ nullptr };
        std::unordered_map<GeometryKey, Microsoft::WRL::ComPtr<ID2D1PathGeometry>, GeometryKeyHasher> m_cache;
    };
}

#endif
#endif