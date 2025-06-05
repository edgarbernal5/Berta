/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGrid.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/EnumTypes.h"

#include <numeric>

namespace Berta
{
	void PropertyGridReactor::Init(ControlBase& control)
	{
	}

	void PropertyGridReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();

		graphics.DrawRectangle(window->Appearance->BoxBackground, true);
	}

	PropertyGrid::PropertyGrid(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "PropertyGrid";
#endif
	}

	CategoryItem PropertyGrid::Append(const std::string& categoryName)
	{
		return CategoryItem();
	}

	CategoryItem PropertyGrid::Insert(CategoryItem existingCategory, const std::string& categoryName)
	{
		return CategoryItem();
	}
}
