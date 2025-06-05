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
	void PropertyGridReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();

		graphics.DrawRectangle(window->Appearance->BoxBackground, true);


		graphics.DrawRectangle(window->Appearance->BoxBorderColor, false);
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
		return m_reactor.GetModule().Append(categoryName);
	}

	void PropertyGrid::Clear()
	{
	}

	CategoryItem PropertyGrid::Insert(CategoryItem existingCategory, const std::string& categoryName)
	{
		return { nullptr,nullptr };
	}

	CategoryItem PropertyGridReactor::Module::Append(const std::string& categoryName) 
	{
		auto category = Find(categoryName);
		if (category)
		{
			return category;
		}

		return { this, m_listModule.CreateCategory(categoryName) };
	}

	CategoryItem PropertyGridReactor::Module::Find(const std::string& categoryName)
	{
		auto it = m_listModule.Begin();
		while (it != m_listModule.End())
		{
			if (it->m_name == categoryName)
			{
				return { this, &(*it) };
			}
			++it;
		}

		return { };
	}

	PropertyGridReactor::ListModule::ListModule()
	{
		m_categories.emplace_back();
	}

	CategoryType* PropertyGridReactor::ListModule::CreateCategory(const std::string& categoryName)
	{
		m_categories.emplace_back(categoryName);
		return &m_categories.back();
	}

	std::vector<CategoryType>::iterator PropertyGridReactor::ListModule::Begin()
	{
		return m_categories.begin();
	}

	std::vector<CategoryType>::const_iterator PropertyGridReactor::ListModule::Begin() const
	{
		return m_categories.cbegin();
	}

	std::vector<CategoryType>::iterator PropertyGridReactor::ListModule::End()
	{
		return m_categories.end();
	}

	std::vector<CategoryType>::const_iterator PropertyGridReactor::ListModule::End() const
	{
		return m_categories.cend();
	}

	CategoryItem::operator bool() const
	{
		return m_module != nullptr && m_category != nullptr;
	}
}
