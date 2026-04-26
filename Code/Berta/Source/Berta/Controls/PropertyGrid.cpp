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
	namespace Internal::PropertyGrid
	{
		void PropertyGridFieldBase::Init(Window* parent)
		{
			m_parent = parent;
			OnCreate(parent);
			SetEnabled(IsEnabled());
		}

		std::string_view PropertyGridFieldBase::GetLabel() const
		{
			return m_label;
		}

		void PropertyGridFieldBase::SetLabel(std::string_view newLabel)
		{
			if (m_label == newLabel)
			{
				return;
			}

			m_label = newLabel;
		}

		bool PropertyGridFieldBase::IsEnabled() const
		{
			return m_enabled;
		}

		void PropertyGridFieldBase::SetEnabled(bool enabled)
		{
			if (m_enabled == enabled)
			{
				return;
			}
			m_enabled = enabled;
			OnEnableChanged(enabled);
		}

		bool PropertyGridFieldBase::IsVisible() const
		{
			return m_isVisible;
		}

		void PropertyGridFieldBase::SetVisibility(bool visible)
		{
			if (m_isVisible == visible)
			{
				return;
			}
			
			m_isVisible = visible;
			OnVisibilityChanged(visible);
		}

		void PropertyGridFieldBase::NotifyValueChanged()
		{
			if (OnValueChanged)
			{ 
				OnValueChanged(); 
			}
		}

		void PropertyGridFieldBase::NotifySelected()
		{
			if (OnSelected)
			{
				OnSelected();
			}
		}

		void PropertyGridModel::Init(Window* ownerWindow)
		{
			if (m_ownerWindow == ownerWindow)
			{
				return;
			}
        
			m_ownerWindow = ownerWindow;
			for (auto& cat : m_rootCategories)
			{
				InitCategoryRecursive(cat);
			}
		}

		CategoryType& PropertyGridModel::AppendRootCategory(std::string_view categoryName)
		{
			StringUtils::StringHash id = StringUtils::Hash(categoryName);
            
			if (auto* existing = FindCategoryById(id))
			{
				return *existing;
			}
			StringUtils::StringHash localHash = StringUtils::Hash(categoryName);
			return m_rootCategories.emplace_back(localHash, categoryName, 0u);
		}

		CategoryType* PropertyGridModel::AppendSubCategory(StringUtils::StringHash parentId, std::string_view name)
		{
			CategoryType* parent = FindCategoryById(parentId);
			if (!parent)
			{
				return nullptr;
			}
			
			StringUtils::StringHash localHash = StringUtils::Hash(name);
			StringUtils::StringHash globalId = StringUtils::HashCombine(parentId, localHash);
        
			// Evitar duplicados en el mismo nivel
			auto it = std::find_if(parent->m_subCategories.begin(), parent->m_subCategories.end(),
			                       [globalId](const CategoryType& c) { return c.m_id == globalId; });

			if (it != parent->m_subCategories.end())
			{
				return &(*it);
			}
			
			return &parent->m_subCategories.emplace_back(globalId, name, parent->m_depth + 1u);
		}

		void PropertyGridModel::AppendPropertyToCategory(StringUtils::StringHash categoryId, StringUtils::StringHash propId, std::unique_ptr<PropertyGridFieldBase> field)
		{
			if (!field)
			{
				return;
			}
				
			field->OnValueChanged = [this, propId]()
			{
				if (OnPropertyModified)
				{
					OnPropertyModified(propId);
				}
			};
			field->OnSelected = [this, propId]()
			{
				if (OnPropertySelected)
				{
					OnPropertySelected(propId);
				}
			};
			
			if (m_ownerWindow)
			{
				field->Init(m_ownerWindow); 
			}
			if (CategoryType* cat = FindCategoryById(categoryId))
			{
				cat->m_properties.push_back({ propId, std::move(field) });
				PropertyFieldData* pointerToMemory = &cat->m_properties.back();
				m_propertyLookup[propId] = pointerToMemory;
			}
		}

		const CategoryType* PropertyGridModel::FindCategoryById(StringUtils::StringHash id) const
		{
			return FindRecursive(id, m_rootCategories);
		}

		CategoryType* PropertyGridModel::FindCategoryById(StringUtils::StringHash id)
		{
			return FindRecursive(id, m_rootCategories);
		}

		PropertyFieldData* PropertyGridModel::FindPropertyById(StringUtils::StringHash m_uniqueId) const
		{
			auto it = m_propertyLookup.find(m_uniqueId);
			return (it != m_propertyLookup.end()) ? it->second : nullptr;
		}

		void PropertyGridModel::Clear()
		{
			m_rootCategories.clear();
			m_selectedItemId = 0;
		}

		bool PropertyGridModel::RemoveProperty(StringUtils::StringHash propId)
		{
			m_propertyLookup.erase(propId);
			
			for (auto& rootCat : m_rootCategories)
			{
				if (RemovePropertyRecursive(rootCat, propId))
				{
					return true;
				}
			}
			return false;
		}

		bool PropertyGridModel::RemoveCategory(StringUtils::StringHash catId)
		{
			auto it = std::find_if(m_rootCategories.begin(), m_rootCategories.end(),
				[catId](const auto& cat) { return cat.m_id == catId; });

			if (it != m_rootCategories.end())
			{
				CleanUpCategoryLookup(*it);
				m_rootCategories.erase(it);
				return true;
			}

			for (auto& rootCat : m_rootCategories)
			{
				if (RemoveSubCategoryRecursive(rootCat, catId))
				{
					return true;
				}
			}
			return false;
		}

		bool PropertyGridModel::GetPropertyEnabled(StringUtils::StringHash propId)
		{
			if (PropertyFieldData* prop = FindPropertyById(propId))
			{
				return prop->field->IsEnabled();
			}
			return false;
		}

		void PropertyGridModel::SetPropertyEnabled(StringUtils::StringHash propId, bool enabled)
		{
			if (PropertyFieldData* prop = FindPropertyById(propId))
			{
				prop->field->SetEnabled(enabled);
			}
		}

		std::string_view PropertyGridModel::GetPropertyLabel(StringUtils::StringHash propId)
		{
			if (PropertyFieldData* prop = FindPropertyById(propId))
			{
				return prop->field->GetLabel();
			}
			return "";
		}

		void PropertyGridModel::SetPropertyLabel(StringUtils::StringHash propId, std::string_view newLabel)
		{
			if (PropertyFieldData* prop = FindPropertyById(propId))
			{
				prop->field->SetLabel(newLabel);
				if (OnVisualsChanged)
				{
					OnVisualsChanged(); 
				}
			}
		}

		std::string PropertyGridModel::GetPropertyValueAsString(StringUtils::StringHash propId)
		{
			if (PropertyFieldData* prop = FindPropertyById(propId))
			{
				return prop->field->GetValueAsString();
			}
        
			return {};
		}

		bool PropertyGridModel::IsCategory(StringUtils::StringHash itemId) const
		{
			for (const auto& rootCat : m_rootCategories)
			{
				if (IsCategoryRecursive(rootCat, itemId))
				{
					return true;
				}
			}
			return false;
		}

		bool PropertyGridModel::IsCategoryExpanded(StringUtils::StringHash catId)
		{
			if (CategoryType* category = FindCategoryById(catId))
			{
				return category->m_isExpanded;
			}
			return false;
		}

		void PropertyGridModel::ToggleCategoryExpansion(StringUtils::StringHash catId)
		{
			if (CategoryType* category = FindCategoryById(catId))
			{
				category->m_isExpanded = !category->m_isExpanded;
			}
		}

		void PropertyGridModel::SetCategoryIcon(StringUtils::StringHash catId, const Image& icon)
		{
			if (CategoryType* cat = FindCategoryById(catId))
			{
				cat->m_icon = icon;
			}
		}

		bool PropertyGridModel::IsRootCategory(StringUtils::StringHash catId)
		{
			return std::any_of(m_rootCategories.begin(), m_rootCategories.end(),
			                   [catId](const auto& cat) { return cat.m_id == catId; });
			
			//1. Evitar iteradores manuales si solo quieres saber si algo existe:
			// Refactor más limpio si extraes esto a la caché, o mantenlo así que está muy bien.
		}

		void PropertyGridModel::MoveRootCategory(size_t fromIndex, size_t toIndex)
		{
			if (fromIndex == toIndex)
			{
				return;
			}
			if (fromIndex >= m_rootCategories.size())
			{
				return;
			}

			if (toIndex >= m_rootCategories.size())
				toIndex = m_rootCategories.size() - 1;
			
			auto itFrom = m_rootCategories.begin() + fromIndex;
			auto itTo = m_rootCategories.begin() + toIndex;

			// std::rotate empuja inteligentemente la memoria de los punteros
			// sin instanciar copias pesadas. Es 100% exception-safe.
			if (fromIndex < toIndex)
			{
				std::rotate(itFrom, itFrom + 1, itTo + 1);
			}
			else
			{
				std::rotate(itTo, itFrom, itFrom + 1);
			}
		}

		size_t PropertyGridModel::GetRootCategoryIndex(StringUtils::StringHash catId)
		{
			auto it = std::find_if(m_rootCategories.begin(), m_rootCategories.end(),
			                       [catId](const auto& cat) { return cat.m_id == catId; });

			if (it != m_rootCategories.end()) 
			{
				return std::distance(m_rootCategories.begin(), it);
			}

			return 0;
		}

		StringUtils::StringHash PropertyGridModel::GetParentCategory(StringUtils::StringHash propId) const
		{
			return GetParentCategoryRecursive(m_rootCategories, propId);
		}

		StringUtils::StringHash PropertyGridModel::GetParentId(StringUtils::StringHash childId) const
		{
			for (const auto& rootCat : m_rootCategories)
			{
				if (rootCat.m_id == childId)
				{
					return 0; 
				}

				StringUtils::StringHash foundParent = GetParentIdRecursive(rootCat, childId);
				if (foundParent != 0) {
					return foundParent;
				}
			}
			return 0;
		}

		CategoryType* PropertyGridModel::FindRecursive(StringUtils::StringHash id, std::vector<CategoryType>& list)
		{
			for (auto& cat : list)
			{
				if (cat.m_id == id)
				{
					return &cat;
				}
				if (!cat.m_subCategories.empty())
				{
					if (auto* found = FindRecursive(id, cat.m_subCategories))
					{
						return found;
					}
				}
			}
			return nullptr;
		}

		const CategoryType* PropertyGridModel::FindRecursive(StringUtils::StringHash id, const std::vector<CategoryType>& list) const
		{
			for (const auto& cat : list)
			{
				if (cat.m_id == id)
				{
					return &cat;
				}
				if (!cat.m_subCategories.empty())
				{
					if (const auto* found = FindRecursive(id, cat.m_subCategories))
					{
						return found;
					}
				}
			}
			return nullptr;
		}

		void PropertyGridModel::InitCategoryRecursive(CategoryType& cat)
		{
			for (auto& prop : cat.m_properties)
			{
				prop.field->Init(m_ownerWindow); 
			}
			
			for (auto& subCat : cat.m_subCategories)
			{
				InitCategoryRecursive(subCat);
			}
		}

		bool PropertyGridModel::IsCategoryRecursive(const CategoryType& category, StringUtils::StringHash id) const
		{
			if (category.m_id == id)
			{
				return true;
			}

			for (const auto& subCat : category.m_subCategories)
			{
				if (IsCategoryRecursive(subCat, id))
				{
					return true;
				}
			}

			return false;
		}

		StringUtils::StringHash PropertyGridModel::GetParentCategoryRecursive(const std::vector<CategoryType>& list, StringUtils::StringHash propId) const
		{
			for (const auto& cat : list)
			{
				auto it = std::find_if(cat.m_properties.begin(), cat.m_properties.end(),
					[propId](const auto& prop) { return prop.m_id == propId; });

				if (it != cat.m_properties.end())
				{
					return cat.m_id;
				}
				
				auto subHash = GetParentCategoryRecursive(cat.m_subCategories, propId);
				if (subHash)
				{
					return subHash;
				}
				
			}
			return 0;
		}

		StringUtils::StringHash PropertyGridModel::GetParentIdRecursive(const CategoryType& currentCat, StringUtils::StringHash targetId) const
		{
			for (const auto& prop : currentCat.m_properties)
			{
				if (prop.m_id == targetId)
				{
					return currentCat.m_id;
				}
			}

			for (const auto& subCat : currentCat.m_subCategories)
			{
				if (subCat.m_id == targetId)
				{
					return currentCat.m_id;
				}

				StringUtils::StringHash foundInSub = GetParentIdRecursive(subCat, targetId);
				if (foundInSub != 0) {
					return foundInSub;
				}
			}

			return 0;
		}

		bool PropertyGridModel::RemovePropertyRecursive(CategoryType& category, StringUtils::StringHash propId)
		{
			auto it = std::find_if(category.m_properties.begin(), category.m_properties.end(),
				[propId](const auto& prop) { return prop.m_id == propId; });

			if (it != category.m_properties.end())
			{
				category.m_properties.erase(it);
				return true;
			}

			for (auto& subCat : category.m_subCategories)
			{
				if (RemovePropertyRecursive(subCat, propId))
				{
					return true;
				}
			}

			return false;
		}

		bool PropertyGridModel::RemoveSubCategoryRecursive(CategoryType& parentCat, StringUtils::StringHash targetCatId)
		{
			auto it = std::find_if(parentCat.m_subCategories.begin(), parentCat.m_subCategories.end(),
			[targetCatId](const auto& cat) { return cat.m_id == targetCatId; });

			if (it != parentCat.m_subCategories.end())
			{
				CleanUpCategoryLookup(*it); // Limpiar caché
				parentCat.m_subCategories.erase(it);
				return true;
			}

			for (auto& subCat : parentCat.m_subCategories)
			{
				if (RemoveSubCategoryRecursive(subCat, targetCatId))
				{
					return true;
				}
			}
			return false;
		}

		void PropertyGridModel::CleanUpCategoryLookup(const CategoryType& category)
		{
			for (const auto& prop : category.m_properties)
			{
				m_propertyLookup.erase(prop.m_id);
			}
			for (const auto& subCat : category.m_subCategories)
			{
				CleanUpCategoryLookup(subCat);
			}
		}

		PropertyHandle::operator bool() const
		{
			return m_model;
		}

		std::string_view PropertyHandle::GetLabel() const
		{
			return m_model ? m_model->GetPropertyLabel(m_uniqueId) : "";
		}

		PropertyHandle& PropertyHandle::SetLabel(std::string_view newLabel)
		{
			if (m_model)
			{
				m_model->SetPropertyLabel(m_uniqueId, newLabel);
			}
			return *this;
		}

		std::string PropertyHandle::GetValueAsString() const
		{
			return m_model ? m_model->GetPropertyValueAsString(m_uniqueId) : "";
		}

		bool PropertyHandle::IsEnabled() const
		{
			if (m_model)
			{
				return m_model->GetPropertyEnabled(m_uniqueId);
			}
			return false;
		}

		PropertyHandle& PropertyHandle::SetEnabled(bool enabled)
		{
			if (m_model)
			{
				m_model->SetPropertyEnabled(m_uniqueId, enabled);
			}
			return *this;
		}

		CategoryHandle CategoryHandle::AppendCategory(std::string_view name)
		{
			CategoryType& rawCategory = m_model->AppendRootCategory(name);

			m_model->OnVisualsChanged();

			return {m_model, rawCategory.m_id};
		}

		CategoryHandle CategoryHandle::AppendSubCategory(std::string_view name)
		{
			if (m_model)
			{
				CategoryType* rawSubCat = m_model->AppendSubCategory(m_id, name);
				if (rawSubCat)
				{
					return {m_model, rawSubCat->m_id};
				}
			}
			return {};
		}

		PropertyHandle CategoryHandle::AppendProperty(StringUtils::StringHash catId, std::unique_ptr<PropertyGridFieldBase> field)
		{
			if (m_model && field)
			{
				StringUtils::StringHash propLocalHash = StringUtils::Hash(field->GetLabel());
				StringUtils::StringHash globalUniqueId = StringUtils::HashCombine(catId, propLocalHash);
				
				m_model->AppendPropertyToCategory(catId, globalUniqueId, std::move(field));
				
				return {m_model, globalUniqueId};
			}
			return {};
		}

		CategoryHandle& CategoryHandle::SetIcon(const Image& icon)
		{
			if (m_model)
			{
				m_model->SetCategoryIcon(m_id, icon);
			}
			return *this;
		}

		CategoryHandle::operator bool() const
		{
			return m_model != nullptr;
		}

		void PropertyGridLayout::Init(Window* owner, Appearance* config)
		{
			if (m_isInitialized)
			{
				return;
			}
			m_owner = owner;
			m_config = config;
			m_internalScrollManager = std::make_unique<ScrollableView>(m_owner);
			m_scrollableView = m_internalScrollManager.get();

			m_scrollableView->SetScrollStep(static_cast<int>(m_owner->ToScale(config->CategoryHeight)), 0);
			m_scrollableView->SetOnScrollChange([this]()
			{
				GUI::MarkAsNeedUpdate(m_owner);
			});
			
			m_isInitialized = true;
		}

		void PropertyGridLayout::CalculateLayout(const PropertyGridModel& model)
		{
			m_itemRects.clear();
			m_visibleItems.clear();
			
			uint32_t currentY = 0;
			for (auto& cat : model.GetRootCategories())
			{
				currentY = CalculateRecursive(cat, 0, currentY);
			}
			SyncControlsVisibility(model);
			
			m_scrollableView->SetContentSize({ 0, currentY });
		}

		void PropertyGridLayout::Draw(Graphics& graphics, const PropertyGridModel& model, Appearance* appearance)
		{
			Point offset = m_scrollableView->GetScrollOffset();
			auto clientArea = m_scrollableView->GetClientArea();
			int currentX = -offset.X + clientArea.X;
			int currentY = -offset.Y + clientArea.Y;

			int viewTop = clientArea.Y; 
			int viewBottom = clientArea.Y + static_cast<int>(clientArea.Height);
			bool isBelowView = false;
			
			for (auto& [itemId, isCategory, itemRect] : m_visibleItems)
			{
				if (isBelowView)
				{
					if (!isCategory) 
					{
						auto prop = model.FindPropertyById(itemId);
						if (prop && prop->field && prop->field->IsVisible())
						{
							prop->field->SetVisibility(false);
						}
					}
					continue; 
				}
				
				Rectangle rect = itemRect;
				rect.Width = clientArea.Width - rect.X;
				rect.X += currentX;
				rect.Y += currentY;
				if ((rect.Y + static_cast<int>(rect.Height) <= viewTop) || (rect.Y >= viewBottom))
				{
					if (!isCategory) 
					{
						auto prop = model.FindPropertyById(itemId);
						if (prop && prop->field && prop->field->IsVisible())
						{
							prop->field->SetVisibility(false);
						}
					}
            
					if (rect.Y >= viewBottom) 
					{
						isBelowView = true; 
					}
					
					continue;
				}
				
				bool isSelected = itemId == model.GetSelectedItemId();
				bool isHovered = itemId == m_hoveredItemId;
				if (isCategory)
				{
					if (const CategoryType* cat = model.FindCategoryById(itemId))
					{
						DrawCategoryHeader(graphics, model, rect, *cat, m_config);
					}
				}
				else
				{
					auto propArea = rect;
					if (isSelected)
					{
						graphics.FillRectangle(propArea, m_config->SelectedBackgroundColor);
					}
					else if (isHovered)
					{
						graphics.FillRectangle(propArea, m_config->HoverBackgroundColor);
					}
					
					auto prop = model.FindPropertyById(itemId);
					if (prop && prop->field)
					{
						if (!prop->field->IsVisible())
						{
							prop->field->SetVisibility(true);
						}
						if (prop->field->IsShowingLabel())
						{
							Rectangle labelArea = rect;
					
							int labelWidth = (int)rect.Width / 2;
							Color textColor = isSelected ? m_config->SelectedTextColor : m_config->Foreground;
							graphics.DrawString({ labelArea.X + 5, labelArea.Y + 4 }, prop->field->GetLabel(), textColor);
							
							propArea.X += labelWidth;
							propArea.Width -= labelWidth;
						}
						propArea.X += 2;
						propArea.Y += 2;
						propArea.Width -= 4;
						propArea.Height -= 4;
						//int splitterX = rect.Width * 0.4f;
						//Rectangle controlRect = { rect.X + splitterX, rect.Y, rect.Width - splitterX, rect.Height };
						
						prop->field->Draw(graphics, propArea, *appearance);
					}
				}
			}
			if (m_showDropIndicator)
			{
				int logicalDropY = 0;
				const auto& rootCategories = model.GetRootCategories();

				if (m_dropIndicatorIndex < rootCategories.size())
				{
					StringUtils::StringHash targetCatId = rootCategories[m_dropIndicatorIndex].m_id;
					Rectangle rect = GetItemRect(targetCatId);
					logicalDropY = rect.Y; 
				}
				else if (!m_visibleItems.empty())
				{
					auto [lastItemId, isCat, lastRect] = m_visibleItems.back();
					logicalDropY = lastRect.Y + static_cast<int>(lastRect.Height);
				}

				int screenDropY = logicalDropY + currentY; // currentY ya tiene el -offset.Y + clientArea.Y

				if (screenDropY >= viewTop && screenDropY <= viewBottom)
				{
					uint32_t dropLineHeight = m_owner->ToScale(3u);
					int dropLineHalf = static_cast<int>(dropLineHeight >> 1);

					Rectangle lineRect = { 
						clientArea.X, 
						screenDropY - dropLineHalf, 
						clientArea.Width, 
						dropLineHeight 
					};

					graphics.FillRectangle(lineRect, Color(0, 120, 215, 255));
				}
			}
		}

		void PropertyGridLayout::RefreshVisibleOnly(const PropertyGridModel& model)
		{
			if (!m_scrollableView)
			{
				return;
			}

			auto clientArea = m_scrollableView->GetClientArea();
			Point offset = m_scrollableView->GetScrollOffset();
			int currentY = -offset.Y + clientArea.Y;
			int viewTop = clientArea.Y; 
			int viewBottom = clientArea.Y + static_cast<int>(clientArea.Height);
        
			for (const auto& [itemId, isCategory, itemRect] : m_visibleItems)
			{
				Rectangle rect = itemRect;
				rect.Y += currentY;
				
				bool isCulled = (rect.Y + static_cast<int>(rect.Height) <= viewTop) || (rect.Y >= viewBottom);
				if (isCulled || isCategory)
				{
					continue;
				}
				auto prop = model.FindPropertyById(itemId);
				if (prop && prop->field)
				{
					if (!prop->field->IsVisible())
					{
						prop->field->Refresh();
					}
				}
			}
		}

		void PropertyGridLayout::ScrollToItem(StringUtils::StringHash targetId)
		{
			if (!m_scrollableView)
			{
				return;
			}
			Rectangle itemRect = GetItemRect(targetId);
			int currentScrollY = m_scrollableView->GetScrollOffset().Y;
			int visibleHeight = static_cast<int>(m_scrollableView->GetClientArea().Height);

			Rectangle viewport = m_scrollableView->GetVisibleRect();
			if (itemRect.Y < currentScrollY) 
			{
				currentScrollY=itemRect.Y;
			}
			else if (itemRect.Y + static_cast<int>(itemRect.Height) > currentScrollY + visibleHeight) 
			{
				currentScrollY = itemRect.Y + static_cast<int>(itemRect.Height) - visibleHeight;
			}

			if (currentScrollY != viewport.Y)
			{
				m_scrollableView->SetScrollToY(currentScrollY); 
			}
			
		}

		Rectangle PropertyGridLayout::GetItemRect(StringUtils::StringHash id) const
		{
			auto it = m_itemRects.find(id);
			if (it != m_itemRects.end())
			{
				return it->second;
			}
    
			return Rectangle{ 0, 0, 0, 0 };
		}

		void PropertyGridLayout::SetDropIndicator(bool show, size_t targetIndex)
		{
			m_showDropIndicator = show;
			m_dropIndicatorIndex = targetIndex;
		}

		uint32_t PropertyGridLayout::CalculateRecursive(const CategoryType& cat, int currentX, uint32_t currentY)
		{
			uint32_t categoryHeaderHeight = m_owner->ToScale(m_config->CategoryHeight);
			auto clientArea = m_scrollableView->GetClientArea();
			Rectangle catRect = { currentX, static_cast<int>(currentY), clientArea.Width, categoryHeaderHeight };
			
			m_itemRects[cat.m_id] = catRect;
			m_visibleItems.emplace_back(cat.m_id, true, catRect);
			
			currentY += categoryHeaderHeight;
			
			if (cat.m_isExpanded)
			{
				for (const auto& prop : cat.m_properties)
				{
					auto propHeight = prop.field->GetHeight();
					Rectangle propRect = { currentX + PG_INDENT_PADDING, static_cast<int>(currentY), clientArea.Width - PG_INDENT_PADDING, propHeight };
					
					m_itemRects[prop.m_id] = propRect;
					m_visibleItems.emplace_back(prop.m_id, false, propRect);
					
					currentY += propHeight;
				}
				
				for (const auto& sub : cat.m_subCategories)
				{
					currentY = CalculateRecursive(sub, currentX + PG_INDENT_PADDING, currentY);
				}
			}
			return currentY;
		}

		void PropertyGridLayout::DrawCategoryHeader(Graphics& graphics, const PropertyGridModel& model, const Rectangle& area, const CategoryType& cat, Appearance* config)
		{
			bool isSelected = cat.m_id == model.GetSelectedItemId();
			
			Color bgColor = isSelected ? config->MenuBackground : config->Background;
			graphics.FillRectangle(area, bgColor);

			int offset = m_owner->ToScale(4);
			int expanderSize = m_owner->ToScale(14);
			int centerY = area.Y + ((int)area.Height - expanderSize) / 2;
			Rectangle expanderArea{ area.X + offset, centerY, (uint32_t)expanderSize, (uint32_t)expanderSize };

			int arrowWidth = m_owner->ToScale(4);
			int arrowLength = m_owner->ToScale(2);

			graphics.DrawArrow
			(
				expanderArea,
				arrowLength,
				arrowWidth,
				cat.m_isExpanded ? Graphics::ArrowDirection::Downwards : Graphics::ArrowDirection::Right,
				config->Foreground2nd,
				true,
				cat.m_isExpanded ? config->Foreground2nd : config->BoxBackground
			);
			
			if (model.IsShowingCategoryIcons())
			{
				auto iconPaddingX = m_owner->ToScale(2);
				auto iconSize = m_owner->ToScale(config->SmallIconSize);
				if (cat.m_icon)
				{
					Rectangle iconRect{ area.X + expanderArea.X + (int)expanderArea.Width + iconPaddingX, area.Y + (((int)area.Height - (int)iconSize) >> 1), iconSize, iconSize };
					cat.m_icon.Paste(graphics, iconRect);
				}
				offset += (int)iconSize + iconPaddingX * 2;
			}
			int textX = expanderArea.X + (int)expanderArea.Width + offset;
			Point textPos = { textX, area.Y + ((int)area.Height - (int)graphics.GetTextExtent().Height) / 2 };
    
			graphics.DrawString(textPos, cat.m_name, config->Foreground);

			Color separatorColor = config->BoxBorderColor;
			graphics.DrawLine({ area.X, area.Y + (int)area.Height - 1 }, { area.X + (int)area.Width, area.Y + (int)area.Height - 1 }, separatorColor);
		}

		void PropertyGridLayout::SyncControlsVisibility(const PropertyGridModel& model)
		{
			for (const auto& rootCat : model.GetRootCategories())
			{
				HideAllControlsRecursive(rootCat);
			}

			for (const auto& [id, isCategory, rect] : m_visibleItems)
			{
				if (!isCategory)
				{
					auto prop = model.FindPropertyById(id);
					if (prop && prop->field)
					{
						prop->field->SetVisibility(true);
					}
				}
			}
		}

		void PropertyGridLayout::HideAllControlsRecursive(const CategoryType& cat) const
		{
			for (const auto& prop : cat.m_properties)
			{
				if (prop.field)
				{
					prop.field->SetVisibility(false);
				}
			}
			for (const auto& subCat : cat.m_subCategories)
			{
				HideAllControlsRecursive(subCat);
			}
		}

		void Module::Draw()
		{
		}

		void Module::Update()
		{
			if (!m_owner->Flags.AutoDraw)
				return;

			GUI::UpdateWindow(m_owner);
		}

		void Module::ClearReferences(StringUtils::StringHash deletedId)
		{
			if (m_model.GetSelectedItemId() == deletedId)
			{
				m_model.SetSelectedItemId(0);
			}
    
			if (m_layout.GetHoveredItemId() == deletedId)
			{
				m_layout.SetHoverItemId(0);
			}
		}

		void Module::OnLayoutChanged()
		{
			m_layout.CalculateLayout(m_model);

			GUI::UpdateWindow(m_owner);
		}

		StringUtils::StringHash Module::HitTest(Point mousePos)
		{
			if (!m_layout.m_scrollableView)
			{
				return 0;
			}
			
			auto offset = m_layout.m_scrollableView->GetScrollOffset();
			auto clientArea = m_layout.m_scrollableView->GetClientArea();
    
			Point virtualPos = { mousePos.X - clientArea.X + offset.X, 
								 mousePos.Y - clientArea.Y + offset.Y };

			for (const auto& [itemId, isCategory, baseRect] : m_layout.GetVisibleItemsList())
			{
				Rectangle hitRect = baseRect;
				hitRect.Width = clientArea.Width;

				if (hitRect.Contains(virtualPos))
				{
					return itemId; 
				}

				if (virtualPos.Y < hitRect.Y) {
					break;
				}
			}

			return 0;
		}

		void Reactor::Update(Graphics& graphics)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_module.m_owner->Appearance.get());
			
			auto globalRect = m_module.m_owner->ClientSize.ToRectangle();
			graphics.FillRectangle(globalRect, m_module.m_owner->Appearance->BoxBackground);
			if (!m_control->IsBorderless())
			{
				graphics.DrawRectangle(globalRect, appearance->BoxBorderColor);
				
				Rectangle localBorderRect = m_control->GetClientArea();
				graphics.SetClipping(localBorderRect);
			}
			
			m_module.m_layout.Draw(graphics, m_module.m_model, appearance);
			
			if (m_module.m_layout.m_scrollableView->HasVerticalScroll() && m_module.m_layout.m_scrollableView->HasHorizontalScroll())
			{
				auto scrollSize = m_module.m_owner->ToScale(m_module.m_owner->Appearance->ScrollBarSize);
				graphics.FillRectangle({ (int)(m_module.m_owner->ClientSize.Width - scrollSize) - 1, (int)(m_module.m_owner->ClientSize.Height - scrollSize) - 1, scrollSize, scrollSize }, m_module.m_owner->Appearance->Background);
			}
		
			if (!m_control->IsBorderless())
			{
				graphics.EndClipping();
			}
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			if (m_module.m_lastHoveredItemId != 0)
			{
				m_module.m_lastHoveredItemId = 0;
				m_module.m_layout.SetHoverItemId(0);
				m_module.OnLayoutChanged();
			}
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			m_module.m_mouseDownPos = args.Position;
			m_module.m_pressedItemId = m_module.HitTest(args.Position);
			
			if (args.ButtonState.LeftButton && m_module.m_pressedItemId != 0)
			{
				if (m_module.m_model.IsRootCategory(m_module.m_pressedItemId)) 
				{
					m_module.m_isWaitingForDrag = true;
					m_module.m_draggedCatIndex = m_module.m_model.GetRootCategoryIndex(m_module.m_pressedItemId);
				}
			}
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			StringUtils::StringHash hitItemId = m_module.HitTest(args.Position);
			
			if (m_module.m_isWaitingForDrag)
			{
				int dx = args.Position.X - m_module.m_mouseDownPos.X;
				int dy = args.Position.Y - m_module.m_mouseDownPos.Y;
        
				if ((dx * dx + dy * dy) > PG_DRAG_THRESHOLD_SQ)
				{
					m_module.m_isWaitingForDrag = false;
					m_module.m_isDraggingCategory = true;
					m_module.m_hoveredDropIndex = m_module.m_draggedCatIndex;
					GUI::Capture(m_module.m_owner);
				}
			}
			if (m_module.m_isDraggingCategory)
			{
				if (hitItemId != 0 && m_module.m_model.IsRootCategory(hitItemId))
				{
					size_t newHoverIndex = m_module.m_model.GetRootCategoryIndex(hitItemId);
					if (newHoverIndex != m_module.m_hoveredDropIndex)
					{
						m_module.m_hoveredDropIndex = newHoverIndex;
						m_module.m_layout.SetDropIndicator(true, m_module.m_hoveredDropIndex);
						//m_module.OnLayoutChanged();
						GUI::MarkAsNeedUpdate(m_module.m_owner);
					}
				}
				else if (hitItemId == 0 && !m_module.m_layout.GetVisibleItemsList().empty())
				{
					auto mousePos = args.Position + m_module.m_layout.m_scrollableView->GetScrollOffset();
					
					auto [id, isCategory, rect] = m_module.m_layout.GetVisibleItemsList().back();
					if (mousePos.Y <= 0)
					{
						m_module.m_hoveredDropIndex = 0;
						m_module.m_layout.SetDropIndicator(true, m_module.m_hoveredDropIndex);
						GUI::MarkAsNeedUpdate(m_module.m_owner);
					}
					else if (mousePos.Y > rect.Y + static_cast<int>(rect.Height))
					{
						m_module.m_hoveredDropIndex = m_module.m_layout.GetVisibleItemsList().size();
						m_module.m_layout.SetDropIndicator(true, m_module.m_hoveredDropIndex);
						GUI::MarkAsNeedUpdate(m_module.m_owner);
					}
				}
				return;
			}
			
			if (hitItemId != m_module.m_lastHoveredItemId)
			{
				m_module.m_lastHoveredItemId = hitItemId;

				m_module.m_layout.SetHoverItemId(hitItemId);
				m_module.OnLayoutChanged();
				GUI::MarkAsNeedUpdate(m_module.m_owner);
			}
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{			
			StringUtils::StringHash releaseItemId = releaseItemId = m_module.HitTest(args.Position);
			
			if (m_module.m_isDraggingCategory)
			{
				GUI::ReleaseCapture(m_module.m_owner);
				if (m_module.m_draggedCatIndex != m_module.m_hoveredDropIndex)
				{
					m_module.m_model.MoveRootCategory(m_module.m_draggedCatIndex, m_module.m_hoveredDropIndex);
            
					// Opcional: Emitir evento para que el motor (Bruno) sepa que el usuario reordenó algo
					// auto events = reinterpret_cast<Events*>(m_control->Handle()->Events.get());
					// if (events) events->CategoryReordered.emit();
            
					m_module.OnLayoutChanged();
				}
			}
			else if (releaseItemId != 0 && releaseItemId == m_module.m_pressedItemId)
			{
				if (m_module.m_model.IsCategory(releaseItemId))
				{
					if (args.ButtonState.RightButton)
					{
						ArgPropertyGridCategory arguments{ CategoryHandle(&m_module.m_model, releaseItemId) };
						m_module.m_events->CategoryRightClicked.Emit(arguments);
					}
					else
					{
						m_module.m_model.ToggleCategoryExpansion(releaseItemId);
						//m_module.m_model.SetSelectedIndex(FindItemIndexInVisibleList(hitCat));
						m_module.OnLayoutChanged();
						
						ArgPropertyGridCategory arguments{ CategoryHandle(&m_module.m_model, releaseItemId) };
						m_module.m_events->CategoryClicked.Emit(arguments);
					}
				}
				else
				{
					if (args.ButtonState.RightButton)
					{
						ArgPropertyGrid arguments{ PropertyHandle(&m_module.m_model, releaseItemId) };
						m_module.m_events->PropertyRightClicked.Emit(arguments);
					}
					else
					{
						// Clic en Propiedad (Seleccionar)
						//m_module.m_model.SetSelectedIndex(FindItemIndexInVisibleList(hitProp));
					}
				}
			}
			
			m_module.m_isDraggingCategory = false;
			m_module.m_isWaitingForDrag = false;
			m_module.m_pressedItemId = 0;
			m_module.m_layout.SetDropIndicator(false);
			
			GUI::MarkAsNeedUpdate(m_module.m_owner);
		}

		void Reactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
		{
			m_module.m_layout.m_scrollableView->HandleMouseWheel(args);
		}

		void Reactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
		{
			const auto& visibleItems = m_module.m_layout.GetVisibleItemsList();
			if (visibleItems.empty())
			{
				return;
			}
			
			auto currentId = m_module.m_model.GetSelectedItemId();
			auto it = std::find_if(visibleItems.begin(), visibleItems.end(), 
				[currentId](const auto& prop) { return prop.m_id == currentId; });
			
			size_t currentIndex = 0;
			if (it != visibleItems.end())
			{
				currentIndex = std::distance(visibleItems.begin(), it);
			}
			bool selectionChanged = false;
			bool layoutChanged = false;
			switch (args.Key)
			{
			case KeyboardKey::ArrowDown:
				if (currentIndex < visibleItems.size() - 1)
				{
					currentIndex++;
					selectionChanged = true;
				}
				break;
			case KeyboardKey::ArrowUp:
				if (currentIndex > 0)
				{
					currentIndex--;
					selectionChanged = true;
				}
				break;
			case KeyboardKey::ArrowRight:
				if (m_module.m_model.IsCategory(currentId) && !m_module.m_model.IsCategoryExpanded(currentId))
				{
					m_module.m_model.ToggleCategoryExpansion(currentId);
					layoutChanged = true;
				}
				break;
			case KeyboardKey::ArrowLeft:
				if (m_module.m_model.IsCategory(currentId) && m_module.m_model.IsCategoryExpanded(currentId))
				{
					m_module.m_model.ToggleCategoryExpansion(currentId);
					layoutChanged = true;
				}
				else
				{
					StringUtils::StringHash parentId = m_module.m_model.GetParentId(currentId);
					if (parentId != 0)
					{
						m_module.m_model.SetSelectedItemId(parentId);

						auto parentIt = std::find_if(visibleItems.begin(), visibleItems.end(), 
							[parentId](const auto& prop) { return prop.m_id == parentId; });
			
						if (parentIt != visibleItems.end())
						{
							currentIndex = std::distance(visibleItems.begin(), parentIt);
							selectionChanged = true;
						}
					}
				}
				break;
			}
			if (layoutChanged)
			{
				m_module.m_layout.CalculateLayout(m_module.m_model);
			}
			if (selectionChanged)
			{
				auto& newSelectedItem = visibleItems[currentIndex];
        
				m_module.m_model.SetSelectedItemId(newSelectedItem.m_id); 

				if (m_module.m_model.IsRootCategory(newSelectedItem.m_id)) 
				{
					m_module.m_layout.ScrollToItem(newSelectedItem.m_id);
				}
				else 
				{
					StringUtils::StringHash parentCatId = m_module.m_model.GetParentCategory(newSelectedItem.m_id); 
            
					m_module.m_layout.ScrollToItem(newSelectedItem.m_id);
				}
			}
			
			if (layoutChanged || selectionChanged)
			{
				GUI::MarkAsNeedUpdate(m_module.m_owner);
			}
		}

		void Reactor::Resize(Graphics& graphics, const ArgResize& args)
		{
			auto clientArea = m_control->GetClientArea();
			m_module.m_layout.m_scrollableView->SetViewRect(clientArea);
			m_module.m_layout.CalculateLayout(m_module.m_model);
		}

		void Reactor::DpiChanged(Graphics& graphics)
		{
			auto clientArea = m_control->GetClientArea();
			m_module.m_layout.m_scrollableView->SetViewRect(clientArea);
			m_module.m_layout.CalculateLayout(m_module.m_model);
		}

		void Reactor::DoOnInit()
		{
			m_module.m_owner = m_control->Handle();

			auto appearance = reinterpret_cast<Appearance*>(m_module.m_owner->Appearance.get());
			m_module.m_events = reinterpret_cast<Events*>(m_module.m_owner->Events.get());

			m_module.m_graphics = m_graphics;
			
			m_module.m_layout.Init(m_module.m_owner, appearance);
			
			m_module.m_model.Init(m_module.m_owner);
			m_module.m_model.OnPropertyModified = [this](StringUtils::StringHash propId) 
			{
				auto events = reinterpret_cast<Events*>(m_control->Handle()->Events.get());
				if (events)
				{
					PropertyHandle handle(&m_module.m_model, propId);
					ArgPropertyGrid arguments(handle);
					
					events->PropertyChanged.Emit(arguments); 
				}
				m_module.OnLayoutChanged();
			};
			m_module.m_model.OnPropertySelected = [this](StringUtils::StringHash propId) 
			{
				m_module.m_model.SetSelectedItemId(propId);
				m_module.m_layout.ScrollToItem(propId);
				m_module.OnLayoutChanged();
				
				auto events = reinterpret_cast<Events*>(m_control->Handle()->Events.get());
				if (events) 
				{
					PropertyHandle handle(&m_module.m_model, propId);
					ArgPropertyGrid arguments(handle);
					
					events->SelectionChanged.Emit(arguments); 
				}
			};
			m_module.m_model.OnVisualsChanged = [this]() 
			{
				m_module.OnLayoutChanged();
			};
		}
	}

	PropertyGrid::PropertyGrid(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "PropertyGrid";
#endif
	}

	PropertyGrid::CategoryItem PropertyGrid::Append(std::string_view categoryName)
	{
		auto& module = GetReactor().GetModule();
		auto& newCategory = module.m_model.AppendRootCategory(categoryName);
		module.m_layout.CalculateLayout(module.m_model);
		
		return {&module.m_model, newCategory.m_id};
	}

	void PropertyGrid::Clear()
	{
		auto& module = GetReactor().GetModule();
		module.m_model.Clear();
		module.m_layout.SetHoverItemId(0);
		module.m_layout.CalculateLayout(module.m_model);
	}

	PropertyGrid::CategoryItem PropertyGrid::Insert(CategoryItem existingCategory, const std::string& categoryName)
	{
		return { nullptr, 0 };
	}

	void PropertyGrid::Erase(CategoryItem categoryItem)
	{
		auto& module = GetReactor().GetModule();
		if (module.m_model.RemoveCategory(categoryItem.GetId()))
		{
			module.ClearReferences(categoryItem.GetId());
			module.m_layout.CalculateLayout(module.m_model);
			
			GUI::UpdateWindow(module.m_owner);
		}
	}

	void PropertyGrid::Erase(PropertyItem propertyItem)
	{
		auto& module = GetReactor().GetModule();
        
		if (module.m_model.RemoveProperty(propertyItem.GetId()))
		{
			module.ClearReferences(propertyItem.GetId());
			module.m_layout.CalculateLayout(module.m_model);
			
			GUI::UpdateWindow(module.m_owner);
		}
	}

	void PropertyGrid::RefreshAll()
	{
		auto& module = GetReactor().GetModule();
		module.m_layout.RefreshVisibleOnly(module.m_model);
		module.OnLayoutChanged();
	}

	void PropertyGrid::ShowCategoryIcons(bool visible)
	{
		auto& module = GetReactor().GetModule();
		module.m_model.ShowCategoryIcons(visible);
		module.OnLayoutChanged();
	}

	/*PropertyGrid::CategoryItem PropertyGrid::Find(std::string_view categoryName)
	{
		auto& module = GetReactor().GetModule();
		auto hash = StringUtils::HashString(categoryName);
		module.m_model.FindCategoryById(hash);
		return GetReactor().GetModule().m_model.Find(categoryName);
	}*/
}
