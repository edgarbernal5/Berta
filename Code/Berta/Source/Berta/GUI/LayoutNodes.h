/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LAYOUT_NODES_HEADER
#define BT_LAYOUT_NODES_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Controls/Panel.h"
#include "Berta/Controls/Form.h"
#include "Berta/Controls/TabBar.h"

#include <unordered_map>
#include <string>
#include <variant>

namespace Berta
{
    struct Window;
    class TabBar;
    class Form;

    enum class LayoutNodeType
    {
        Container,
        Leaf,
        Splitter,
        Dock,
        DockPane,
        DockPaneTab,
    };

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
        std::vector<WindowArea> m_windows; //TODO: quitar el vector y hacer una sola instancia por nodo. No se si aplica al Grid Layout (no se ha implementado por el momento)
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
            isPercentage = false;
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

        LayoutNode(LayoutNodeType type);
        virtual ~LayoutNode() = default;

        virtual void AddWindow(Window* window) {};
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
        
        void RemoveProperty(const std::string& key)
        {
            auto it = m_properties.find(key);
            if (it != m_properties.end())
            {
                m_properties.erase(it);
            }
        }

        size_t GetIndex() const;

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

        LayoutNodeType GetType() const
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

        void SetAreaWithPercentage(const Rectangle& newSize, const Size& parentSize, Size fixedSize, size_t splitterCount)
        {
            auto newArea = newSize;
            fixedSize *= (uint32_t)splitterCount;
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

        virtual void CalculateAreas() = 0;

        LayoutNode* Find(const std::string& id);
        LayoutNode* FindFirst(LayoutNodeType nodeType);

        void SetParentWindow(Window* window)
        {
            SetParentWindow(this, window);
        }

        LayoutNode* GetParentNode() const
        {
            return m_parentNode;
        }
        
        LayoutNode* GetPrev() const
        {
            //return m_prevNode;
            if (!m_parentNode)
                return nullptr;

            for (size_t i = 0; i < m_parentNode->m_children.size(); i++)
            {
                if (m_parentNode->m_children[i]->GetNext() == this)
                {
                    return m_parentNode->m_children[i].get();
                }
            }
            return nullptr;
        }

        LayoutNode* GetNext() const
        {
            return m_nextNode;
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
        LayoutNode* FindFirst(LayoutNodeType nodeType, LayoutNode* node);

        std::string m_id;
        Rectangle m_area;

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

        LayoutNodeType m_type{ LayoutNodeType::Container };
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

    protected:
        ContainerLayoutNode(LayoutNodeType type);

    private:
        bool m_isVertical{ false };
    };

    class LeafLayoutNode : public LayoutNode
    {
    public:
        LeafLayoutNode();

        void AddWindow(Window* window) override;
        void CalculateAreas() override;

    private:
        Window* m_window{ nullptr };
    };

    class SplitterLayoutNode : public LayoutNode
    {
    public:
        const static int Size = 4;
    public:
        SplitterLayoutNode(bool isVertical);
        
        void CalculateAreas() override;
        void SetOrientation(bool isVertical);

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

    struct PaneInfo
    {
        bool ShouldShowCloseButton() const
        {
            return showCaption && showCloseButton;
        }

        std::string id;
        bool showCaption{ true };
        bool showCloseButton{ true };
    };

    class DockLayoutNode : public LayoutNode
    {
    public:
        DockLayoutNode();

        void CalculateAreas() override;
    };

    class DockEventsNotifier
    {
    public:
        virtual ~DockEventsNotifier() = default;

        virtual void NotifyFloat() = 0;
        virtual void NotifyMove() = 0;
        virtual void NotifyMoveStopped() = 0;
        virtual void RequestClose() = 0;
    };

    constexpr int DockAreaCaptionButtonSize = 14;

    class DockAreaCaptionReactor : public ControlReactor
    {
    public:
        void Update(Graphics& graphics) override;

        void MouseDown(Graphics& graphics, const ArgMouse& args) override;
        void MouseMove(Graphics& graphics, const ArgMouse& args) override;
        void MouseUp(Graphics& graphics, const ArgMouse& args) override;
        void Resize(Graphics& graphics, const ArgResize& args) override;

        enum class State
        {
            None,
            Pressed,
            Hovered
        };

        PaneInfo* m_paneInfo{ nullptr };
        bool m_mouseDownCloseButton{ false };
        bool m_clickedCloseButton{ false };
        Rectangle m_buttonRect{};
        State m_buttonStatus{ State::None };
    };

    class DockAreaCaption : public Control<DockAreaCaptionReactor>
    {
    public:
        DockAreaCaption() = default;

        void SetPaneInfo(PaneInfo* paneInfo);
        bool WasPressedCloseButton() const;
        bool HaveClickedCloseButton() const;
    };

    class DockArea : public Control<ControlReactor>
    {
    public:
        DockArea() = default;

        void AddTab(const std::string& id, ControlBase* control);
        void Create(Window* parent, PaneInfo* paneInfo);
        void Dock();
        int GetTabSelectedIndex() const;

        struct MouseInteraction
        {
            bool m_dragStarted{ false };
            bool m_hasChanged{ false };
            Point m_dragStartPos{ };
            Point m_dragStartLocalPos{ };
        };

        bool IsFloating() const
        {
            return m_nativeContainer != nullptr;
        }

        MouseInteraction m_mouseInteraction;
        uint32_t m_savedDPI{ 0 };
        Window* m_hostWindow{ nullptr };
        DockEventsNotifier* m_eventsNotifier{ nullptr };
        std::unique_ptr<Form> m_nativeContainer;
        std::unique_ptr<DockAreaCaption> m_caption;
        std::unique_ptr<TabBar> m_tabBar;
        std::vector<ControlBase*> m_tabBarPanels;

        PaneInfo* m_paneInfo{ nullptr };
    };

    class DockPaneLayoutNode : public LayoutNode, public DockEventsNotifier
    {
    public:
        DockPaneLayoutNode();

        void AddTab(const std::string& id, ControlBase* control);
        void AddPane(DockPaneLayoutNode* paneNode);
        void AddWindow(Window* window) override;
        void CalculateAreas() override;

        void NotifyFloat() override;
        void NotifyMove() override;
        void NotifyMoveStopped() override;
        void RequestClose() override;

        LayoutDockPaneEventsNotifier* m_dockLayoutEvents{ nullptr };
        std::unique_ptr<DockArea> m_dockArea;
        std::string m_paneId;

    protected:
    };

    class DockPaneTabLayoutNode : public LayoutNode
    {
    public:
        DockPaneTabLayoutNode();

        void CalculateAreas() override;

        std::string m_tabId;
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