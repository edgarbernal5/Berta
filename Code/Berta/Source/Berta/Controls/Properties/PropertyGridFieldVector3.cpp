/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldVector3.h"

namespace Berta
{
    PropertyGridFieldVector3::PropertyGridFieldVector3(std::string_view label, GetterFn getter, SetterFn setter)
        : TypedPropertyField(label, std::move(getter), std::move(setter))
    {
    }

    void PropertyGridFieldVector3::Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config)
    {
        graphics.DrawString({ area.X, area.Y + 4 }, m_summaryText, config.Foreground);
    }

    void PropertyGridFieldVector3::SetFocus()
    {
    }

    bool PropertyGridFieldVector3::HasFocus() const
    {
        return false;
    }

    std::wstring PropertyGridFieldVector3::GetValueAsString() const
    {
        return m_summaryText;
    }

    void PropertyGridFieldVector3::OnCreate(Window* parent)
    {
        this->Refresh();
    }

    void PropertyGridFieldVector3::OnVisibilityChanged(bool visible)
    {
        
    }

    void PropertyGridFieldVector3::OnEnableChanged(bool enabled)
    {
        
    }

    void PropertyGridFieldVector3::SetValueInternal(const OptionalVector3& value)
    {
        m_summaryText = FormatSummary(value);
    }

    void PropertyGridFieldVector3::SetMixedValuesInternal()
    {
        m_summaryText = L"(---, ---, ---)";
    }

    std::wstring PropertyGridFieldVector3::FormatSummary(const OptionalVector3& v) const
    {
        std::wstring summary = L"(";
        summary += v.x.has_value() ? FormatFloat(v.x.value()) : L"---";
        summary += L", ";
        summary += v.y.has_value() ? FormatFloat(v.y.value()) : L"---";
        summary += L", ";
        summary += v.z.has_value() ? FormatFloat(v.z.value()) : L"---";
        summary += L")";
        return summary;
    }

    std::wstring PropertyGridFieldVector3::FormatFloat(float val) const
    {
        std::wstring str = std::to_wstring(val);
        str.erase(str.find_last_not_of(L'0') + 1, std::wstring::npos);
        if (!str.empty() && str.back() == L'.')
        {
            str.push_back(L'0'); // Asegura que se lea "10.0" y no "10."
        }
        return str;
    }
}
