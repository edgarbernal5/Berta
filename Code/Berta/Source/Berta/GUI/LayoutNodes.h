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
    struct Window;

    class NodeControlContainer
    {
    public:
        NodeControlContainer() = default;

    private:
        std::vector<Window*> m_fields;
    };

    class LayoutNode
    {
    public:
        using PropertyValue = std::variant<int, double, std::string, bool>;

        LayoutNode() = default;
        LayoutNode(const std::string& id);

        void SetProperty(const std::string& key, const PropertyValue& value)
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
        }

        void SetId(const std::string& id)
        {
            m_id = id;
        }

        Rectangle GetArea() const
        {
            return m_area;
        }

        void SetArea(const Rectangle& newSize)
        {
            m_area = newSize;
        }

        virtual void Apply() = 0;
        virtual void CalculateAreas() = 0;

    protected:
        std::string m_id;
        Rectangle m_area;

        std::unordered_map<std::string, PropertyValue> m_properties;
        NodeControlContainer m_controlContainer;
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

        void Apply() override;
        void CalculateAreas() override;
    private:

        bool m_isVertical{ false };
        std::vector<std::unique_ptr<LayoutNode>> m_children;
    };
}

#endif