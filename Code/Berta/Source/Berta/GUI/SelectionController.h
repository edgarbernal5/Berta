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
    template <typename T>
    class SelectionController
    {
    public:
        using RangeResolver = std::function<std::vector<T>(const T& anchor, const T& current)>;
        
    public:
        SelectionController() = default;
        
        bool Select(const T& item, bool ctrlPressed, bool shiftPressed, const RangeResolver& resolver)
        {
            if (!m_isMultiSelect)
            {
                if (m_selectedItems.size() == 1 && m_selectedItems.find(item) != m_selectedItems.end())
                {
                    return false;
                }

                m_selectedItems.clear();
                m_selectedItems.insert(item);
                m_anchorItem = item;
                return true;
            }
            
            bool selectionChanged = true;
            if (shiftPressed && m_anchorItem.has_value())
            {
                if (!ctrlPressed)
                {
                    m_selectedItems.clear();
                }

                std::vector<T> range = resolver(m_anchorItem.value(), item);
                for (const auto& rangeItem : range)
                {
                    m_selectedItems.insert(rangeItem);
                }
            }
            else if (ctrlPressed)
            {
                if (m_selectedItems.find(item) != m_selectedItems.end())
                {
                    m_selectedItems.erase(item);
                }
                else
                {
                    m_selectedItems.insert(item);
                }
            
                m_anchorItem = item;
            }
            else
            {
                if (m_selectedItems.size() == 1 && *m_selectedItems.begin() == item)
                {
                    selectionChanged = false; 
                }
                else
                {
                    m_selectedItems.clear();
                    m_selectedItems.insert(item);
                }
        
                m_anchorItem = item;
            }

            return selectionChanged;
        }
        
        bool SelectBatch(const std::vector<T>& items, bool append)
        {
            if (!m_isMultiSelect)
            {
                return false;
            }

            bool changed = false;
    
            // Si no estamos sumando a la selección existente (sin Ctrl), limpiamos primero
            if (!append)
            {
                if (!m_selectedItems.empty())
                {
                    m_selectedItems.clear();
                    changed = true;
                }
            }

            // Insertar los nuevos nodos
            for (const auto& item : items)
            {
                // En C++17 'insert' de un set devuelve un std::pair<iterator, bool>
                auto [it, inserted] = m_selectedItems.insert(item);
                if (inserted) 
                {
                    changed = true;
                }
            }
    
            // Actualizar el ancla para que funcione el Shift después de esta selección
            if (!items.empty()) 
            {
                m_anchorItem = items.back();
            }

            return changed;
        }
        
        void ClearSnapshot()
        {
            m_snapshotItems.clear();
            m_snapshotAnchor.reset();
        }
        
        void SaveSnapshot()
        {
            m_snapshotItems = m_selectedItems;
            m_snapshotAnchor = m_anchorItem;
        }

        void RestoreSnapshot()
        {
            m_selectedItems = m_snapshotItems;
            m_anchorItem = m_snapshotAnchor;
        }
        
        bool ApplyLassoSelection(const std::vector<T>& lassoedItems, bool ctrlPressed)
        {
            m_selectedItems = m_snapshotItems;
            
            for (const auto& item : lassoedItems)
            {
                if (ctrlPressed)
                {
                    if (m_snapshotItems.find(item) != m_snapshotItems.end())
                    {
                        m_selectedItems.erase(item);
                    }
                    else
                    {
                        m_selectedItems.insert(item);
                    }
                }
                else
                {
                    m_selectedItems.insert(item);
                }
            }
            return true;
        }
        
        void SetSelected(const T& item, bool selected)
        {
            if (selected)
            {
                if (!m_isMultiSelect)
                {
                    m_selectedItems.clear();
                }
                m_selectedItems.insert(item);
                m_anchorItem = item;
            }
            else
            {
                m_selectedItems.erase(item);
            }
        }
        
        void Clear()
        {
            m_selectedItems.clear();
            m_anchorItem.reset();
            ClearSnapshot();
        }
        
        void SetMultiSelect(bool enabled)
        {
            m_isMultiSelect = enabled;
            
            if (!m_isMultiSelect && m_selectedItems.size() > 1)
            {
                T firstItem = *m_selectedItems.begin();
                m_selectedItems.clear();
                m_selectedItems.insert(firstItem);
                m_anchorItem = firstItem;
            }
        }
        bool IsMultiSelect() const { return m_isMultiSelect; }
                
        bool HasSelectionChangedSinceSnapshot() const
        {
            return m_selectedItems != m_snapshotItems;
        }

        bool IsSelected(const T& item) const
        {
            return m_selectedItems.find(item) != m_selectedItems.end();
        }
        bool IsEmpty() const { return m_selectedItems.empty(); }
        size_t GetCount() const { return m_selectedItems.size(); }
        
        std::vector<T> GetSelectedItems() const
        {
            return std::vector<T>(m_selectedItems.begin(), m_selectedItems.end());
        }

    private:
        std::unordered_set<T> m_selectedItems;
        std::unordered_set<T> m_snapshotItems;
        
        bool m_isMultiSelect{ true };
        
        std::optional<T> m_anchorItem;
        std::optional<T> m_snapshotAnchor;
    };
}

#endif