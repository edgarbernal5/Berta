/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_STRING_BUTTON_HEADER
#define BT_PROPERTY_GRID_FIELD_STRING_BUTTON_HEADER

#include "Berta/Controls/Properties/PropertyGridFieldString.h"
#include "Berta/Controls/Button.h"

#include <string>
#include <vector>

namespace Berta
{
	class PropertyGridFieldStringButton : public PropertyGridFieldString
	{
	public:
		PropertyGridFieldStringButton(const std::string& label, const std::string& value) :
			PropertyGridFieldString(label, value)
		{
		}

		virtual void Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor) override;
		
		virtual void SetEnabled(bool enabled) override;

		void SetButtonClick(std::function<void(PropertyGridFieldStringButton*)> callback);

	protected:
		virtual void Create(Window* parent) override;

		std::string m_buttonText = "...";
		Button m_button;
		std::function<void(PropertyGridFieldStringButton*)> m_clickCallback;

	private:
	};
}

#endif
