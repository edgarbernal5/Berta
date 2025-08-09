/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_NUMBER_SLIDER_HEADER
#define BT_PROPERTY_GRID_FIELD_NUMBER_SLIDER_HEADER

#include "Berta/Controls/Properties/FieldNumberBase.h"
#include "Berta/Controls/PropertyGrid.h"
#include "Berta/Controls/Slider.h"

namespace Berta
{
	template<typename TNumber, typename = std::enable_if_t<IsNumeric<TNumber>::value>>
	class PropertyGridFieldSlider : public PropertyGridField, public FieldNumberBase<TNumber, std::enable_if_t<IsNumeric<TNumber>::value>>
	{
	public:
		PropertyGridFieldSlider(const std::string& label, const std::string& value) :
			PropertyGridField(label, value),
			FieldNumberBase<TNumber, std::enable_if_t<IsNumeric<TNumber>::value>>()
		{
		}

		virtual void Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor) override
		{
			PropertyGridField::Draw(graphics, area, labelWidth, textColor);

			Rectangle valueRect = area;

			valueRect.X += static_cast<int>(labelWidth);
			valueRect.Width -= labelWidth;

			if (valueRect.Width == 0)
				return;

			valueRect.X = 0;
			valueRect.Y = 0;
			m_slider.SetArea(valueRect);
			m_slider.Show();
		}

		virtual void SetEnabled(bool enabled) override
		{
			PropertyGridField::SetEnabled(enabled);
			m_slider.SetEnabled(enabled);
		}

		void SetValue(const std::string& value) override
		{
			try
			{
				TNumber numberValue{};
				std::istringstream iss(value);
				iss >> numberValue;

				m_slider.SetValue(static_cast<int>(numberValue));
				PropertyGridField::SetValue(value);
			}
			catch (...)
			{
			}
		}

		void SetValue(TNumber value) override
		{
			PropertyGridField::SetValue(std::to_string(value));
			m_slider.SetValue(static_cast<int>(value));
		}

		TNumber ToNumber() const override
		{
			TNumber result{};
			try
			{
				std::istringstream iss(PropertyGridField::GetValue());
				iss >> result;
			}
			catch (...)
			{
			}
			return result;
		}

		void SetMinMax(TNumber min, TNumber max) override
		{
			FieldNumberBase<TNumber, std::enable_if_t<IsNumeric<TNumber>::value>>::SetMinMax(min, max);
			m_slider.SetMinMax(static_cast<int>(min), static_cast<int>(max));
		}

	protected:
		void Create(Window* parent) override
		{
			m_slider.Create(parent);

			m_slider.GetEvents().ValueChanged.Connect([this](const ArgSlider& args)
				{
					TNumber result = static_cast<TNumber>(args.Value);
					
					SetValue(result);
					EmitEvent();
				});

		}

	private:
		Slider m_slider;
	};

	using PropertyGridFieldSliderInt = PropertyGridFieldSlider<int>;
}

#endif
