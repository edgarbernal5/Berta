/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_BASE_HEADER
#define BT_PROPERTY_GRID_FIELD_BASE_HEADER

#include "Berta/Controls/Properties/Appearance.h"
#include "Berta/GUI/Window.h"

namespace Berta::Internal::PropertyGrid
{
    class PropertyGridFieldBase
    {
    public:
    	static constexpr std::string_view MIXED_VALUES_TEXT = "---";
    	using LayoutConfig = Berta::Internal::PropertyGrid::LayoutConfig;
    	
    public:
        PropertyGridFieldBase() = default;
        PropertyGridFieldBase(std::string_view label) :
            m_label(label)
        {
        }
        virtual ~PropertyGridFieldBase() = default;
    	
        PropertyGridFieldBase(const PropertyGridFieldBase&) = delete;
        PropertyGridFieldBase& operator=(const PropertyGridFieldBase&) = delete;
			
        PropertyGridFieldBase(PropertyGridFieldBase&&) = default;
        PropertyGridFieldBase& operator=(PropertyGridFieldBase&&) = default;

        void Init(Window* parent);

        virtual std::string_view GetLabel() const;
        virtual void SetLabel(std::string_view newLabel);

        [[nodiscard]] virtual std::wstring GetValueAsString() const = 0;
    	
        bool IsEnabled() const;
        void SetEnabled(bool enabled);

        bool IsShowingLabel() const { return m_showLabel; }
        void SetShowLabel(bool show) { m_showLabel = show; }

        virtual uint32_t GetHeight() const
        {
            return m_parent->ToScale(m_height);
        }
    	
        virtual void OnMouseClick(const Point& localPosition, uint32_t labelWidth) 
        {
        }
			
        virtual void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) = 0;

        virtual void SetFocus() = 0;
    	[[nodiscard]] virtual bool HasFocus() const = 0;
    	
        virtual void Refresh() = 0;
    	
        bool IsVisible() const;
        void SetVisibility(bool visible);
    	
        std::function<void()> OnValueChanged;
        std::function<void()> OnSelected;
			
    protected:
        virtual void OnCreate(Window* parent) = 0;
        virtual void OnVisibilityChanged(bool visible) {}
        virtual void OnEnableChanged(bool enabled) {}
    	
        void NotifyValueChanged();
        void NotifySelected();
    	
        Window* m_parent{ nullptr };

        std::string	m_label;

        uint32_t m_height{ 24 };
        bool m_enabled { true };
        bool m_isVisible { true };
        bool m_showLabel { true };
    };
}

#endif