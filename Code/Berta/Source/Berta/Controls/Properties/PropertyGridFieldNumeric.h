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
#include <optional>

namespace Berta
{
	template <typename T>
    class PropertyGridFieldNumeric : public Internal::PropertyGrid::PropertyGridFieldBase
    {
        static_assert(std::is_arithmetic_v<T>, "Type T must be numeric.");

    public:
        using GetterFn = std::function<std::optional<T>()>;
        using SetterFn = std::function<void(T)>;

        PropertyGridFieldNumeric(std::string_view label, GetterFn getter, SetterFn setter)
            : PropertyGridFieldBase(label), m_getter(std::move(getter)), m_setter(std::move(setter))
        {
        }

        void OnCreate(Window* parent) override
        {
            m_textBox.Create(parent);
            m_textBox.SetFocusBehavior(TextFocusBehavior::SelectOnClick);
            
            m_textBox.SetCharFilter([](wchar_t c)
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

            m_textBox.GetEvents().Focus.Connect([this](const ArgFocus& args)
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

            m_textBox.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
            {
                if (args.Key == KeyboardKey::Enter)
                {
                    ApplyValue();
                }
            });
        }

        void Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config) override
        {
            m_textBox.SetArea(area);
        }

        void Refresh() override
        {
            if (m_getter) 
            {
                std::optional<T> currentOpt = m_getter();
                if (currentOpt.has_value()) 
                {
                    std::string str = ToString(currentOpt.value());
                    if (m_textBox.GetCaption() != str)
                    {
                        m_textBox.SetCaption(str);
                    }
                }
                else 
                {
                    if (m_textBox.GetCaption() != "---")
                    {
                        m_textBox.SetCaption("---");
                    }
                }
            }
        }

        void SetFocus() override 
        { 
            m_textBox.Focus(); 
        }
	    
	    bool HasFocus() const override
        {
            return false;
        }

	    std::string GetValueAsString() const override
        {
            if (m_getter)
            {
                std::optional<T> currentState = m_getter();
                if (currentState.has_value())
                {
                    return ToString(currentState.value());
                }
                return "---"; 
            }
            
            return "";
        }
	    
	protected:
	    
	    void OnVisibilityChanged(bool visible) override
	    {
	        if (visible)
	        {
	            m_textBox.Show();
	        }
	        else
	        {
	            m_textBox.Hide();
	        }
	    }
	    void OnEnableChanged(bool enabled) override
	    {
	        m_textBox.SetEnabled(enabled);
	    }
	    
    private:
	    std::string FormatValue(T value) const
	    {
	        if constexpr (std::is_floating_point_v<T>) 
	        {
	            std::ostringstream out;
	            out << std::fixed << std::setprecision(3) << value;
            
	            std::string str = out.str();
	            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
	            if (str.back() == '.')
	            {
	                str.pop_back();
	            }
            
	            return str;
	        }
	        else 
	        {
	            return std::to_string(value);
	        }
	    }
        void ApplyValue()
        {
            if (!m_setter || !m_getter)
            {
                return;
            }

            try
            {
                T parsedValue = FromString(m_textBox.GetCaption());
                std::optional<T> currentState = m_getter();
                
                if (!currentState.has_value() || parsedValue != currentState.value())
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

        GetterFn m_getter;
        SetterFn m_setter;
        TextBox m_textBox;
    };

    using PropertyGridFieldInt    = PropertyGridFieldNumeric<int>;
    using PropertyGridFieldFloat  = PropertyGridFieldNumeric<float>;
    using PropertyGridFieldDouble = PropertyGridFieldNumeric<double>;
}

#endif
