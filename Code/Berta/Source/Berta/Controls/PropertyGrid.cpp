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
		m_control = &control;
		m_module.m_owner = control.Handle();

		m_module.m_appearance = reinterpret_cast<PropertyGridAppearance*>(m_module.m_owner->Appearance.get());
	}

	void PropertyGridReactor::Update(Graphics& graphics)
	{
		auto window = m_module.m_owner;

		graphics.DrawRectangle(window->Appearance->BoxBackground, true);
		for (auto it = m_module.m_listModule.Begin(); it < m_module.m_listModule.End(); ++it)
		{
			//graphics.DrawString({ 0,0 }, it->m_name, m_module.m_appearance->Foreground);
		}

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
		m_reactor.GetModule().Clear();
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

	void PropertyGridReactor::Module::Clear()
	{
	}

	PropertyGridReactor::ListModule::ListModule()
	{
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

	PropertyItem CategoryItem::Append(PropertyGridFieldPtr propGridItem)
	{
		m_category->m_properties.emplace_back(std::move(propGridItem));

		return { m_module, m_category->m_properties.back().get()};
	}

	CategoryItem::operator bool() const
	{
		return m_module != nullptr && m_category != nullptr;
	}

	std::string PropertyItem::GetLabel() const
	{
		return m_propGridField->GetLabel();
	}
	
	PropertyItem& PropertyItem::SetLabel(const std::string& label)
	{
		return *this;
	}
	
	std::string PropertyItem::GetValue() const
	{
		return std::string();
	}
	
	PropertyItem& PropertyItem::SetValue(const std::string& value, bool emit)
	{
		return *this;
	}

	std::string PropertyGridField::GetLabel() const
	{
		return m_label;
	}

	void PropertyGridField::SetLabel(const std::string& label)
	{
		m_label = label;
	}

	std::string PropertyGridField::GetValue() const
	{
		return m_value;
	}

	void PropertyGridField::SetValue(const std::string& value)
	{
		if (m_value == value)
			return;

		m_value = value;
		Update();
	}

	std::string PropertyGridField::GetDefaultValue() const
	{
		return m_defaultValue;
	}

	void PropertyGridField::SetDefaultValue(const std::string& value)
	{
		if (m_defaultValue == value)
			return;

		m_defaultValue = value;
		Update();
	}

	void PropertyGridField::Update()
	{
	}
}
