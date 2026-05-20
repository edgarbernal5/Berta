/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldBase.h"

namespace Berta::Internal::PropertyGrid
{
    void PropertyGridFieldBase::Init(Window* parent)
    {
        m_parent = parent;
        OnCreate(parent);
        SetEnabled(IsEnabled());
    }

    std::string_view PropertyGridFieldBase::GetLabel() const
    {
        return m_label;
    }

    void PropertyGridFieldBase::SetLabel(std::string_view newLabel)
    {
        if (m_label == newLabel)
        {
            return;
        }

        m_label = newLabel;
    }

    bool PropertyGridFieldBase::IsEnabled() const
    {
        return m_enabled;
    }

    void PropertyGridFieldBase::SetEnabled(bool enabled)
    {
        if (m_enabled == enabled)
        {
            return;
        }
        m_enabled = enabled;
        OnEnableChanged(enabled);
    }

    bool PropertyGridFieldBase::IsVisible() const
    {
        return m_isVisible;
    }

    void PropertyGridFieldBase::SetVisibility(bool visible)
    {
        if (m_isVisible == visible)
        {
            return;
        }
			
        m_isVisible = visible;
        OnVisibilityChanged(visible);
    }

    bool PropertyGridFieldBase::IsReadOnly() const
    {
        return m_readOnly;
    }

    void PropertyGridFieldBase::SetReadOnly(bool readOnly)
    {
        if (m_readOnly == readOnly)
        {
            return;
        }
			
        m_readOnly = readOnly;
        OnReadOnlyChanged(readOnly);
    }

    void PropertyGridFieldBase::NotifyValueChanged()
    {
        if (OnValueChanged)
        { 
            OnValueChanged(); 
        }
    }

    void PropertyGridFieldBase::NotifySelected()
    {
        if (OnSelected)
        {
            OnSelected();
        }
    }
}