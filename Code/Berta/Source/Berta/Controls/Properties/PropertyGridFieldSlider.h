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
#include "Berta/Controls/InputText.h"

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

			auto txtValueWidth = m_parent->ToScale(60);
			auto margin = m_parent->ToScale(4);
			valueRect.Width -= txtValueWidth + margin;
			m_slider.SetArea(valueRect);
			m_slider.Show();

			valueRect.X += valueRect.Width + margin;
			valueRect.Width = txtValueWidth;

			m_valueInputText.SetArea(valueRect);
			m_valueInputText.Show();
		}

		virtual void SetEnabled(bool enabled) override
		{
			PropertyGridField::SetEnabled(enabled);
			m_slider.SetEnabled(enabled);
			m_valueInputText.SetEnabled(enabled);
		}

		void SetValue(const std::string& value) override
		{
			try
			{
				TNumber numberValue{};
				std::istringstream iss(value);
				iss >> numberValue;

				PropertyGridField::SetValue(value);
				m_slider.SetValue(static_cast<int>(numberValue));
				m_valueInputText.SetText(value);
			}
			catch (...)
			{
			}
		}

		void SetValue(TNumber value) override
		{
			auto newString = std::to_string(value);
			PropertyGridField::SetValue(newString);
			m_slider.SetValue(static_cast<int>(value));
			m_valueInputText.SetText(newString);
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
			m_valueInputText.SetText(std::to_wstring(m_slider.GetValue()));
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

			m_valueInputText.Create(parent);
			m_valueInputText.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
				{
					if (args.Key == KeyboardKey::Enter && m_valueInputText.GetCaption() != PropertyGridField::GetValue())
					{
						TNumber result{};
						if (this->ValidateUserInput(result))
						{
							SetValue(result);
							EmitEvent();
						}
						else
						{
							m_valueInputText.SetCaption(m_value);
						}
					}
				});

			m_valueInputText.GetEvents().Focus.Connect([this](const ArgFocus& args)
				{
					if (args.Focused)
						return;

					TNumber result{};
					if (this->ValidateUserInput(result))
					{
						SetValue(result);
						EmitEvent();
					}
					else
					{
						m_valueInputText.SetCaption(m_value);
					}
				});

			m_valueInputText.SetCharFilter([this](wchar_t chr)
				{
					auto isDigit = std::isdigit(chr);
					auto isMinus = false;
					if constexpr (std::is_signed_v<TNumber>)
					{
						isMinus = chr == '-' && m_valueInputText.GetCaretPosition() == 0 && m_valueInputText.GetCaption().find('-') == std::string::npos;
					}
					return isDigit || isMinus;
				});

			SetValue(m_value);
		}

	protected:
		virtual bool ValidateUserInput(TNumber& outValue) override
		{
			try
			{
				std::istringstream iss(m_valueInputText.GetCaption());
				iss >> outValue;
				if (this->m_useMinMax)
				{
					outValue = std::clamp(outValue, this->m_min, this->m_max);
				}
			}
			catch (...)
			{
				return false;
			}
			return true;
		}

	private:
		Slider m_slider;
		InputText m_valueInputText;
	};

	using PropertyGridFieldSliderInt = PropertyGridFieldSlider<int>;
	using PropertyGridFieldSliderFloat = PropertyGridFieldSlider<float>;
}

#endif
