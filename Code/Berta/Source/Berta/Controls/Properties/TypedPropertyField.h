/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TYPED_PROPERTY_GRID_FIELD_HEADER
#define BT_TYPED_PROPERTY_GRID_FIELD_HEADER

#include "Berta/Controls/Properties/PropertyGridFieldBase.h"

#include <optional>

namespace Berta
{
    template <typename T>
    class TypedPropertyField : public Internal::PropertyGrid::PropertyGridFieldBase
    {
    public:
        using GetterFn = std::function<std::optional<T>()>;
        using SetterFn = std::function<void(const T&)>;
        
        TypedPropertyField(std::string_view label, GetterFn getter, SetterFn setter)
            : PropertyGridFieldBase(label), m_getter(std::move(getter)), m_setter(std::move(setter))
        {
        }

        void Refresh() override
        {
            if (HasFocus())
            {
                return;
            }

            if (m_getter)
            {
                std::optional<T> currentOpt = m_getter();
                if (currentOpt.has_value())
                {
                    m_isMixedValue = false;
                    SetValueInternal(currentOpt.value());
                }
                else
                {
                    m_isMixedValue = true;
                    SetMixedValuesInternal();
                }
            }
        }

    protected:
        virtual void SetValueInternal(const T& value) = 0;
        virtual void SetMixedValuesInternal() = 0;
        
        GetterFn m_getter;
        SetterFn m_setter;
        bool m_isMixedValue{ false };
    };
}

#endif
