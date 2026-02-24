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
#include <functional>
#include <optional>

namespace Berta
{
    class SelectionController
    {
    public:
        using T = size_t;
        using RangeResolver = std::function<std::vector<T>(const T& anchor, const T& current)>;
        
    public:
        SelectionController() = default;

        bool Select(size_t index, bool ctrlPressed, bool shiftPressed, const RangeResolver& resolver);

        void SaveSnapshot();
        bool ApplyLassoSelection(const std::vector<T>& lassoedItems, bool ctrlPressed);

        // --- Modificación Manual ---
        void SetSelected(size_t index, bool selected);
        void Clear();
        void SelectAll(size_t totalItems);

        // --- Consultas ---
        bool IsSelected(size_t index) const;
        bool IsEmpty() const { return m_selectedItems.empty(); }
        size_t GetCount() const { return m_selectedItems.size(); }
        
        // Retorna un vector ordenado de los índices seleccionados
        std::vector<size_t> GetSelectedIndices() const;

    private:
        std::unordered_set<size_t> m_selectedItems;
        std::unordered_set<size_t> m_snapshotItems; // Para el recuadro de selección
        
        // El "Ancla" guarda el índice desde donde se empezó a hacer Shift+Clic
        std::optional<T> m_anchorItem;
    };
}

#endif