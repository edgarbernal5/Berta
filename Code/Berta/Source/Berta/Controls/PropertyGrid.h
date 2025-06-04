/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_HEADER
#define BT_PROPERTY_GRID_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/ScrollBar.h"
#include "Berta/Paint/Image.h"

#include <string>
#include <vector>

namespace Berta
{
	struct PropertyGridAppearance : public ControlAppearance
	{
	};

	struct ArgListBox
	{
		size_t SelectedIndex{ 0 };
	};

	struct PropertyGridEvents : public ControlEvents
	{
	};

	class PropertyGridReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;
		
		struct Module
		{

		};
		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		Module m_module;
	};


	class PropertyGrid : public Control<PropertyGridReactor, PropertyGridEvents, PropertyGridAppearance>
	{
	public:
		PropertyGrid() = default;
		PropertyGrid(Window* parent, const Rectangle& rectangle = {});

		
	};
}

#endif
