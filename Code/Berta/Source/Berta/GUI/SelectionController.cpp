/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "SelectionController.h"

namespace Berta
{
    bool SelectionController::Select(size_t index, bool ctrlPressed, bool shiftPressed, size_t totalItems)
    {
        if (index >= totalItems) return false;

        bool selectionChanged = true;

        if (shiftPressed && m_anchorIndex != static_cast<size_t>(-1))
        {
            // SHIFT + CLIC: Selecciona un rango desde el ancla hasta el índice actual
            if (!ctrlPressed)
            {
                m_selectedIndices.clear();
            }

            size_t start = (std::min)(m_anchorIndex, index);
            size_t end = (std::max)(m_anchorIndex, index);

            for (size_t i = start; i <= end; ++i)
            {
                m_selectedIndices.insert(i);
            }
        }
        else if (ctrlPressed)
        {
            // CTRL + CLIC: Alterna la selección del ítem individual
            if (m_selectedIndices.find(index) != m_selectedIndices.end())
            {
                m_selectedIndices.erase(index);
            }
            else
            {
                m_selectedIndices.insert(index);
            }
            m_anchorIndex = index; // Actualizamos el ancla
        }
        else
        {
            // CLIC NORMAL: Limpia todo y selecciona solo este
            if (m_selectedIndices.size() == 1 && *m_selectedIndices.begin() == index)
            {
                selectionChanged = false; // Ya estaba seleccionado, evitamos repintados inútiles
            }
            else
            {
                m_selectedIndices.clear();
                m_selectedIndices.insert(index);
            }
            m_anchorIndex = index;
        }

        return selectionChanged;
    }

    void SelectionController::SaveSnapshot()
    {
        m_snapshotIndices = m_selectedIndices;
    }

    bool SelectionController::ApplyLassoRange(size_t startIndex, size_t endIndex, bool ctrlPressed)
    {
        // 1. Restaurar al estado antes de arrastrar
        m_selectedIndices = m_snapshotIndices;

        // 2. Aplicar la nueva zona
        for (size_t i = startIndex; i <= endIndex; ++i)
        {
            if (ctrlPressed)
            {
                // Invertimos lo que había en la foto
                if (m_snapshotIndices.find(i) != m_snapshotIndices.end())
                    m_selectedIndices.erase(i);
                else
                    m_selectedIndices.insert(i);
            }
            else
            {
                // Forzamos selección
                m_selectedIndices.insert(i);
            }
        }
        return true; // En un escenario real, podrías comparar si el set cambió para retornar false
    }

    void SelectionController::SetSelected(size_t index, bool selected)
    {
        if (selected)
        {
            m_selectedIndices.insert(index);
            m_anchorIndex = index;
        }
        else
        {
            m_selectedIndices.erase(index);
        }
    }

    void SelectionController::Clear()
    {
        m_selectedIndices.clear();
        m_anchorIndex = static_cast<size_t>(-1);
    }

    void SelectionController::SelectAll(size_t totalItems)
    {
        for (size_t i = 0; i < totalItems; ++i)
        {
            m_selectedIndices.insert(i);
        }
    }

    bool SelectionController::IsSelected(size_t index) const
    {
        return m_selectedIndices.find(index) != m_selectedIndices.end();
    }

    std::vector<size_t> SelectionController::GetSelectedIndices() const
    {
        std::vector<size_t> result(m_selectedIndices.begin(), m_selectedIndices.end());
        std::sort(result.begin(), result.end());
        return result;
    }
}