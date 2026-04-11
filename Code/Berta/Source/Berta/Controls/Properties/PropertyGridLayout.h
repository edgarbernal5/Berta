/*
* MIT License
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_LAYOUT_HEADER
#define BT_PROPERTY_GRID_LAYOUT_HEADER

#include "PropertyGridModel.h"
#include "Berta/Controls/ScrollableView.h"
#include "Berta/GUI/Window.h"
#include "Berta/Paint/Graphics.h" // O tu equivalente para dibujar

#include <memory>

namespace Berta::Internal::PropertyGrid
{
    // Opcional: Podrías inyectar Appearance directamente
    struct LayoutConfig
    {
        uint32_t CategoryHeight{ 22 };
        uint32_t PropertyHeight{ 22 };
        uint32_t LabelWidth{ 120 };
        Color TextColor{ 255, 255, 255 };
    };

    class PropertyGridLayout
    {
    public:
        PropertyGridLayout(Window* owner, const LayoutConfig& config = {});
        ~PropertyGridLayout() = default;

        // Recorre el modelo, suma las alturas de lo que está expandido y avisa al scroll
        void CalculateLayout(const PropertyGridModel& model);

        // Dibuja aplicando el offset de m_scrollableView
        void Draw(Graphics& graphics, const PropertyGridModel& model);

        // Expuesto públicamente para que el Reactor enrute eventos (MouseWheel, etc)
        ScrollableView* m_scrollableView{ nullptr };

        // Permite actualizar la configuración visual en caliente
        void SetConfig(const LayoutConfig& config) { m_config = config; }
        [[nodiscard]] const LayoutConfig& GetConfig() const { return m_config; }

    private:
        Window* m_owner{ nullptr };
        std::unique_ptr<ScrollableView> m_internalScrollManager;
        LayoutConfig m_config;
    };
}

#endif