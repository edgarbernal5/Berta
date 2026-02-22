/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LASSO_SELECTION_HEADER
#define BT_LASSO_SELECTION_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Paint/Graphics.h"

namespace Berta
{
    class LassoSelection
    {
    public:
        LassoSelection() = default;

        // Inicia el recuadro en una posición
        void Start(const Point& startPosition);

        // Actualiza la posición y retorna 'true' si el recuadro cambió de tamaño
        bool Update(const Point& currentPosition);

        // Termina la selección
        void End();

        // Estado
        bool IsActive() const { return m_isActive; }

        // Retorna el rectángulo normalizado (siempre con Width y Height positivos)
        Rectangle GetRect() const;

        // Dibuja el recuadro visual
        void Draw(Graphics& graphics, const Color& fillColor, const Color& borderColor) const;

    private:
        bool m_isActive{ false };
        Point m_startPos{ 0, 0 };
        Point m_currentPos{ 0, 0 };
    };
}

#endif