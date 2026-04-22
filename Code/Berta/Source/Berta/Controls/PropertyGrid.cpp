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
			StringUtils::StringHash id = StringUtils::HashString(categoryName);
            
			if (auto* existing = FindCategoryById(id))
			{
				return *existing;
			}
			return m_rootCategories.emplace_back(categoryName, 0);
		}

		CategoryType* PropertyGridModel::AppendSubCategory(StringUtils::StringHash parentId, std::string_view name)
		{
			CategoryType* parent = FindCategoryById(parentId);
			if (!parent)
			{
				return nullptr;
			}
			
			StringUtils::StringHash childId = StringUtils::HashString(name);
        
			// Evitar duplicados en el mismo nivel
			auto it = std::find_if(parent->m_subCategories.begin(), parent->m_subCategories.end(),
			                       [childId](const CategoryType& c) { return c.m_id == childId; });

			if (it != parent->m_subCategories.end())
			{
				return &(*it);
			}
			
			return &parent->m_subCategories.emplace_back(name, parent->m_depth + 1u);
		}

		void PropertyGridModel::AppendPropertyToCategory(StringUtils::StringHash categoryId, std::unique_ptr<PropertyGridFieldBase> field)
		{
			if (!field)
			{
				return;
			}
			
			StringUtils::StringHash permanentId = StringUtils::HashString(field->GetLabel());
				
			field->OnValueChanged = [this, categoryId, permanentId]()
			{
				if (OnPropertyModified)
				{
					OnPropertyModified(categoryId, permanentId);
				}
			};
			field->OnSelected = [this, categoryId, permanentId]()
			{
				if (OnPropertySelected)
				{
					OnPropertySelected(categoryId, permanentId);
				}
			};
			
			if (m_ownerWindow)
			{
				field->Init(m_ownerWindow); 
			}
			if (CategoryType* cat = FindCategoryById(categoryId))
			{
				cat->m_properties.push_back({ permanentId, std::move(field) });
			}
		}

		CategoryType* PropertyGridModel::FindCategoryById(StringUtils::StringHash id)
		{
			return FindRecursive(id, m_rootCategories);
		}

		PropertyFieldData* PropertyGridModel::FindPropertyById(StringUtils::StringHash catId, StringUtils::StringHash propId)
		{
			auto* cat = FindCategoryById(catId);
			if (!cat)
			{
				return nullptr;
			}
			for (auto& prop : cat->m_properties)
			{
				if (prop.m_id == propId)
				{
					return &prop;
				}
			}
			return nullptr;
		}

		void PropertyGridModel::Clear()
		{
			m_rootCategories.clear();
		}

		bool PropertyGridModel::GetPropertyEnabled(StringUtils::StringHash catId, StringUtils::StringHash propId)
		{
			if (PropertyFieldData* prop = FindPropertyById(catId, propId))
			{
				return prop->field->IsEnabled();
			}
			return false;
		}

		void PropertyGridModel::SetPropertyEnabled(StringUtils::StringHash catId, StringUtils::StringHash propId, bool enabled)
		{
			if (PropertyFieldData* prop = FindPropertyById(catId, propId))
			{
				prop->field->SetEnabled(enabled);
			}
		}

		std::string_view PropertyGridModel::GetPropertyLabel(StringUtils::StringHash catId, StringUtils::StringHash propId)
		{
			if (PropertyFieldData* prop = FindPropertyById(catId, propId))
			{
				return prop->field->GetLabel();
			}
			return "";
		}

		void PropertyGridModel::SetPropertyLabel(StringUtils::StringHash catId, StringUtils::StringHash propId, std::string_view newLabel)
		{
			if (PropertyFieldData* prop = FindPropertyById(catId, propId))
			{
				prop->field->SetLabel(newLabel);
				if (OnVisualsChanged)
				{
					OnVisualsChanged(); 
				}
			}
		}

		std::string PropertyGridModel::GetPropertyValueAsString(StringUtils::StringHash catId, StringUtils::StringHash propId)
		{
			if (PropertyFieldData* prop = FindPropertyById(catId, propId))
			{
				return prop->field->GetValueAsString();
			}
        
			return {};
		}

		void PropertyGridModel::SetSelectedProperty(StringUtils::StringHash catId, StringUtils::StringHash propId)
		{
			m_selectedCatId = catId; 
			m_selectedPropId = propId;
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

		PropertyHandle::operator bool() const
		{
			return m_model;
		}

		std::string_view PropertyHandle::GetLabel() const
		{
			return m_model ? m_model->GetPropertyLabel(m_catId, m_propId) : "";
		}

		PropertyHandle& PropertyHandle::SetLabel(std::string_view newLabel)
		{
			if (m_model)
			{
				m_model->SetPropertyLabel(m_catId, m_propId, newLabel);
			}
			return *this;
		}

		std::string PropertyHandle::GetValueAsString() const
		{
			return m_model ? m_model->GetPropertyValueAsString(m_catId, m_propId) : "";
		}

		bool PropertyHandle::IsEnabled() const
		{
			if (m_model)
			{
				return m_model->GetPropertyEnabled(m_catId, m_propId);
			}
			return false;
		}

		PropertyHandle& PropertyHandle::SetEnabled(bool enabled)
		{
			if (m_model)
			{
				m_model->SetPropertyEnabled(m_catId, m_propId, enabled);
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

		PropertyHandle CategoryHandle::AppendProperty(std::unique_ptr<PropertyGridFieldBase> field)
		{
			if (m_model && field)
			{
				uint32_t propId = StringUtils::HashString(field->GetLabel());
				m_model->AppendPropertyToCategory(m_id, std::move(field));
				return {m_model, m_id, propId};
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
			uint32_t totalHeight = 0;
			for (auto& cat : model.GetRootCategories())
			{
				totalHeight += CalculateCategoryHeight(cat);
			}
			m_scrollableView->SetContentSize({ 0, totalHeight });
		}

		void PropertyGridLayout::Draw(Graphics& graphics, const PropertyGridModel& model, Appearance* appearance)
		{
			Point offset = m_scrollableView->GetScrollOffset();
			auto clientArea = m_scrollableView->GetClientArea();
			int currentX = -offset.X + clientArea.X;
			int currentY = -offset.Y + clientArea.Y;

			for (auto& cat : model.GetRootCategories())
			{
				currentY = DrawRecursive(graphics, model, cat, currentX, currentY);
			}
		}

		void PropertyGridLayout::ScrollToItem(const PropertyGridModel& model, StringUtils::StringHash catId, StringUtils::StringHash propId)
		{
			if (!m_scrollableView)
			{
				return;
			}
			int currentY = 0;
			Rectangle itemRect;

			if (FindItemRectRecursive(model.GetRootCategories(), catId, propId, currentY, itemRect))
			{
				Rectangle viewport = m_scrollableView->GetVisibleRect();

				int newScrollY = viewport.Y;

				if (itemRect.Y < viewport.Y) 
				{
					newScrollY = itemRect.Y;
				}
				else if ((itemRect.Y + itemRect.Height) > (viewport.Y + viewport.Height)) 
				{
					newScrollY = (itemRect.Y + itemRect.Height) - viewport.Height;
				}
				
				if (newScrollY != viewport.Y)
				{
					m_scrollableView->SetScrollToY(newScrollY); 
				}
			}
		}

		uint32_t PropertyGridLayout::CalculateCategoryHeight(const CategoryType& cat)
		{
			uint32_t height = m_owner->ToScale(m_config->CategoryHeight);
			if (cat.m_isExpanded)
			{
				for (const auto& prop : cat.m_properties)
				{
					height += prop.field->GetHeight();
				}
				
				for (const auto& sub : cat.m_subCategories)
				{
					height += CalculateCategoryHeight(sub);
				}
			}
			return height;
		}

		int PropertyGridLayout::DrawRecursive(Graphics& graphics, const PropertyGridModel& model, const CategoryType& cat, int x, int y)
		{
			auto width = m_scrollableView->GetClientArea().Width;
			int indent = (int)cat.m_depth * m_owner->ToScale(20);
			auto categoryHeight = m_owner->ToScale(m_config->CategoryHeight);
			
			Rectangle catArea{ x + indent, y, width - indent, categoryHeight };
			if (IsVisible(catArea))
			{
				DrawCategoryHeader(graphics, model, catArea, cat, m_config);
			}
			y += (int)categoryHeight;

			if (cat.m_isExpanded)
			{
				for (const auto& prop : cat.m_properties)
				{
					Rectangle fullPropArea{ x + indent + 10, y, width - (indent + 10), prop.field->GetHeight() };
					
					if (IsVisible(fullPropArea))
					{
						Rectangle propArea = fullPropArea;
						
						bool isSelected = (cat.m_id == model.GetSelectedCatId() && prop.m_id == model.GetSelectedPropId());
						bool isHovered = (cat.m_id == m_hoveredCatId && prop.m_id == m_hoveredPropId);
						
						if (isSelected)
						{
							graphics.FillRectangle(fullPropArea, m_config->SelectedBackgroundColor);
						}
						else if (isHovered)
						{
							graphics.FillRectangle(fullPropArea, m_config->HoverBackgroundColor);
						}
						
						if (prop.field->IsShowingLabel())
						{
							int labelWidth = ((int)width - indent) / 2;
							
							Rectangle labelArea = propArea;
							labelArea.Width = labelWidth;
							
							Color textColor = isSelected ? m_config->SelectedTextColor : m_config->Foreground;
							graphics.DrawString({ labelArea.X + 5, labelArea.Y + 4 }, prop.field->GetLabel(), textColor);
							
							propArea.X += labelWidth;
							propArea.Width -= labelWidth;
						}
						
						propArea.X += 2;
						propArea.Y += 2;
						propArea.Width -= 4;
						propArea.Height -= 4;
						
						prop.field->Draw(graphics, propArea, *m_config);
						prop.field->SetVisibility(true);
					}
					else 
					{
						prop.field->SetVisibility(false);
					}
					y += (int)fullPropArea.Height;
				}
				
				for (const auto& sub : cat.m_subCategories)
				{
					y = DrawRecursive(graphics, model, sub, x, y);
				}
			}
			else 
			{
				HideCategoryRecursive(cat);
			}
			return y;
		}

		void PropertyGridLayout::DrawCategoryHeader(Graphics& graphics, const PropertyGridModel& model, const Rectangle& area, const CategoryType& cat, Appearance* config)
		{
			Color bgColor = config->Background;
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

		bool PropertyGridLayout::IsVisible(const Rectangle& area) const
		{
			if (!m_scrollableView)
			{
				return false;
			}
			
			Rectangle clientArea = m_scrollableView->GetClientArea();
			return clientArea.Intersects(area); 
		}

		void PropertyGridLayout::HideCategoryRecursive(const CategoryType& cat)
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
				HideCategoryRecursive(subCat);
			}
		}

		bool PropertyGridLayout::FindItemRectRecursive(const std::vector<CategoryType>& list, StringUtils::StringHash targetCat, StringUtils::StringHash targetProp, int& currentY, Rectangle& outRect) const
		{
			auto width = m_scrollableView->GetClientArea().Width;

			for (const auto& cat : list)
			{
				auto indent = cat.m_depth * m_owner->ToScale(20u);
				Rectangle catRect{ (int)indent, currentY, width - indent, m_owner->ToScale(m_config->CategoryHeight) };
				
				if (cat.m_id == targetCat && targetProp == 0)
				{
					outRect = catRect;
					return true; 
				}

				currentY += static_cast<int>(catRect.Height);

				if (cat.m_isExpanded)
				{
					for (const auto& prop : cat.m_properties)
					{
						auto propHeight = prop.field->GetHeight();
						Rectangle propRect{ (int)indent + 10, currentY, width - (indent + 10), propHeight };

						if (cat.m_id == targetCat && prop.m_id == targetProp)
						{
							outRect = propRect;
							return true;
						}
						currentY += static_cast<int>(propHeight);
					}

					if (FindItemRectRecursive(cat.m_subCategories, targetCat, targetProp, currentY, outRect))
					{
						return true;
					}
				}
			}
			return false;
		}

		void PropertyGridLayout::RefreshVisibleOnly(const PropertyGridModel& model)
		{
			if (!m_scrollableView) return;

			Rectangle viewport = m_scrollableView->GetVisibleRect();
			int startY = 0;
        
			RefreshVisibleRecursive(model.GetRootCategories(), startY, viewport);
		}

		bool PropertyGridLayout::RefreshVisibleRecursive(const std::vector<CategoryType>& list, int& currentY, const Rectangle& viewport)
		{
			int viewportBottom = viewport.Y + (int)viewport.Height;

			for (const auto& cat : list)
			{
				auto indent = cat.m_depth * m_owner->ToScale(20u);
				Rectangle catRect{ (int)indent, currentY, viewport.Width, m_owner->ToScale(m_config->CategoryHeight) };

				currentY += (int)catRect.Height;

				if (currentY > viewportBottom)
				{
					return false;
				}
				
				if (cat.m_isExpanded)
				{
					for (const auto& prop : cat.m_properties)
					{
						auto propHeight = prop.field->GetHeight();
						Rectangle propRect{ (int)indent + 10, currentY, viewport.Width, propHeight };
						
						if (propRect.Intersects(viewport)) 
						{
							prop.field->Refresh();
						}

						currentY += (int)propHeight;

						if (currentY > viewportBottom) 
						{
							return false;
						}
					}
					
					if (!RefreshVisibleRecursive(cat.m_subCategories, currentY, viewport))
					{
						return false;
					}
				}
			}
        
			return true;
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

		void Module::OnLayoutChanged()
		{
			m_layout.CalculateLayout(m_model);

			GUI::UpdateWindow(m_owner);
		}

		int Module::HitTestRecursive(const std::vector<CategoryType>& list, Point pos, int currentY, StringUtils::StringHash& outCatId, StringUtils::StringHash& outPropId)
		{
			auto width = m_layout.m_scrollableView->GetClientArea().Width;

			for (const auto& cat : list)
			{
				auto indent = cat.m_depth * m_owner->ToScale(20u) ;
				
				Rectangle catRect{ (int)indent, currentY, width - indent, m_owner->ToScale(m_layout.GetConfig()->CategoryHeight) };

				if (catRect.Contains(pos))
				{
					outCatId = cat.m_id;
					outPropId = 0;
					return -1;
				}

				currentY += static_cast<int>(catRect.Height);
				
				if (cat.m_isExpanded)
				{
					for (const auto& prop : cat.m_properties)
					{
						auto propHeight = prop.field->GetHeight();
						Rectangle propRect{ (int)indent + 10, currentY, width - (indent + 10), propHeight };

						if (propRect.Contains(pos))
						{
							outCatId = cat.m_id;
							outPropId = prop.m_id;
							return -1;
						}
						currentY += static_cast<int>(propHeight);
					}

					currentY = HitTestRecursive(cat.m_subCategories, pos, currentY, outCatId, outPropId);
					
					if (currentY == -1)
					{
						return -1;
					}
				}
			}
			
			return currentY;
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
			if (m_module.m_lastHoveredCat != 0 || m_module.m_lastHoveredProp != 0)
			{
				m_module.m_lastHoveredCat = 0;
				m_module.m_lastHoveredProp = 0;
				m_module.m_layout.SetHoverState(0, 0);
				m_module.OnLayoutChanged();
			}
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			auto offset = m_module.m_layout.m_scrollableView->GetScrollOffset();
			Point clickPos = { args.Position.X + offset.X, args.Position.Y + offset.Y };
			
			m_module.HitTestRecursive(m_module.m_model.GetRootCategories(), clickPos, 0, m_module.m_pressedCatId, m_module.m_pressedPropId);
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			StringUtils::StringHash hitCatId = 0;
			StringUtils::StringHash hitPropId = 0;

			m_module.HitTestRecursive(m_module.m_model.GetRootCategories(), args.Position, -m_module.m_layout.m_scrollableView->GetScrollOffset().Y, hitCatId, hitPropId);

			if (hitCatId != m_module.m_lastHoveredCat || hitPropId != m_module.m_lastHoveredProp)
			{
				m_module.m_lastHoveredCat = hitCatId;
				m_module.m_lastHoveredProp = hitPropId;

				m_module.m_layout.SetHoverState(hitCatId, hitPropId);
				m_module.OnLayoutChanged(); 
			}
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			auto offset = m_module.m_layout.m_scrollableView->GetScrollOffset();
			Point clickPos = { args.Position.X + offset.X, args.Position.Y + offset.Y };
			
			StringUtils::StringHash hitCat = 0, hitProp = 0;
			m_module.HitTestRecursive(m_module.m_model.GetRootCategories(), clickPos, 0, hitCat, hitProp);
			if (hitCat == m_module.m_pressedCatId && hitProp == m_module.m_pressedPropId)
			{
				if (hitCat != 0)
				{
					if (hitProp != 0)
					{
						if (args.ButtonState.RightButton)
						{
							ArgPropertyGrid arguments{ PropertyHandle(&m_module.m_model, hitCat, hitProp) };
							m_module.m_events->PropertyRightClicked.Emit(arguments);
						}
					}
					else
					{
						if (args.ButtonState.RightButton)
						{
							ArgPropertyGridCategory arguments{ CategoryHandle(&m_module.m_model, hitCat) };
							m_module.m_events->CategoryRightClicked.Emit(arguments);
						}
						else if (args.ButtonState.LeftButton)
						{
							m_module.m_model.ToggleCategoryExpansion(hitCat);
							m_module.OnLayoutChanged();
							
							ArgPropertyGridCategory arguments{ CategoryHandle(&m_module.m_model, hitCat) };
							m_module.m_events->CategoryClicked.Emit(arguments);
						}
					}
				}
			}
			
			m_module.m_pressedCatId = 0;
			m_module.m_pressedPropId = 0;
			
			GUI::MarkAsNeedUpdate(m_module.m_owner);
		}

		void Reactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
		{
			m_module.m_layout.m_scrollableView->HandleMouseWheel(args);
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
			m_module.m_model.OnPropertyModified = [this](StringUtils::StringHash catId, StringUtils::StringHash propId) 
			{
				auto events = reinterpret_cast<Events*>(m_control->Handle()->Events.get());
				if (events) 
				{
					PropertyHandle handle(&m_module.m_model, catId, propId);
					ArgPropertyGrid arguments(handle);
					
					events->PropertyChanged.Emit(arguments); 
				}
				m_module.OnLayoutChanged();
			};
			m_module.m_model.OnPropertySelected = [this](StringUtils::StringHash catId, StringUtils::StringHash propId) 
			{
				m_module.m_model.SetSelectedProperty(catId, propId);
				m_module.m_layout.ScrollToItem(m_module.m_model, catId, propId);
				m_module.OnLayoutChanged();
				
				auto events = reinterpret_cast<Events*>(m_control->Handle()->Events.get());
				if (events) 
				{
					PropertyHandle handle(&m_module.m_model, catId, propId);
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
		module.m_layout.CalculateLayout(module.m_model);
	}

	PropertyGrid::CategoryItem PropertyGrid::Insert(CategoryItem existingCategory, const std::string& categoryName)
	{
		return { nullptr, 0 };
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
