/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_NUMERIC_HEADER
#define BT_PROPERTY_GRID_FIELD_NUMERIC_HEADER

#include "Berta/GUI/EnumTypes.h"

#include "Berta/Controls/TextBox.h"
#include "Berta/Controls/Properties/TypedPropertyField.h"

#include <string>
#include <type_traits>
#include <cwctype>
#include <optional>

namespace Berta
{
	template <typename T>
    class PropertyGridFieldNumeric : public TypedPropertyField<T>
    {
        static_assert(std::is_arithmetic_v<T>, "Type T must be numeric.");

    public:
	    using GetterFn = typename TypedPropertyField<T>::GetterFn;
	    using SetterFn = typename TypedPropertyField<T>::SetterFn;
	    
	    PropertyGridFieldNumeric(std::string_view label, GetterFn getter, SetterFn setter)
            : TypedPropertyField<T>(label, std::move(getter), std::move(setter))
	    {
	    }
	    ~PropertyGridFieldNumeric() override = default;

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

            this->Refresh(); 

            m_textBox.GetEvents().Focus.Connect([this](const ArgFocus& args)
            {
                if (args.Focused)
                {
                    this->NotifySelected();
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

        void Draw(Graphics& graphics, const Rectangle& area, const Internal::PropertyGrid::LayoutConfig& config) override
        {
            m_textBox.SetArea(area);
        }

        void SetFocus() override 
        { 
            m_textBox.Focus(); 
        }
	    
	    [[nodiscard]] bool HasFocus() const override
        {
            return false;
        }

	    std::wstring GetValueAsString() const override
        {
            if (this->m_getter)
            {
                std::optional<T> currentState = this->m_getter();
                if (currentState.has_value())
                {
                    return ToString(currentState.value());
                }
                return L"---"; 
            }
            
            return L"";
        }
	    
	protected:
	    void SetValueInternal(const T& value) override
	    {
	        std::wstring strValue = ToString(value);

	        if (m_textBox.GetCaptionW() != strValue)
	        {
	            m_textBox.SetText(strValue);
	        }
	    }
	    
	    void SetMixedValuesInternal() override
	    {
	        if (m_textBox.GetCaption()!= "---")
	        {
	            m_textBox.SetCaption("---"); 
	        }
	    }
	    
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
		
		void OnReadOnlyChanged(bool readOnly) override
	    {
	    	m_textBox.SetEditable(!readOnly);
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
            if (!this->m_setter || !this->m_getter)
            {
                return;
            }

            try
            {
                T parsedValue = FromString(m_textBox.GetCaptionW());
                std::optional<T> currentState = this->m_getter();
                
                if (!currentState.has_value() || parsedValue != currentState.value())
                {
                    this->m_setter(parsedValue);
                    this->NotifyValueChanged(); 
                }
                this->Refresh();
            } 
            catch (const std::exception&) 
            {
                this->Refresh(); 
            }
        }

        std::wstring ToString(T val) const 
        {
            if constexpr (std::is_floating_point_v<T>)
            {
                std::wstring str = std::to_wstring(val);
                str.erase(str.find_last_not_of('0') + 1, std::string::npos);
                if (str.back() == '.')
                {
                    str.push_back('0');
                }
                return str;
            }
            else
            {
                return std::to_wstring(val);
            }
        }

        T FromString(const std::wstring& str) const 
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

        TextBox m_textBox;
    };

    using PropertyGridFieldInt    = PropertyGridFieldNumeric<int>;
    using PropertyGridFieldFloat  = PropertyGridFieldNumeric<float>;
    using PropertyGridFieldDouble = PropertyGridFieldNumeric<double>;
}

#endif
