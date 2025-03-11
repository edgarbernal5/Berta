/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LAYOUT_NODES_HEADER
#define BT_LAYOUT_NODES_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Controls/Panel.h"

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
        Number(NumberValue value) : scalar(value), isPercentage(false), hasValue(true) {}
        Number(NumberValue value, bool percentage) : scalar(value), isPercentage(percentage), hasValue(true) {}

        bool HasValue() const
        {
            return hasValue;
        }

        template<class T>
        T GetValue(float dpiFactor);

        template<class T>
        T GetValue();

        void SetValue(double newValue)
        {
            scalar = newValue;
            hasValue = true;
        }

        void SetValue(int newValue)
        {
            scalar = newValue;
            hasValue = true;
        }

        void Reset()
        {
            scalar = 0;
            hasValue = false;
        }

        bool isPercentage{ false };
    private:

        NumberValue scalar{ 0 };
        bool hasValue{ false };
    };

    class SplitterLayoutControl : public Panel
    {
    public:
        SplitterLayoutControl() = default;
        SplitterLayoutControl(Window* parent, const Rectangle& rectangle = {}, bool visible = true);
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

            return (it != m_properties.end() && std::holds_alternative<T>(it->second));
        }

        std::string GetId() const
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
        Window* GetParentWindow() const
        {
            return m_parentWindow;
        }

        void SetArea(const Rectangle& newSize)
        {
            m_area = newSize;
        }

        void SetAreaWithPercentage(const Rectangle& newSize, const Size& parentSize, Size fixedSize)
        {
            auto newArea = newSize;
            if (m_fixedWidth.HasValue())
            {
                auto remainSize = parentSize - fixedSize;

                auto newScalar = static_cast<double>(newSize.Width) / remainSize.Width;
                m_fixedWidth.isPercentage = true;
                m_fixedWidth.SetValue(newScalar);

                newArea.Width = static_cast<uint32_t>(newScalar * remainSize.Width);
                if (HasProperty<Number>("Width"))
                {
                    auto widthProp = GetProperty<Number>("Width");

                    auto newScalar = static_cast<double>(newSize.Width) / parentSize.Width;
                    widthProp.isPercentage = true;
                    widthProp.SetValue(newScalar * 100.0);
                    SetProperty("Width", widthProp);
                }
            }
            if (m_fixedHeight.HasValue())
            {
                auto remainSize = parentSize - fixedSize;

                auto newScalar = static_cast<double>(newSize.Height) / remainSize.Height;
                m_fixedHeight.isPercentage = true;
                m_fixedHeight.SetValue(newScalar);

                newArea.Height = static_cast<uint32_t>(newScalar * remainSize.Height);
                if (HasProperty<Number>("Height"))
                {
                    auto widthProp = GetProperty<Number>("Height");

                    auto newScalar = static_cast<double>(newSize.Height) / parentSize.Height;
                    widthProp.isPercentage = true;
                    widthProp.SetValue(newScalar * 100.0);
                    SetProperty("Height", widthProp);
                }
            }
            m_area = newArea;
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

        void SetPrev(LayoutNode* node)
        {
            m_prevNode = node;
        }

        void SetNext(LayoutNode* node)
        {
            m_nextNode = node;
        }

        void SetParentNode(LayoutNode* node)
        {
            m_parentNode = node;
        }

        std::unordered_map<std::string, PropertyValue> m_properties;
        std::vector<std::unique_ptr<LayoutNode>> m_children;

        Number m_fixedWidth;
        Number m_fixedHeight;

    protected:
        LayoutNode* Find(const std::string& id, LayoutNode* node);

        std::string m_id;
        Rectangle m_area;

        LayoutControlContainer m_controlContainer;
        Window* m_parentWindow{ nullptr };
        LayoutNode* m_prevNode{ nullptr };
        LayoutNode* m_nextNode{ nullptr };
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

        bool GetOrientation() const
        {
            return m_isVertical;
        }

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

    private:
        Point m_mousePositionOffset{};
        Rectangle m_splitterBeginRect{};
        Rectangle m_leftArea{};
        Rectangle m_rightArea{};
        bool m_isSplitterMoving{ false };
        bool m_isVertical{ false };
        ContainerLayoutNode* m_containerNode{ nullptr };
        std::unique_ptr<SplitterLayoutControl> m_splitter;
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