/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SELECTION_CONTROLLER_HEADER
#define BT_SELECTION_CONTROLLER_HEADER

#include <unordered_set>
#include <vector>
#include <algorithm>

namespace Berta
{
    class SelectionController
    {
    public:
        SelectionController() = default;

        // --- Interacción con Ratón / Teclado ---
        // Maneja automáticamente la lógica de Ctrl (Toggle) y Shift (Rango)
        bool Select(size_t index, bool ctrlPressed, bool shiftPressed, size_t totalItems);

        // --- Interacción con Lasso (Recuadro) ---
        void SaveSnapshot(); // Guarda el estado antes de empezar a arrastrar
        bool ApplyLassoRange(size_t startIndex, size_t endIndex, bool ctrlPressed);

        // --- Modificación Manual ---
        void SetSelected(size_t index, bool selected);
        void Clear();
        void SelectAll(size_t totalItems);

        // --- Consultas ---
        bool IsSelected(size_t index) const;
        bool IsEmpty() const { return m_selectedIndices.empty(); }
        size_t GetCount() const { return m_selectedIndices.size(); }
        
        // Retorna un vector ordenado de los índices seleccionados
        std::vector<size_t> GetSelectedIndices() const;

    private:
        std::unordered_set<size_t> m_selectedIndices;
        std::unordered_set<size_t> m_snapshotIndices; // Para el recuadro de selección
        
        // El "Ancla" guarda el índice desde donde se empezó a hacer Shift+Clic
        size_t m_anchorIndex{ static_cast<size_t>(-1) }; 
    };
}

#endif