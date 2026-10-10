#include "btpch.h"
#include "GeometryCache.h"

#include "Berta/Paint/Graphics.h"

namespace Berta
{
    GeometryCache::GeometryCache()
    {
        m_factory = DirectX::D2DModule::GetInstance().GetFactory();
    }

    void GeometryCache::Clear()
    {
        m_cache.clear();
    }

    void GeometryCache::Initialize(ID2D1Factory* factory)
    {
        m_factory = factory;
    }

    ID2D1PathGeometry* GeometryCache::GetGeometry(uint32_t width, uint32_t height, const CornerRadii& radii, float inset)
    {
        // 1. Agregamos el inset a la llave del caché para diferenciar 
        // la geometría de recorte (inset = 0) de la geometría de borde (inset > 0)
        GeometryKey key{ width, height, radii, inset };
    
        auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            return it->second.Get(); // Hit del caché
        }

        // Miss del caché: Creamos la nueva geometría
        Microsoft::WRL::ComPtr<ID2D1PathGeometry> newGeometry;
        if (SUCCEEDED(m_factory->CreatePathGeometry(&newGeometry)))
        {
            Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
            if (SUCCEEDED(newGeometry->Open(&sink)))
            {
                // 2. Trazamos el path pasando el parámetro inset
                BuildRoundedPath(sink.Get(), (float)width, (float)height, radii, inset);
                sink->Close();
            }
        }

        // Guardamos en el caché y retornamos
        m_cache[key] = newGeometry;
        return newGeometry.Get();
    }

    void GeometryCache::BuildRoundedPath(ID2D1GeometrySink* sink, float W, float H, const CornerRadii& radii, float inset)
    {
        // Calculamos los verdaderos límites del dibujo
        float left = inset;
        float top = inset;
        float right = W - inset;
        float bottom = H - inset;

        // Ajustamos los radios (si el radio es menor que el inset, se vuelve 0/recto para evitar artefactos visuales)
        float tl = std::max<float>(0.0f, radii.TopLeft - inset);
        float tr = std::max<float>(0.0f, radii.TopRight - inset);
        float br = std::max<float>(0.0f, radii.BottomRight - inset);
        float bl = std::max<float>(0.0f, radii.BottomLeft - inset);

        sink->BeginFigure(D2D1::Point2F(left + tl, top), D2D1_FIGURE_BEGIN_FILLED);
    
        sink->AddLine(D2D1::Point2F(right - tr, top));
        if (tr > 0)
            sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(right, top + tr), D2D1::SizeF(tr, tr), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
        
        sink->AddLine(D2D1::Point2F(right, bottom - br));
        if (br > 0)
            sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(right - br, bottom), D2D1::SizeF(br, br), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

        sink->AddLine(D2D1::Point2F(left + bl, bottom));
        if (bl > 0)
            sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(left, bottom - bl), D2D1::SizeF(bl, bl), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

        sink->AddLine(D2D1::Point2F(left, top + tl));
        if (tl > 0)
            sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(left + tl, top), D2D1::SizeF(tl, tl), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    }
}
