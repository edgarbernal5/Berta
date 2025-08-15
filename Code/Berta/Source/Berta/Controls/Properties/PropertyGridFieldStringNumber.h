/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_STRING_NUMBER_HEADER
#define BT_PROPERTY_GRID_FIELD_STRING_NUMBER_HEADER

#include "Berta/Controls/Properties/FieldNumberBase.h"

#include "Berta/GUI/EnumTypes.h"

#include <string>
#include <vector>
#include <type_traits>

namespace Berta
{
	/*template<typename TNumber>
	struct IsIntOrUint : std::false_type {};

	template<>
	struct IsIntOrUint<int> : std::true_type {};

	template<>
	struct IsIntOrUint<unsigned int> : std::true_type {};*/

	template<typename TNumber, typename = std::enable_if_t<IsNumeric<TNumber>::value>>
	class PropertyGridFieldStringNumber : public PropertyGridFieldString, public FieldNumberBase<TNumber, std::enable_if_t<IsNumeric<TNumber>::value>>
	{
	public:
		PropertyGridFieldStringNumber(const std::string& label, const std::string& value) : 
			PropertyGridFieldString(label, value),
			FieldNumberBase<TNumber, std::enable_if_t<IsNumeric<TNumber>::value>>()
		{
		}

		virtual void SetValue(TNumber value) override
		{
			PropertyGridFieldString::SetValue(std::to_string(value));
		}

		virtual TNumber ToNumber() const override
		{
			TNumber result{};
			try
			{
				std::istringstream iss(PropertyGridFieldString::GetValue());
				iss >> result;
			}
			catch (...)
			{
			}
			return result;
		}

	protected:
		virtual void Create(Window* parent) override
		{
			PropertyGridFieldString::Create(parent);

			m_inputText.GetEvents().KeyPressed.Reset();
			m_inputText.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
				{
					if (args.Key == KeyboardKey::Enter && m_inputText.GetCaption() != PropertyGridFieldBase::GetValue())
					{
						TNumber result{};
						if (ValidateUserInput(result))
						{
							SetValue(result);
							EmitEvent();
						}
						else
						{
							m_inputText.SetCaption(m_value);
						}
					}
				});

			m_inputText.SetCharFilter([this](wchar_t chr)
				{
					auto isDigit = std::isdigit(chr);
					auto isMinus = false;
					if constexpr (std::is_signed_v<TNumber>)
					{
						isMinus = chr == '-' && m_inputText.GetCaretPosition() == 0 && m_inputText.GetCaption().find('-') == std::string::npos;
					}
					return isDigit || isMinus;
				});
		}

		virtual bool ValidateUserInput(TNumber& outValue) override
		{
			try
			{
				std::istringstream iss(m_inputText.GetCaption());
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
	};

	using PropertyGridFieldStringInt = PropertyGridFieldStringNumber<int>;
	using PropertyGridFieldStringUInt = PropertyGridFieldStringNumber<uint32_t>;
}

#endif
