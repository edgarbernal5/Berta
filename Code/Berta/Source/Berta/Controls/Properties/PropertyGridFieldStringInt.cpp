/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldStringInt.h"

#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	void PropertyGridFieldStringInt::SetValue(int value)
	{
		PropertyGridFieldString::SetValue(std::to_string(value));
	}

	int PropertyGridFieldStringInt::ToInt() const
	{
		int result = -1;
		try
		{
			result = std::stoi(PropertyGridField::GetValue());
		}
		catch (...)
		{
		}
		return result;
	}

	void PropertyGridFieldStringInt::SetMinMax(int min, int max)
	{
		m_useMinMax = true;
		m_min = min;
		m_max = max;
	}

	void PropertyGridFieldStringInt::Create(Berta::Window* parent)
	{
		PropertyGridFieldString::Create(parent);
		
		m_inputText.GetEvents().KeyPressed.Reset();
		m_inputText.GetEvents().KeyPressed.Connect([this](const ArgKeyboard& args)
			{
				if (args.Key == KeyboardKey::Enter && m_inputText.GetCaption() != PropertyGridField::GetValue())
				{
					int result = -1;
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
				auto isMinus = chr == '-' && m_inputText.GetCaretPosition() == 0 && m_inputText.GetCaption().find('-') == std::string::npos;
				return isDigit || isMinus;
			});
	}

	bool PropertyGridFieldStringInt::ValidateUserInput(int& value)
	{
		int result = -1;
		try
		{
			result = std::stoi(m_inputText.GetCaption());
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
}
