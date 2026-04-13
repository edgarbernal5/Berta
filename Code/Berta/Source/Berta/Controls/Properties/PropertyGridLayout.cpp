/*
* MIT License
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridLayout.h"

#include "PropertyGridFieldBase.h" // Para poder llamar a field->Draw()

#include "Berta/GUI/ScrollableView.h"

namespace Berta::Internal::PropertyGrid
{
    /*PropertyGridLayout::PropertyGridLayout(Window* owner, const LayoutConfig& config)
        : m_owner(owner), m_config(config)
    {
        m_internalScrollManager = std::make_unique<ScrollableView>(m_owner);
        m_scrollableView = m_internalScrollManager.get();

        // Configuración inicial del scroll
        m_scrollableView->SetScrollStep(m_config.CategoryHeight, 0);
        
        m_scrollableView->SetOnScrollChange([this]() {
            if (m_owner) {
                // Invalidamos para repintar al scrollear
                //GUI::InvalidateWindowArea(m_owner, m_scrollableView->GetClientArea()); 
            }
        });
    }

    void PropertyGridLayout::CalculateLayout(const PropertyGridModel& model)
    {
        uint32_t totalHeight = 0;
        uint32_t maxWidth = m_scrollableView->GetClientArea().Width; // O un ancho fijo si no quieres scroll horizontal

        for (const auto& category : model.GetCategories())
        {
            // La categoría siempre ocupa espacio
            totalHeight += m_config.CategoryHeight;

            // Si está expandida, sumamos el espacio de sus propiedades
            if (category.m_isExpanded)
            {
                // Asumiendo que todas las propiedades tienen el mismo alto. 
                // Si tienen alto dinámico, deberás preguntar a cada item.
                totalHeight += static_cast<uint32_t>(category.m_properties.size()) * m_config.PropertyHeight;
            }
        }

        // Le informamos al scroll el tamaño total del lienzo interno
        m_scrollableView->SetContentSize({ static_cast<int>(maxWidth), static_cast<int>(totalHeight) });
    }

    void PropertyGridLayout::Draw(Graphics& graphics, const PropertyGridModel& model)
    {
        Point offset = m_scrollableView->GetScrollOffset();
        Rectangle visibleRect = m_scrollableView->GetVisibleRect();

        // Empezamos a dibujar en negativo según el scroll
        int currentY = -offset.Y; 
        int width = visibleRect.Width;

        for (const auto& category : model.GetCategories())
        {
            // --- DIBUJAR CATEGORÍA ---
            Rectangle catArea{ 0, currentY, width, static_cast<int>(m_config.CategoryHeight) };
            
            // Culling (Optimización: solo dibujamos si está dentro de la pantalla)
            if (catArea.Y + catArea.Height > 0 && catArea.Y < visibleRect.Height)
            {
                // Aquí dibujas el fondo de la categoría, el triángulo de expandir y el texto
                // graphics.FillRectangle(catArea, Color::DarkGray);
                // graphics.DrawString({15, currentY}, category.m_name, m_config.TextColor);
            }
            
            currentY += m_config.CategoryHeight;

            // --- DIBUJAR PROPIEDADES ---
            if (category.m_isExpanded)
            {
                for (const auto& item : category.m_properties)
                {
                    Rectangle propArea{ 0, currentY, width, static_cast<int>(m_config.PropertyHeight) };

                    if (propArea.Y + propArea.Height > 0 && propArea.Y < visibleRect.Height)
                    {
                        // Inyectamos el área calculada a la propiedad para que se dibuje
                        item.field->Draw(graphics, propArea, m_config.LabelWidth, m_config.TextColor);
                    }
                    currentY += m_config.PropertyHeight;
                }
            }
        }
    }*/
}
