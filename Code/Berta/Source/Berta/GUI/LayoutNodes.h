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

    class LayoutControlContainer
    {
    public:
        struct WindowArea
        {
            Window* window{ nullptr };
            Rectangle area{ };
        };

        LayoutControlContainer() = default;

        void AddWindow(Window* window);
        std::vector<WindowArea>& GetWindowsAreas()
        {
            return m_windows;
        }
    private:
        std::vector<WindowArea> m_windows;
    };

    class LayoutNode
    {
    public:
        using PropertyValue = std::variant<int, double, std::string, bool>;

        LayoutNode() = default;
        LayoutNode(const std::string& id);
        LayoutNode(bool isVertical);

        void AddWindow(Window* window);
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

        std::string GetId()
        {
            return m_id;
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

        virtual void Apply();
        virtual void CalculateAreas();
        void AddChild(std::unique_ptr<LayoutNode>&& child);

        LayoutNode* Find(const std::string& id);
        void SetOrientation(bool isVertical)
        {
            m_isVertical = isVertical;
        }

        std::vector<LayoutControlContainer::WindowArea>& GetWindowsAreas()
        {
            return m_controlContainer.GetWindowsAreas();
        }

    protected:
        LayoutNode* Find(const std::string& id, LayoutNode* node);

        std::string m_id;
        Rectangle m_area;

        std::unordered_map<std::string, PropertyValue> m_properties;
        LayoutControlContainer m_controlContainer;

    private:

        bool m_isVertical{ false };
        std::vector<std::unique_ptr<LayoutNode>> m_children;
    };
}

#endif