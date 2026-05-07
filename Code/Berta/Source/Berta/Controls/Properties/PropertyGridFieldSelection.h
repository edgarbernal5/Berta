/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_SELECTION_HEADER
#define BT_PROPERTY_GRID_FIELD_SELECTION_HEADER

#include "Berta/Controls/Properties/TypedPropertyField.h"
#include "Berta/Controls/ComboBox.h"

#include <string>
#include <vector>
#include <functional>
#include <any>

namespace Berta
{
    template <typename T>
    class PropertyGridFieldSelection : public TypedPropertyField<T>
    {
    public:
        using GetterFn = std::function<std::optional<T>()>;
        using SetterFn = std::function<void(T)>;
        
        using OptionList = std::vector<std::pair<std::wstring, T>>;

        PropertyGridFieldSelection(std::string_view label, GetterFn getter, SetterFn setter, OptionList options)
            : TypedPropertyField<T>(label, std::move(getter), std::move(setter)), m_options(std::move(options))
        {
        }

        void OnCreate(Window* parent) override
        {
            m_comboBox.Create(parent);

            for (const auto& option : m_options)
            {
                m_comboBox.PushBack(option.first, std::make_any<T>(option.second));
            }

            this->Refresh();

            m_comboBox.GetEvents().Selected.Connect([this](const ArgComboBox& args) 
            {
                if (args.SelectedIndex.has_value())
                {
                    std::any payload = m_comboBox.GetSelectedData();
                    
                    if (payload.has_value() && payload.type() == typeid(T))
                    {
                        T selectedValue = std::any_cast<T>(payload);
                        
                        if (this->m_getter && selectedValue != this->m_getter())
                        {
                            this->m_setter(selectedValue);
                            this->NotifyValueChanged();
                        }
                    }
                }
            });
            
            m_comboBox.GetEvents().Focus.Connect([this](const ArgFocus& args)
            {
                if (args.Focused)
                {
                    this->NotifySelected();
                }
            });
        }

        void Draw(Graphics& graphics, const Rectangle& area, const Internal::PropertyGrid::LayoutConfig& config) override
        {
            m_comboBox.SetArea(area);
        }

        /*void Refresh() override
        {
            if (!this->m_getter)
            {
                return;
            }
            
            std::optional<T> currentOpt = m_getter();
            if (!currentOpt.has_value())
            {
                m_comboBox.SetSelectedIndex(std::nullopt);
                return;
            }
            std::optional<size_t> foundIndex = std::nullopt;

            for (size_t i = 0; i < m_options.size(); ++i)
            {
                if (m_options[i].second == currentOpt)
                {
                    foundIndex = i;
                    break;
                }
            }

            if (m_comboBox.GetSelectedIndex() != foundIndex)
            {
                m_comboBox.SetSelectedIndex(foundIndex);
            }
        }*/

        std::wstring GetValueAsString() const override
        {
            if (!this->m_getter)
            {
                return L"";
            }

            std::optional<T> currentOpt = this->m_getter();
            if (!currentOpt.has_value())
            {
                return L"";
            }
            
            for (const auto& [text, value] : m_options)
            {
                if (value == currentOpt)
                {
                    return text;
                }
            }
            return L"";
        }

        void SetFocus() override 
        { 
            m_comboBox.Focus(); 
        }
	    
        [[nodiscard]] bool HasFocus() const override
        {
            return false;
        }

    protected:
        void OnVisibilityChanged(bool visible) override
        {
            if (visible)
            {
                m_comboBox.Show();
            }
            else
            {
                m_comboBox.Hide();
            }
        }

        void OnEnableChanged(bool enabled) override
        {
            m_comboBox.SetEnabled(enabled);
        }

        void SetValueInternal(const T& value) override
        {
            std::optional<size_t> foundIndex = std::nullopt;

            for (size_t i = 0; i < m_options.size(); ++i)
            {
                if (m_options[i].second == value)
                {
                    foundIndex = i;
                    break;
                }
            }

            if (m_comboBox.GetSelectedIndex() != foundIndex)
            {
                m_comboBox.SetSelectedIndex(foundIndex);
            }
        }
        
        void SetMixedValuesInternal() override
        {
            m_comboBox.SetSelectedIndex(std::nullopt);
        }
    private:
        OptionList m_options;
        ComboBox m_comboBox;
    };
}

#endif
