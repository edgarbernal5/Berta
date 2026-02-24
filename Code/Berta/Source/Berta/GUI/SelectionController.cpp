/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "SelectionController.h"

namespace Berta
{
    bool SelectionController::Select(size_t item, bool ctrlPressed, bool shiftPressed, const RangeResolver& resolver)
    {
        bool selectionChanged = true;

        if (shiftPressed && m_anchorItem.has_value())
        {
            // --- SHIFT + CLIC: Selección de Rango ---
        
            // Si no está presionado CTRL al mismo tiempo, limpiamos la selección anterior
            if (!ctrlPressed)
            {
                m_selectedItems.clear();
            }

            // El Control (ListBox/TreeBox) nos resuelve qué elementos hay entre el Ancla y el Clic
            std::vector<T> range = resolver(m_anchorItem.value(), item);
        
            for (const auto& rangeItem : range)
            {
                m_selectedItems.insert(rangeItem);
            }
        }
        else if (ctrlPressed)
        {
            // --- CTRL + CLIC: Alternar Selección ---
        
            if (m_selectedItems.find(item) != m_selectedItems.end())
            {
                m_selectedItems.erase(item); // Si ya estaba, lo quitamos
            }
            else
            {
                m_selectedItems.insert(item); // Si no estaba, lo añadimos
            }
            
            m_anchorItem = item; // El último elemento tocado se convierte en la nueva ancla
        }
        else
        {
            // --- CLIC NORMAL ---
        
            // Optimización visual: si hacemos clic normal en el único elemento que ya
            // estaba seleccionado, no hay cambios reales, evitamos repintar la UI.
            if (m_selectedItems.size() == 1 && *m_selectedItems.begin() == item)
            {
                selectionChanged = false; 
            }
            else
            {
                m_selectedItems.clear();
                m_selectedItems.insert(item);
            }
        
            m_anchorItem = item; // Se establece la nueva ancla
        }

        return selectionChanged;
    }

    void SelectionController::SaveSnapshot()
    {
        m_snapshotItems = m_selectedItems;
    }

    bool SelectionController::ApplyLassoSelection(const std::vector<T>& lassoedItems, bool ctrlPressed)
    {
        // 1. Restaurar al estado antes de arrastrar
        m_selectedItems = m_snapshotItems;

        // 2. Aplicar la nueva zona
        for (const auto& item : lassoedItems)
        {
            if (ctrlPressed)
            {
                // Invertimos lo que había en la foto
                if (m_snapshotItems.find(item) != m_snapshotItems.end())
                    m_selectedItems.erase(item);
                else
                    m_selectedItems.insert(item);
            }
            else
            {
                // Forzamos selección
                m_selectedItems.insert(item);
            }
        }
        return true; // En un escenario real, podrías comparar si el set cambió para retornar false
    }

    void SelectionController::SetSelected(size_t item, bool selected)
    {
        if (selected)
        {
            m_selectedItems.insert(item);
            m_anchorItem = item;
        }
        else
        {
            m_selectedItems.erase(item);
        }
    }

    void SelectionController::Clear()
    {
        m_selectedItems.clear();
        m_anchorItem = static_cast<size_t>(-1);
    }

    void SelectionController::SelectAll(size_t totalItems)
    {
        for (size_t i = 0; i < totalItems; ++i)
        {
            m_selectedItems.insert(i);
        }
    }

    bool SelectionController::IsSelected(size_t index) const
    {
        return m_selectedItems.find(index) != m_selectedItems.end();
    }

    std::vector<size_t> SelectionController::GetSelectedIndices() const
    {
        std::vector<size_t> result(m_selectedItems.begin(), m_selectedItems.end());
        std::sort(result.begin(), result.end());
        return result;
    }
}