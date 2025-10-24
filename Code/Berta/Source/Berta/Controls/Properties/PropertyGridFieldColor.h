/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_FIELD_COLOR_HEADER
#define BT_PROPERTY_GRID_FIELD_COLOR_HEADER

#include "Berta/Controls/PropertyGrid.h"
#include "Berta/Controls/Label.h"

#include <string>
#include <vector>

namespace Berta
{
	class PropertyGridFieldColor : public PropertyGrid::PropertyGridFieldBase
	{
	public:
		PropertyGridFieldColor(const std::string& label, const std::string& value) :
			PropertyGridFieldBase(label, value)
		{
		}

		virtual void Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor) override;
		
		virtual void SetEnabled(bool enabled) override;
		virtual void SetValue(const std::string& value) override;
		virtual void SetValue(const Color& value);
		virtual Color ToColor() const;

		void SetButtonClick(std::function<void(PropertyGridFieldColor*)> callback);

	protected:
		virtual void Create(Window* parent) override;

		Color m_color{ 0, 0, 0, 255 };
		Label m_colorRegion;
		std::function<void(PropertyGridFieldColor*)> m_clickCallback;

	private:
	};
}

#endif
