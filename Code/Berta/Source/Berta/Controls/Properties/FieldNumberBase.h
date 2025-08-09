/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FIELD_NUMBER_BASE_HEADER
#define BT_FIELD_NUMBER_BASE_HEADER

#include "Berta/GUI/EnumTypes.h"

#include <string>
#include <type_traits>

namespace Berta
{
	template <typename T>
	struct IsNumeric : std::is_arithmetic<T> {};

	template<typename TNumber, typename = std::enable_if_t<IsNumeric<TNumber>::value>>
	class FieldNumberBase
	{
	public:
		virtual void SetValue(TNumber value) = 0;

		virtual TNumber ToNumber() const = 0;

		void NoMinMax()
		{
			m_useMinMax = false;
			m_min = {};
			m_max = {};
		}

		virtual void SetMinMax(TNumber min, TNumber max)
		{
			m_useMinMax = true;
			m_min = min;
			m_max = max;
		}

	protected:
		virtual bool ValidateUserInput(TNumber& value) { return true; }

		bool m_useMinMax{ false };
		TNumber m_min{ 0 };
		TNumber m_max{ 0 };

	private:
	};
}

#endif
