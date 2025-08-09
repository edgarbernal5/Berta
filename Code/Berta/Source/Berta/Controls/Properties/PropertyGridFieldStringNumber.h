/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_STRING_NUMBER_HEADER
#define BT_PROPERTY_GRID_FIELD_STRING_NUMBER_HEADER

#include "Berta/Controls/Properties/PropertyGridFieldString.h"

#include "Berta/GUI/EnumTypes.h"

#include <string>
#include <vector>
#include <type_traits>

namespace Berta
{
	template<typename TNumber>
	struct IsIntOrUint : std::false_type {};

	template<>
	struct IsIntOrUint<int> : std::true_type {};

	template<>
	struct IsIntOrUint<unsigned int> : std::true_type {};

	template<typename TNumber, typename = std::enable_if_t<IsIntOrUint<TNumber>::value>>
	class PropertyGridFieldStringNumber : public PropertyGridFieldString
	{
	public:
		PropertyGridFieldStringNumber(const std::string& label, const std::string& value) :
			PropertyGridFieldString(label, value)
		{
		}

		virtual void SetValue(TNumber value)
		{
			PropertyGridFieldString::SetValue(std::to_string(value));
		}

		virtual TNumber ToNumber() const
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

		void NoMinMax()
		{
			m_useMinMax = false;
			m_min = {};
			m_max = {};
		}

		void SetMinMax(TNumber min, TNumber max)
		{
			m_useMinMax = true;
			m_min = min;
			m_max = max;
		}

	protected:
		void Create(Berta::Window* parent) override
		{
			PropertyGridFieldString::Create(parent);

			m_inputText.GetEvents().KeyPressed.Reset();
			m_inputText.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
				{
					if (args.Key == KeyboardKey::Enter && m_inputText.GetCaption() != PropertyGridField::GetValue())
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
		bool ValidateUserInput(TNumber& value)
		{
			TNumber result{};
			try
			{
				std::istringstream iss(m_inputText.GetCaption());
				iss >> result;
				if (m_useMinMax)
				{
					result = std::clamp(result, m_min, m_max);
				}
			}
			catch (...)
			{
				return false;
			}

			value = result;
			return true;
		}

	private:
		bool m_useMinMax{ false };
		TNumber m_min{ 0 };
		TNumber m_max{ 0 };
	};

	using PropertyGridFieldStringInt = PropertyGridFieldStringNumber<int>;
	using PropertyGridFieldStringUInt = PropertyGridFieldStringNumber<uint32_t>;
}

#endif
