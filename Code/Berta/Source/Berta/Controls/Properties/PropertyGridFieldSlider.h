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
    class PropertyGridFieldSlider : public PropertyGridFieldNumeric<TNumber>
    {
    public:
        using GetterFn = std::function<std::optional<TNumber>()>;
        using SetterFn = std::function<void(TNumber)>;

    public:
        PropertyGridFieldSlider(std::string_view label, GetterFn getter, SetterFn setter, TNumber minVal, TNumber maxVal, TNumber step = static_cast<TNumber>(0)) : 
            PropertyGridFieldNumeric<TNumber>(label, std::move(getter), std::move(setter)), m_min(minVal), m_max(maxVal), m_step(step)
        {
        }

        void OnCreate(Window* parent) override
        {
            m_textBox.Create(parent);
            m_textBox.SetFocusBehavior(TextFocusBehavior::SelectOnClick);
            m_textBox.SetScrollBarVisibility(ScrollBarVisibility::Hidden);
            
            m_slider.Create(parent);
            
            if constexpr (std::is_floating_point_v<TNumber>)
            {
                m_slider.SetMinMax(0.0f, FLOAT_RESOLUTION);
            }
            else
            {
                m_slider.SetMinMax(static_cast<float>(m_min), static_cast<float>(m_max));
            }
            
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
                ApplySliderValue(static_cast<float>(args.Value));
            });
            m_slider.MakeActive(false, m_textBox);
        }

        void Draw(Graphics& graphics, const Rectangle& area, const Internal::PropertyGrid::LayoutConfig& config) override
        {
            int textWidth = area.Width * 0.3f;
            int margin = this->m_parent->ToScale(4);

            Rectangle textRect = { area.X, area.Y, static_cast<uint32_t>(textWidth), area.Height };
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

        std::wstring GetValueAsString() const override
        {
            if (!this->m_getter)
            {
                return L"";
            }
		    
            auto val = this->m_getter();
            return val.has_value() ? std::to_wstring(val.value()) : L"---";
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
            std::string strVal = this->FormatValue(value);
                
            if (m_textBox.GetCaption() != strVal)
            {
                m_textBox.SetCaption(strVal);
            }

            float targetSliderValue;
            if constexpr (std::is_floating_point_v<TNumber>) 
            {
                float t = (m_max > m_min) ? static_cast<float>(value - m_min) / static_cast<float>(m_max - m_min) : 0.0f;
                targetSliderValue = t * FLOAT_RESOLUTION;
            } 
            else 
            {
                targetSliderValue = static_cast<float>(value);
            }

            if (m_slider.GetValue() != targetSliderValue)
            {
                m_slider.SetValue(targetSliderValue);
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
        TNumber SnapToStep(TNumber value) const
        {
            if (m_step <= static_cast<TNumber>(0))
            {
                return std::clamp(value, m_min, m_max);
            }

            TNumber snappedValue;
            if constexpr (std::is_floating_point_v<TNumber>)
            {
                snappedValue = m_min + static_cast<TNumber>(std::round(static_cast<double>(value - m_min) / static_cast<double>(m_step))) * m_step;
            }
            else
            {
                snappedValue = m_min + ((value - m_min + (m_step / 2)) / m_step) * m_step;
            }
            
            return std::clamp(snappedValue, m_min, m_max);
        }
        
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

                parsedValue = SnapToStep(parsedValue);

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

        void ApplySliderValue(float rawSliderValue)
        {
            TNumber newValue;
            
            if constexpr (std::is_floating_point_v<TNumber>)
            {
                float t = rawSliderValue / FLOAT_RESOLUTION;
                newValue = static_cast<TNumber>(m_min + (m_max - m_min) * t);
            }
            else
            {
                newValue = static_cast<TNumber>(rawSliderValue);
            }

            newValue = SnapToStep(newValue);

            if (!this->m_getter() || this->m_getter().value() != newValue)
            {
                this->m_setter(newValue);
                m_textBox.SetCaption(this->FormatValue(newValue)); 
                this->NotifyValueChanged();
            }
        }
        
        /*void ApplySliderValue(TNumber newValue)
        {
            if (!this->m_getter() || this->m_getter().value() != newValue)
            {
                this->m_setter(newValue);
                m_textBox.SetCaption(std::to_string(newValue)); 
                this->NotifyValueChanged();
            }
        }*/
        
        static constexpr float FLOAT_RESOLUTION = 10000.0f;

        TNumber m_min, m_max;
        TNumber m_step;
        TextBox m_textBox;
        Slider m_slider;
    };

    using PropertyGridFieldSliderInt = PropertyGridFieldSlider<int>;
    using PropertyGridFieldSliderUInt8 = PropertyGridFieldSlider<uint8_t>;
    using PropertyGridFieldSliderUInt = PropertyGridFieldSlider<uint32_t>;
    using PropertyGridFieldSliderFloat = PropertyGridFieldSlider<float>;
}

#endif
