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
        
        void Start(const Point& startPosition);
        bool Update(const Point& currentPosition);
        void End();

        bool IsActive() const { return m_isActive; }

        Rectangle GetRect() const;

        void Draw(Graphics& graphics, Graphics& selectionBox, const Color& blendColor, const Color& borderColor) const;

    private:
        bool m_isActive{ false };
        Point m_startPos{ 0, 0 };
        Point m_currentPos{ 0, 0 };
    };
}

#endif