/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_SLIDER_HEADER
#define BT_PROPERTY_GRID_FIELD_SLIDER_HEADER

#include "Berta/Controls/Properties/TypedPropertyField.h"
#include "Berta/Controls/Slider.h"
#include "Berta/Controls/TextBox.h"

namespace Berta
{
    template<typename TNumber, typename = std::enable_if_t<std::is_arithmetic_v<TNumber>>>
    class PropertyGridFieldSlider : public TypedPropertyField<TNumber>
    {
    public:
        using GetterFn = std::function<std::optional<TNumber>()>;
        using SetterFn = std::function<void(TNumber)>;

    public:
        PropertyGridFieldSlider(std::string_view label, GetterFn getter, SetterFn setter, TNumber minVal, TNumber maxVal) : 
            TypedPropertyField<TNumber>(label, std::move(getter), std::move(setter)), m_min(minVal), m_max(maxVal)
        {
        }

        void OnCreate(Window* parent) override
        {
            m_textBox.Create(parent);
            m_textBox.SetFocusBehavior(TextFocusBehavior::SelectOnClick);
            m_textBox.SetScrollBarVisibility(ScrollBarVisibility::Hidden);
            
            m_slider.Create(parent);
            m_slider.SetMinMax(static_cast<float>(m_min), static_cast<float>(m_max));
		    
            this->Refresh();
            
            m_textBox.GetEvents().Focus.Connect([this](const ArgFocus& args)
            {
                if (args.Focused)
                {
                    this->NotifySelected();
                }
                else
                {
                    ApplyTextValue();
                }
            });
		    
            m_textBox.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
            {
                if (args.Key == KeyboardKey::Enter)
                {
                    ApplyTextValue();
                }
            });

            m_slider.GetEvents().ValueChanged.Connect([this](ArgSlider args)
            {
                ApplySliderValue(static_cast<TNumber>(args.Value));
            });
            m_slider.MakeActive(false, m_textBox);
        }

        void Draw(Graphics& graphics, const Rectangle& area, const Internal::PropertyGrid::LayoutConfig& config) override
        {
            int textWidth = area.Width * 0.3f;
            int margin = this->m_parent->ToScale(4);

            Rectangle textRect = { area.X, area.Y, (uint32_t)textWidth, area.Height };
            Rectangle sliderRect = { area.X + textWidth + margin, area.Y, area.Width - textWidth - margin, area.Height };

            m_textBox.SetArea(textRect);
            m_slider.SetArea(sliderRect);
        }
	    
        void SetFocus() override
        {
            m_textBox.Focus();    
        }
        
        [[nodiscard]] bool HasFocus() const override
        {
            return false;
        }
	    
        /*void Refresh() override
        {
            if (!this->m_getter)
            {
                return;
            }
		    
            std::optional<TNumber> currentOpt = this->m_getter();
            if (currentOpt.has_value())
            {
                TNumber val = currentOpt.value();
                std::string strVal = std::to_string(val);
                
                if (m_textBox.GetCaption() != strVal)
                {
                    m_textBox.SetCaption(strVal);
                }
                if (m_slider.GetValue() != static_cast<float>(val))
                {
                    m_slider.SetValue(static_cast<float>(val));
                }
            }
            else
            {
                if (m_textBox.GetCaption() != "---")
                {
                    m_textBox.SetCaption("---");
                }
            }
        }*/

        std::string GetValueAsString() const override
        {
            if (!this->m_getter)
            {
                return "";
            }
		    
            auto val = this->m_getter();
            return val.has_value() ? std::to_string(val.value()) : "---";
        }

    protected:
        void OnVisibilityChanged(bool visible) override
        {
            if (visible)
            {
                m_textBox.Show();
                m_slider.Show();
            }
            else
            {
                m_textBox.Hide();
                m_slider.Hide();
            }
        }
	    
        void OnEnableChanged(bool enabled) override
        {
            m_textBox.SetEnabled(enabled); 
            m_slider.SetEnabled(enabled);
        }
        
        void SetValueInternal(const TNumber& value) override
        {
            std::string strVal = std::to_string(value);
                
            if (m_textBox.GetCaption() != strVal)
            {
                m_textBox.SetCaption(strVal);
            }
            if (m_slider.GetValue() != static_cast<float>(value))
            {
                m_slider.SetValue(static_cast<float>(value));
            }
        }
        
        void SetMixedValuesInternal() override
        {
            if (m_textBox.GetCaption() != "---")
            {
                m_textBox.SetCaption("---");
            }
        }

    private:
        void ApplyTextValue()
        {
            std::string str = m_textBox.GetCaption();
            if (str == "---")
            {
                return;
            }

            try
            {
                TNumber parsedValue;
                std::istringstream iss(str);
                iss >> parsedValue;

                parsedValue = std::clamp(parsedValue, m_min, m_max);

                if (!this->m_getter() || this->m_getter().value() != parsedValue)
                {
                    this->m_setter(parsedValue);
                    this->NotifyValueChanged();
                }
            } 
            catch (...)
            {
            }
            this->Refresh();
        }

        void ApplySliderValue(TNumber newValue)
        {
            if (!this->m_getter() || this->m_getter().value() != newValue)
            {
                this->m_setter(newValue);
                m_textBox.SetCaption(std::to_string(newValue)); 
                this->NotifyValueChanged();
            }
        }

        TNumber m_min, m_max;
        TextBox m_textBox;
        Slider m_slider;
    };

    using PropertyGridFieldSliderInt = PropertyGridFieldSlider<int>;
    using PropertyGridFieldSliderFloat = PropertyGridFieldSlider<float>;
}

#endif
