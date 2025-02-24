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
        std::vector<WindowArea> m_windows; //TODO: quitar el vector y hacer una sola instancia por nodo.
    };

    struct Margin
    {

    };

    struct Number
    {
        using NumberValue = std::variant<int, double>;

        Number() = default;
        Number(NumberValue value) : scalar(value), isPercentage(false) {}
        Number(NumberValue value, bool percentage) : scalar(value), isPercentage(percentage) {}

        template<class T>
        T GetValue(float dpiFactor);

        template<class T>
        T GetValue();

        NumberValue scalar{ 0 };
        bool isPercentage{ false };
    };

    class LayoutNode
    {
    public:
        using PropertyValue = std::variant<Number, std::string, bool>;

        enum class Type
        {
            Container,
            Leaf,
            Splitter
        };

        LayoutNode(Type type);

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

        template <typename T>
        bool HasProperty(const std::string& key) const
        {
            auto it = m_properties.find(key);
            if (it != m_properties.end() && std::holds_alternative<T>(it->second))
            {
                return true;
            }
            return false;
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

        Type GetType() const
        {
            return m_type;
        }

        void SetArea(const Rectangle& newSize)
        {
            m_area = newSize;
        }

        void Apply();
        virtual void CalculateAreas() = 0;

        LayoutNode* Find(const std::string& id);

        std::vector<LayoutControlContainer::WindowArea>& GetWindowsAreas()
        {
            return m_controlContainer.GetWindowsAreas();
        }

        void SetParentWindow(Window* window)
        {
            SetParentWindow(this, window);
        }
        LayoutNode* GetParentNode() const
        {
            return m_parentNode;
        }
        void SetParentNode(LayoutNode* node)
        {
            m_parentNode = node;
        }
        std::unordered_map<std::string, PropertyValue> m_properties;
        std::vector<std::unique_ptr<LayoutNode>> m_children;

    protected:
        LayoutNode* Find(const std::string& id, LayoutNode* node);

        std::string m_id;
        Rectangle m_area;

        LayoutControlContainer m_controlContainer;
        Window* m_parentWindow{ nullptr };
        LayoutNode* m_parentNode{ nullptr };

    private:
        void SetParentWindow(LayoutNode* node, Window* window)
        {
            if (node == nullptr)
                return;

            node->m_parentWindow = window;

            for (auto& childNode : node->m_children)
            {
                SetParentWindow(childNode.get(), window);
            }
        }
        Type m_type{ Type::Container };

    };

    class ContainerLayoutNode : public LayoutNode
    {
    public:
        ContainerLayoutNode(bool isVertical);

        void SetOrientation(bool isVertical)
        {
            m_isVertical = isVertical;
        }

        void AddChild(std::unique_ptr<LayoutNode>&& child);

        void CalculateAreas() override;

    private:
        bool m_isVertical{ false };
    };

    class LeafLayoutNode : public LayoutNode
    {
    public:
        LeafLayoutNode();

        void CalculateAreas() override;
    };

    class SplitterLayoutNode : public LayoutNode
    {
    public:
        SplitterLayoutNode();
        
        void CalculateAreas() override;
    };

    template<class T>
    inline T Number::GetValue(float dpiFactor)
    {
        if (std::holds_alternative<T>(scalar))
        {
            return static_cast<T>(static_cast<double>(std::get<T>(scalar) * dpiFactor));
        }

        return T{ 0 };
    }

    template<class T>
    inline T Number::GetValue()
    {
        if (std::holds_alternative<int>(scalar))
        {
            return static_cast<T>(std::get<int>(scalar));
        }
        if (std::holds_alternative<double>(scalar))
        {
            return static_cast<T>(std::get<double>(scalar));
        }
        return T{ 0 };
    }
}

#endif