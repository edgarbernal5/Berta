/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_STRING_INT_HEADER
#define BT_PROPERTY_GRID_FIELD_STRING_INT_HEADER

#include "Berta/Controls/Properties/PropertyGridFieldString.h"

#include <string>
#include <vector>

namespace Berta
{
	class PropertyGridFieldStringInt : public PropertyGridFieldString
	{
	public:
		PropertyGridFieldStringInt(const std::string& label, const std::string& value) :
			Berta::PropertyGridFieldString(label, value)
		{
		}

		virtual void SetValue(int value);
		virtual int ToInt() const;

		void SetMinMax(int min, int max);

	protected:
		void Create(Berta::Window* parent) override;
		bool ValidateUserInput(int& value);

	private:
		bool m_useMinMax{ false };
		int m_min, m_max;
	};
}

#endif
