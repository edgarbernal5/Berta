/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LAYOUT_NODES_HEADER
#define BT_LAYOUT_NODES_HEADER

#include "Berta/Core/BasicTypes.h"

#include <unordered_map>
#include <string>
#include <variant>

namespace Berta
{
    class LayoutNode
    {
    public:
        using PropertyValue = std::variant<int, float, std::string, bool>;

        LayoutNode() = default;
        LayoutNode(const std::string& id);

       /* void SetProperty(const std::string& key, const PropertyValue& value)
        {
            m_properties[key] = value;
        }

        template <typename T>
        T GetProperty(const std::string& key, const T& defaultValue = T()) const
        {
            auto it = m_properties.find(key);
            if (it != m_properties.end() && std::holds_alternative<T>(it->second))
            {
                return std::get<T>(it->second);
            }
            return defaultValue;
        }*/
        void SetId(const std::string& id)
        {
            m_id = id;
        }

    private:
        std::string m_id;
        std::unordered_map<std::string, PropertyValue> m_properties;
    };

    class ContainerLayout : public LayoutNode
    {
    public:
        ContainerLayout(bool isVertical);

        void AddChild(std::unique_ptr<LayoutNode>&& child);
        void SetOrientation(bool isVertical)
        {
            m_isVertical = isVertical;
        }
    private:
        bool m_isVertical{ false };
        std::vector<std::unique_ptr<LayoutNode>> m_children;
    };
}

#endif