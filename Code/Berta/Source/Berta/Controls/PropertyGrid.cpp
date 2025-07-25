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
	void PropertyGridReactor::Init(ControlBase& control, Graphics* graphics)
	{
		m_control = &control;
		m_module.m_owner = control.Handle();

		m_module.m_appearance = reinterpret_cast<PropertyGridAppearance*>(m_module.m_owner->Appearance.get());
		m_module.m_events = reinterpret_cast<PropertyGridEvents*>(m_module.m_owner->Events.get());

		m_module.m_graphics = graphics;
		m_module.CalculateViewport(m_module.m_viewport);
	}

	void PropertyGridReactor::Update(Graphics& graphics)
	{
		m_module.Update();
	}

	void PropertyGridReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_mouseInteraction.m_hoveredCategory)
		{
			m_module.m_mouseInteraction.m_hoveredCategory = nullptr;
			GUI::UpdateWindow(m_module.m_owner);
		}
	}

	void PropertyGridReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		auto category = m_module.GetCategoryOnMouse(args.Position);
		m_module.m_mouseInteraction.m_selectedCategory = category;
	}

	void PropertyGridReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		if (args.ButtonState.LeftButton)
			return;

		auto category = m_module.GetCategoryOnMouse(args.Position);
		if (category != m_module.m_mouseInteraction.m_hoveredCategory)
		{
			m_module.m_mouseInteraction.m_hoveredCategory = category;
			GUI::UpdateWindow(m_module.m_owner);
		}
	}

	void PropertyGridReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_mouseInteraction.m_selectedCategory)
		{
			m_module.m_mouseInteraction.m_selectedCategory->m_isExpanded = !m_module.m_mouseInteraction.m_selectedCategory->m_isExpanded;
			m_module.m_mouseInteraction.m_selectedCategory = nullptr;

			m_module.BuildItems();
			GUI::UpdateWindow(m_module.m_owner);
		}
	}

	void PropertyGridReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.CalculateViewport(m_module.m_viewport);
		m_module.BuildItems();
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
		BuildItems();

		return newCategory;
	}

	void PropertyGridReactor::Module::BuildItems() 
	{
		auto one = m_owner->ToScale(1);
		Point offset{};
		for (auto it = m_listModule.Begin(); it < m_listModule.End(); ++it)
		{
			Rectangle categoryRect{ offset.X + m_viewport.m_backgroundRect.X,
				offset.Y + m_viewport.m_backgroundRect.Y,
				m_viewport.m_backgroundRect.Width, m_viewport.m_categoryItemHeight };

			it->m_area = categoryRect;
			offset.Y += categoryRect.Height;

			if (it->m_isExpanded)
			{
				for (size_t i = 0; i < it->m_properties.size(); i++)
				{
					auto field = it->m_properties[i].get();
					auto fieldSize = field->GetSize();

					offset.Y += fieldSize;
				}
			}
			offset.Y += one;
		}
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
		
		CalculateContentSize(viewportData);

		viewportData.m_needVerticalScroll = viewportData.m_contentSize > viewportData.m_backgroundRect.Height;
	}

	void PropertyGridReactor::Module::CalculateContentSize(ViewportData& viewportData)
	{
		viewportData.m_contentSize = viewportData.m_categoryItemHeight * static_cast<uint32_t>(m_listModule.Size());

		for (auto it = m_listModule.Begin(); it < m_listModule.End(); ++it)
		{
			if (it->m_isExpanded)
			{
				for (size_t i = 0; i < it->m_properties.size(); i++)
				{
					auto field = it->m_properties[i].get();
					bool fieldVisible = it->m_isExpanded;
					auto fieldContainer = it->m_fieldContainers[i].get();
					auto fieldSize = field->GetSize();

					viewportData.m_contentSize += fieldSize;
				}
			}
		}
	}

	void PropertyGridReactor::Module::UpdateScrollBar()
	{
		auto scrollSize = m_owner->ToScale(m_owner->Appearance->ScrollBarSize);
		if (m_viewport.m_needVerticalScroll)
		{
			Rectangle scrollRect{ static_cast<int>(m_owner->ClientSize.Width - scrollSize) - 1, 1, scrollSize, m_owner->ClientSize.Height - 2u };
			
			if (!m_scrollBar)
			{
				m_scrollBar = std::make_unique<ScrollBar>(m_owner, false, scrollRect);
				m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						m_scrollOffset.Y = args.Value;

						GUI::UpdateWindow(m_owner);
					});
			}
			else
			{
				GUI::MoveWindow(m_scrollBar->Handle(), scrollRect);
			}
		}
		else if (m_scrollBar)
		{
			m_scrollBar.reset();
			m_scrollOffset.Y = 0;
		}
	}

	void PropertyGridReactor::Module::Update()
	{
		auto& graphics = *m_graphics;
		graphics.DrawRectangle(m_owner->Appearance->BoxBackground, true);

		auto one = m_owner->ToScale(1);
		for (auto it = m_listModule.Begin(); it < m_listModule.End(); ++it)
		{
			Rectangle categoryRect = it->m_area;
			categoryRect.X += m_scrollOffset.X + m_viewport.m_backgroundRect.X;
			categoryRect.Y += m_scrollOffset.Y + m_viewport.m_backgroundRect.Y;

			bool isCategoryHovered = m_mouseInteraction.m_hoveredCategory == &(*it);
			if (isCategoryHovered)
			{
				graphics.DrawRoundRectBox(categoryRect, m_appearance->ButtonHighlightBackground, m_appearance->BoxBorderColor, true);
			}
			else
			{
				graphics.DrawRoundRectBox(categoryRect, m_appearance->ButtonBackground, m_appearance->BoxBorderColor, true);
			}

			Rectangle expanderRect{ categoryRect.X + m_viewport.m_categoryTextOffset,
				categoryRect.Y + static_cast<int>((m_viewport.m_categoryItemHeight - m_viewport.m_expanderButtonSize) >> 1),
				m_viewport.m_expanderButtonSize, m_viewport.m_expanderButtonSize };

			int arrowWidth = m_owner->ToScale(4);
			int arrowLength = m_owner->ToScale(2);
			
			graphics.DrawArrow(expanderRect,
				arrowLength,
				arrowWidth,
				it->m_isExpanded ? Graphics::ArrowDirection::Downwards : Graphics::ArrowDirection::Right,
				m_appearance->Foreground2nd,
				true,
				it->m_isExpanded ? m_appearance->Foreground2nd : m_appearance->BoxBackground
			);

			Point textOffset = { expanderRect.X  + static_cast<int>(expanderRect.Width) + m_viewport.m_categoryTextOffset,static_cast<int>(m_viewport.m_categoryItemHeight) - static_cast<int>(graphics.GetTextExtent().Height) };
			textOffset.Y >>= 1;

			graphics.DrawString({ textOffset.X,categoryRect.Y + textOffset.Y }, it->m_name, m_appearance->Foreground);

			Point scrollOffset{ 0,categoryRect.Y };
			scrollOffset.Y += categoryRect.Height;

			for (size_t i = 0; i < it->m_properties.size(); i++)
			{
				auto field = it->m_properties[i].get();
				bool fieldVisible = it->m_isExpanded;
				auto fieldContainer = it->m_fieldContainers[i].get();
				auto fieldSize = field->GetSize();

				if (it->m_isExpanded)
				{
					if (scrollOffset.Y + static_cast<int>(fieldSize) < 0 || scrollOffset.Y - m_viewport.m_backgroundRect.Y > static_cast<int>(m_viewport.m_backgroundRect.Height))
					{
						fieldVisible = false;
					}
				}

				if (fieldVisible)
				{
					Rectangle fieldArea{ scrollOffset.X + one, scrollOffset.Y + one,m_viewport.m_backgroundRect.Width - one * 2,fieldSize - one * 2 };
					Rectangle fieldContainerArea = fieldArea;
					fieldContainerArea.X += fieldArea.Width >> 1;
					fieldContainerArea.Width -= fieldArea.Width >> 1;

					GUI::MoveWindow(*fieldContainer, fieldContainerArea);

					field->Draw(graphics, fieldArea, m_viewport.m_backgroundRect.Width >> 1, m_appearance->Foreground);
					scrollOffset.Y += fieldSize;
				}

				GUI::ShowWindow(*fieldContainer, fieldVisible);
			}
		}

		graphics.DrawRectangle(m_owner->Appearance->BoxBorderColor, false);
	}

	CategoryType* PropertyGridReactor::Module::GetCategoryOnMouse(const Point& mousePosition)
	{
		Point offsetPosition = mousePosition + m_scrollOffset;
		for (auto it = m_listModule.Begin(); it < m_listModule.End(); ++it)
		{
			if (it->m_area.IsInside(offsetPosition))
				return &(*it);
		}
		return nullptr;
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

		m_module->BuildItems();
		return { m_module, newField };
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
		m_parent = parent;
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

	bool PropertyGridField::IsEnabled() const
	{
		return m_enabled;
	}

	void PropertyGridField::SetEnabled(bool enabled)
	{
		m_enabled = enabled;
	}

	void PropertyGridField::Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor)
	{
		Rectangle labelArea = area;
		labelArea.Width = labelWidth;

		DrawLabel(graphics, labelArea, textColor);
	}

	void PropertyGridField::Update()
	{
	}

	void PropertyGridField::DrawLabel(Graphics& graphics, const Rectangle& area, const Color& textColor)
	{
		auto textExtents = graphics.GetTextExtent();
		Point position = area;
		position.Y += static_cast<int>((area.Height - textExtents.Height) >> 1);

		graphics.DrawString(position, m_label, textColor);
	}

	FieldControlContainter::FieldControlContainter(Window* parent, const Rectangle& rect) : 
		Panel(parent, rect)
	{
	}
}
