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
#include "Berta/GUI/Layouts/BasicTypes.h"

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

    class SplitterLayoutControl : public Panel
    {
    public:
        SplitterLayoutControl() = default;
        SplitterLayoutControl(Window* parent, const Rectangle& rectangle = {}, bool visible = true);
    };

    class LayoutNode
    {
    public:

        LayoutNode(LayoutNodeType type);
        virtual ~LayoutNode() = default;

        virtual void AddWindow(Window* window) {}

        void SetProperty(std::string_view name, PropertyValue value)
        {
            m_properties[std::string(name)] = std::move(value);
        }

        template<typename T>
        [[nodiscard]] const T& GetProperty(std::string_view name) const
        {
            auto it = m_properties.find(name);
            if (it == m_properties.end())
            {
                throw std::runtime_error("Propiedad no encontrada en el nodo.");
            }
            return std::get<T>(it->second);
        }
        
        template<typename T>
        [[nodiscard]] T* TryGetProperty(std::string_view name) noexcept {
            if (auto it = m_properties.find(name); it != m_properties.end())
            {
                return std::get_if<T>(&it->second);
            }
            return nullptr;
        }
        template <typename T>
        bool HasProperty(const std::string& key) const
        {
            auto it = m_properties.find(key);
            return (it != m_properties.end() && std::holds_alternative<T>(it->second));
        }
        
        void RemoveProperty(std::string_view name)
        {
            m_properties.erase(std::string(name));
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

        Window* GetOwnerWindow() const
        {
            return m_ownerWindow;
        }

        void SetArea(const Rectangle& newSize)
        {
            m_area = newSize;
        }

        void SetAreaWithPercentage(Rectangle& newArea, const Size& parentSize, Size fixedSize, bool isVertical)
        {
            const std::string_view propName = isVertical ? "Height" : "Width";
    
            uint32_t parentDim = isVertical ? parentSize.Height : parentSize.Width;
            uint32_t fixedDim  = isVertical ? fixedSize.Height  : fixedSize.Width;
            uint32_t remainDim = parentDim - fixedDim;
    
            uint32_t& areaDim  = isVertical ? newArea.Height    : newArea.Width;

            // 1. Calculamos y actualizamos el peso dinámico (proporción del espacio remanente)
            double newWeight = static_cast<double>(areaDim) / remainDim;
            SetProperty("LayoutWeight", Berta::Dimension{ newWeight, Berta::DimensionUnit::Percentage });

            // 2. Si el nodo además tenía una propiedad estática del script original, 
            // la actualizamos en cascada para mantener coherencia si se redimensiona la ventana de Bruno
            if (auto* staticDim = TryGetProperty<Berta::Dimension>(propName))
            {
                staticDim->unit = Berta::DimensionUnit::Percentage;
                staticDim->value = (static_cast<double>(areaDim) / parentDim) * 100.0;
        
                areaDim = static_cast<uint32_t>((staticDim->value * parentDim) / 100.0);
            }
            else
            {
                areaDim = static_cast<uint32_t>(newWeight * remainDim);
            }

            m_area = newArea;
        }

        virtual void CalculateAreas() = 0;

        LayoutNode* Find(std::string_view id);
        LayoutNode* FindFirst(LayoutNodeType nodeType);

        void SetOwnerWindow(Window* owner)
        {
            SetOwnerWindow(this, owner);
        }

        LayoutNode* GetParentNode() const
        {
            return m_parentNode;
        }
        
        LayoutNode* GetPrev() const
        {
            return m_prevNode;
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
        
        virtual void EnterSizeMove();
        virtual void ExitSizeMove();
        
        std::map<std::string, PropertyValue, std::less<>> m_properties;
        std::vector<std::unique_ptr<LayoutNode>> m_children;

        Number m_fixedWidth;
        Number m_fixedHeight;

    protected:
        LayoutNode* Find(std::string_view id, LayoutNode* node);
        LayoutNode* FindFirst(LayoutNodeType nodeType, LayoutNode* node);

        std::string m_id;
        Rectangle m_area;

        Window* m_ownerWindow{ nullptr };
        LayoutNode* m_prevNode{ nullptr };
        LayoutNode* m_nextNode{ nullptr };
        LayoutNode* m_parentNode{ nullptr };

    private:
        void SetOwnerWindow(LayoutNode* node, Window* window)
        {
            if (node == nullptr)
            {
                return;
            }

            node->m_ownerWindow = window;

            for (auto& childNode : node->m_children)
            {
                SetOwnerWindow(childNode.get(), window);
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
        void ProcessFixedChildren(const Rectangle& parentArea, Rectangle& remainArea, 
                                  std::vector<Rectangle>& areas, std::vector<bool>& markedChildren, 
                                  int& fixedNodesCount, float dpi);

        void ProcessDynamicChildren(const Rectangle& parentArea, const Rectangle& remainArea, 
                                    const std::vector<Rectangle>& areas, const std::vector<bool>& markedChildren, 
                                    int fixedNodesCount, float dpi);
        
        bool m_isVertical{ false };
    };

    class LeafLayoutNode : public LayoutNode
    {
    public:
        LeafLayoutNode();

        void AddWindow(Window* window) override;
        void CalculateAreas() override;

        void EnterSizeMove() override;
        void ExitSizeMove() override;
        
    private:
        Window* m_window{ nullptr };
    };

    class SplitterLayoutNode : public LayoutNode
    {
    public:
        static constexpr int SizeInPixels = 5;
        
    public:
        SplitterLayoutNode(bool isVertical);
        
        void CalculateAreas() override;
        void SetOrientation(bool isVertical);

    private:
        void EnsureControlCreated();

        void OnMouseDown(const ArgMouse& args);
        void OnMouseMove(const ArgMouse& args);
        void OnMouseUp();
        void OnMouseEnter();
        void OnMouseLeave();
        
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
        [[nodiscard]] bool ShouldShowCloseButton() const
        {
            return showCaption && showCloseButton;
        }

        std::string id;
        bool showCaption{ true };
        bool showCloseButton{ true };
        bool showTabBar{ true };
    };

    class DockLayoutNode : public LayoutNode
    {
    public:
        DockLayoutNode();

        void CalculateAreas() override;
    };

    constexpr int DOCK_AREA_CAPTION_BUTTON_SIZE = 14;

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

    class DockAreaCaption : public Control<Category::ControlTag, DockAreaCaptionReactor>
    {
    public:
        DockAreaCaption() = default;

        void SetPaneInfo(PaneInfo* paneInfo);
        bool WasPressedCloseButton() const;
        bool HaveClickedCloseButton() const;
    };

    class DockArea : public Control<Category::PanelTag, ControlReactor>
    {
    public:
        DockArea() = default;

        void AddTab(const std::string& id, std::unique_ptr<ControlBase> control);
        void Create(Window* parent, PaneInfo* paneInfo);
        void Dock();
        std::optional<size_t> GetTabSelectedIndex() const;
        
        struct PanelDock
        {
            std::unique_ptr<ControlBase> ControlPtr;
        };
        
        struct MouseInteraction
        {
            bool m_dragStarted{ false };
            bool m_hasChanged{ false };
            Point m_dragStartPos{ };
            Point m_dragStartLocalPos{ };
            Point m_dragStartCaptionPos{ };
            uint32_t m_savedDPI{ 0 };
            
            std::optional<size_t> m_draggedTabIndex;
        };

        bool IsFloating() const
        {
            return m_nativeContainer != nullptr;
        }
        
        void MakeFloating(const Rectangle& rect);
        
        void OnTabMouseDown(size_t tabIndex, const Point& mouseScreenPos);
        void OnTabMouseMove(const Point& mouseScreenPos);
        void OnTabMouseUp(const Point& mouseScreenPos);
        
        MouseInteraction m_mouseInteraction;
        Window* m_hostWindow{ nullptr };
        DockPaneLayoutNode* m_ownerDockPane{ nullptr };
        std::unique_ptr<Form> m_nativeContainer;
        std::unique_ptr<DockAreaCaption> m_caption;
        std::unique_ptr<TabBar> m_tabBar;
        std::vector<PanelDock> m_tabBarPanels;
        
        PaneInfo* m_paneInfo{ nullptr };
    };

    struct ArgFloatTab
    {
        DockPaneTabLayoutNode* tabNode;
        Point mouseScreenPos;
    };
    
    struct DockPaneEvents 
    {
        Event<DockPaneLayoutNode*> OnFloat;
        Event<DockPaneLayoutNode*> OnMoveStarted;
        Event<DockPaneLayoutNode*> OnMove;
        Event<DockPaneLayoutNode*> OnMoveStopped;
        Event<DockPaneLayoutNode*> OnRequestClose;
        Event<ArgFloatTab> OnFloatTab;
    };
    
    class DockPaneLayoutNode : public LayoutNode
    {
    public:
        DockPaneLayoutNode();

        void AddTab(std::string_view id, std::unique_ptr<ControlBase> control);
        void AppendPane(DockPaneLayoutNode* paneNode);
        void AddWindow(Window* window) override;
        void CalculateAreas() override;

        void NotifyFloat();
        void NotifyMove();
        void NotifyMoveStarted();
        void NotifyMoveStopped();
        void RequestClose();
        
        std::unique_ptr<DockArea> m_dockArea;
        std::string m_paneId;
        
        DockPaneEvents Events;
    protected:
    };

    class DockPaneTabLayoutNode : public LayoutNode
    {
    public:
        DockPaneTabLayoutNode();

        void CalculateAreas() override;

        void EnterSizeMove() override;
        void ExitSizeMove() override;
        
        std::string m_tabId;
    };
}

#endif