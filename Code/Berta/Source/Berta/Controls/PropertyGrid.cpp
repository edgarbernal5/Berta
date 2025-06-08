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

		m_module.CalculateViewport(m_module.m_viewport);
	}

	void PropertyGridReactor::Update(Graphics& graphics)
	{
		auto window = m_module.m_owner;

		graphics.DrawRectangle(window->Appearance->BoxBackground, true);

		Point scrollOffset = m_module.m_scrollOffset;
		for (auto it = m_module.m_listModule.Begin(); it < m_module.m_listModule.End(); ++it)
		{
			Rectangle categoryRect{ scrollOffset.X + m_module.m_viewport.m_backgroundRect.X,
				scrollOffset.Y + m_module.m_viewport.m_backgroundRect.Y,
				m_module.m_viewport.m_backgroundRect.Width, m_module.m_viewport.m_categoryItemHeight };

			graphics.DrawRoundRectBox(categoryRect, m_module.m_appearance->ButtonBackground, m_module.m_appearance->BoxBorderColor, true);
			
			Rectangle expanderRect{ scrollOffset.X + m_module.m_viewport.m_backgroundRect.X + m_module.m_viewport.m_categoryTextOffset,
				scrollOffset.Y + m_module.m_viewport.m_backgroundRect.Y + static_cast<int>((m_module.m_viewport.m_categoryItemHeight - m_module.m_viewport.m_expanderButtonSize) >> 1),
				m_module.m_viewport.m_expanderButtonSize, m_module.m_viewport.m_expanderButtonSize };

			int arrowWidth = window->ToScale(4);
			int arrowLength = window->ToScale(2);
			graphics.DrawArrow(expanderRect, arrowLength, arrowWidth, it->m_isExpanded ? Graphics::ArrowDirection::Downwards: Graphics::ArrowDirection::Right, m_module.m_appearance->Foreground);

			Point textOffset = { static_cast<int>(m_module.m_viewport.m_expanderButtonSize) + m_module.m_viewport.m_categoryTextOffset,static_cast<int>(m_module.m_viewport.m_categoryItemHeight) - static_cast<int>(graphics.GetTextExtent().Height) };
			textOffset.Y >>= 1;

			graphics.DrawString({ categoryRect.X + textOffset.X,categoryRect.Y + textOffset.Y }, it->m_name, m_module.m_appearance->Foreground);

			scrollOffset.Y += categoryRect.Height;

			for (size_t i = 0; i < it->m_properties.size(); i++)
			{
				auto field = it->m_properties[i].get();
				bool fieldVisible = it->m_isExpanded;
				auto fieldContainer = it->m_fieldContainers[i].get();
				auto fieldSize = field->GetSize();

				if (it->m_isExpanded)
				{
					if (scrollOffset.Y + fieldSize < 0 || scrollOffset.Y - m_module.m_viewport.m_backgroundRect.Y > m_module.m_viewport.m_backgroundRect.Height)
					{
						fieldVisible = false;
					}
				}

				if (fieldVisible)
				{

				}

				GUI::ShowWindow(*fieldContainer, fieldVisible);
			}
		}

		graphics.DrawRectangle(window->Appearance->BoxBorderColor, false);
	}

	void PropertyGridReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.CalculateViewport(m_module.m_viewport);
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

		CategoryItem newCategory = { this, m_listModule.CreateCategory(categoryName) };
		CalculateViewport(m_viewport);

		return newCategory;
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
		CalculateViewport(m_viewport);
	}

	void PropertyGridReactor::Module::CalculateViewport(ViewportData& viewportData)
	{
		viewportData.m_backgroundRect = m_owner->ClientSize.ToRectangle();
		viewportData.m_backgroundRect.X = viewportData.m_backgroundRect.Y = 1;
		viewportData.m_backgroundRect.Width -= 2u;
		viewportData.m_backgroundRect.Height -= 2u;

		viewportData.m_categoryItemHeight = m_owner->ToScale(m_appearance->CategoryHeight);
		viewportData.m_expanderButtonSize = m_owner->ToScale(m_appearance->ExpanderButtonSize);
		viewportData.m_categoryTextOffset = m_owner->ToScale(4);
		viewportData.m_contentSize = viewportData.m_categoryItemHeight * m_listModule.Size();

		for (auto it = m_listModule.Begin(); it < m_listModule.End(); ++it)
		{

		}
		viewportData.m_needVerticalScroll = viewportData.m_contentSize > viewportData.m_backgroundRect.Height;
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
		auto newField = m_category->m_properties.back().get();

		std::unique_ptr<FieldControlContainter> containerPtr(new FieldControlContainter(m_module->m_owner));
		
		newField->Init(containerPtr->Handle());
		
		m_category->m_fieldContainers.emplace_back(std::move(containerPtr));
		

		return { m_module,newField };
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

	void PropertyGridField::Init(Window* parent)
	{
		Create(parent);
	}

	std::string PropertyGridField::GetLabel() const
	{
		return m_label;
	}

	void PropertyGridField::SetLabel(const std::string& label)
	{
		if (m_label == label)
			return;

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

	void PropertyGridField::Draw(Graphics& graph, Rectangle area)
	{
	}

	void PropertyGridField::Update()
	{
	}

	FieldControlContainter::FieldControlContainter(Window* parent, const Rectangle& rect) : 
		Panel(parent, rect)
	{
	}
}
