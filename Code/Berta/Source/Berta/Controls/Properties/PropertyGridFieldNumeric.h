/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_NUMERIC_HEADER
#define BT_PROPERTY_GRID_FIELD_NUMERIC_HEADER

#include "Berta/GUI/EnumTypes.h"

#include <string>
#include <functional>
#include <type_traits>
#include <cwctype>

namespace Berta
{
	/*template<typename TNumber>
	struct IsIntOrUint : std::false_type {};

	template<>
	struct IsIntOrUint<int> : std::true_type {};

	template<>
	struct IsIntOrUint<unsigned int> : std::true_type {};*/

	template <typename T>
    class PropertyGridFieldNumeric : public Internal::PropertyGrid::PropertyGridFieldBase
    {
        static_assert(std::is_arithmetic_v<T>, "El tipo T debe ser numérico.");

    public:
        using Getter = std::function<T()>;
        using Setter = std::function<void(T)>;

        PropertyGridFieldNumeric(std::string_view label, Getter getter, Setter setter)
            : PropertyGridFieldBase(label), m_getter(std::move(getter)), m_setter(std::move(setter))
        {}

        void OnCreate(Window* parent) override
        {
            m_inputText.Create(parent);
            m_inputText.SetFocusBehavior(TextFocusBehavior::SelectOnClick);
            
            m_inputText.SetCharFilter([](wchar_t c)
            {
                if constexpr (std::is_integral_v<T>)
                {
                    return std::iswdigit(c) || c == L'-';
                }
                else
                {
                    return std::iswdigit(c) || c == L'-' || c == L'.';
                }
            });

            Refresh(); 

            m_inputText.GetEvents().Focus.Connect([this](const ArgFocus& args)
            {
                if (args.Focused)
                {
                    NotifySelected();
                }
                else
                {
                    ApplyValue();
                }
            });

            m_inputText.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
            {
                if (args.Key == KeyboardKey::Enter)
                {
                    ApplyValue();
                }
            });
        }

        void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override
        {
            m_inputText.SetArea(area);
        }

        void Refresh() override
        {
            if (m_getter) 
            {
                std::string value = ToString(m_getter());
                if (m_inputText.GetCaption() != value)
                {
                    m_inputText.SetCaption(value);
                }
            }
        }

        void SetFocus() override 
        { 
            m_inputText.Focus(); 
        }

	    std::string GetValueAsString() const override
        {
            return ToString(m_getter());
        }
	protected:
	    
	    void OnVisibilityChanged(bool visible) override
	    {
	        if (visible)
	        {
	            m_inputText.Show();
	        }
	        else
	        {
	            m_inputText.Hide();
	        }
	    }
	    void OnEnableChanged(bool enabled) override
	    {
	        m_inputText.SetEnabled(enabled);
	    }
	    
    private:
	    
        void ApplyValue()
        {
            if (!m_setter || !m_getter)
            {
                return;
            }

            try
            {
                T parsedValue = FromString(m_inputText.GetCaption());
                
                if (parsedValue != m_getter())
                {
                    m_setter(parsedValue);
                    NotifyValueChanged(); 
                }
                Refresh();
            } 
            catch (const std::exception&) 
            {
                Refresh(); 
            }
        }

        std::string ToString(T val) const 
        {
            if constexpr (std::is_floating_point_v<T>)
            {
                std::string str = std::to_string(val);
                str.erase(str.find_last_not_of('0') + 1, std::string::npos);
                if (str.back() == '.')
                {
                    str.push_back('0');
                }
                return str;
            }
            else
            {
                return std::to_string(val);
            }
        }

        T FromString(const std::string& str) const 
        {
            if constexpr (std::is_integral_v<T>)
            {
                return static_cast<T>(std::stoi(str));
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                return std::stof(str);
            }
            else
            {
                return std::stod(str);
            }
        }

        Getter m_getter;
        Setter m_setter;
        InputText m_inputText;
    };

    using PropertyGridFieldInt    = PropertyGridFieldNumeric<int>;
    using PropertyGridFieldFloat  = PropertyGridFieldNumeric<float>;
    using PropertyGridFieldDouble = PropertyGridFieldNumeric<double>;
}

#endif
