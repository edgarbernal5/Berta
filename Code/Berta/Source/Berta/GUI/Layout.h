/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LAYOUT_HEADER
#define BT_LAYOUT_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Controls/Docking/DockPanel.h"
#include "Berta/GUI/EnumTypes.h"
#include "Berta/Controls/Docking/DockIndicatorForm.h"

#include <memory>
#include <unordered_map>
#include <map>
#include <string_view>

#include "Berta/GUI/Layouts/BasicTypes.h"

namespace Berta
{
    struct Window;
    class LayoutNode;
    class DockPaneLayoutNode;
    class DockPaneTabLayoutNode;
    class ControlBase;
    class Form;
    enum class LayoutNodeType;

    struct PaneInfo;

    struct DockIndicator
    {
        DockPosition Position{ DockPosition::Tab };
        std::unique_ptr<DockIndicatorForm> Docker;
    };

    class Layout
    {
    public:
        Layout();
        Layout(Window* owner);
        ~Layout();

        void AddPane(std::string_view paneId);
        void AddPaneTab(std::string_view paneId, std::string_view tabId, std::unique_ptr<ControlBase> control);
        void AddPaneTab(std::string_view paneId, std::string_view tabId, std::unique_ptr<ControlBase> control, std::string_view relativePaneId, DockPosition dockPosition);

        void Apply();
        Layout& Attach(std::string_view fieldId, Window* window);
        void Create(Window* owner);
        void Parse(const std::string& source);

        bool RemoveDockPane(DockPaneLayoutNode* node);
        
    private:
        void WireDockPaneEvents(DockPaneLayoutNode* node);
        
        void HandleFloat(DockPaneLayoutNode* const& node);
        void HandleMove(DockPaneLayoutNode* const& node);
        void HandleMoveStarted(DockPaneLayoutNode* const& node);
        void HandleMoveStopped(DockPaneLayoutNode* const& node);
        void HandleRequestClose(DockPaneLayoutNode* const& node);
        void HandleFloatTab(DockPaneTabLayoutNode* tabNode, const Point& mouseScreenPos);
        
        DockPaneLayoutNode* GetPane(std::string_view paneId);
        DockPaneTabLayoutNode* GetPaneTab(std::string_view paneId, std::string_view tabId);
        void InitPaneIndicators();
        void HidePaneDockIndicators();
        void ShowPaneDockIndicators(LayoutNode* node);
        bool IsMouseInsideWindow() const;
        bool IsMouseInsideDockIndicator(DockPosition* outDockPosition = nullptr) const;
        LayoutNode* GetPaneOrDockOnMousePosition() const;
        LayoutNode* GetPaneOrDockOnMousePositionInternal(LayoutNode* node, LayoutNodeType nodeType) const;

        bool IsAlreadyDocked(LayoutNode* node, size_t& nodeIndex) const;
        bool DoFloat(DockPaneLayoutNode* paneNode);
        bool DoDock(DockPaneLayoutNode* paneNode, LayoutNode* target, DockPosition dockPosition);

        void Print();
        void Print(LayoutNode* node, uint32_t level);

        Window* m_owner{ nullptr };
        std::unique_ptr<LayoutNode> m_rootNode;
        
        // C++17: Búsqueda con std::string_view garantizada con 0 asignaciones de memoria
        std::map<std::string, LayoutNode*, std::less<>> m_fields;
        std::map<std::string, DockPaneLayoutNode*, std::less<>> m_dockPaneFields;
        std::map<std::string, DockPaneTabLayoutNode*, std::less<>> m_dockPaneTabFields;
        std::map<std::string, PaneInfo, std::less<>> m_dockPaneInfoFields;
        std::vector<std::unique_ptr<LayoutNode>> m_floatingDockFields;
        
        std::unique_ptr<LayoutNode> m_tabDockField;

        EventHandlerId m_resizeEventId { 0 };
        
        struct DragDropContext
        {
            LayoutNode* lastTargetNode{ nullptr };
            bool lockPaneIndicators{ false };
            std::vector<std::unique_ptr<DockIndicator>> paneIndicators;
            std::unique_ptr<DockPanel> dockPanelTarget;
        };
        DragDropContext m_dragDropCtx;
    };
}

#endif