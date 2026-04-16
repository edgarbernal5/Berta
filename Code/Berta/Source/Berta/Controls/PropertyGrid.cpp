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
		void Reactor::DoOnInit()
		{
			m_module.m_owner = m_control->Handle();

			m_module.m_appearance = reinterpret_cast<Appearance*>(m_module.m_owner->Appearance.get());
			m_module.m_events = reinterpret_cast<Events*>(m_module.m_owner->Events.get());

			m_module.m_graphics = m_graphics;
			
			m_module.m_layout.Init(m_module.m_owner, *m_module.m_appearance);
			
			m_module.m_model.Init(m_module.m_owner);
			m_module.m_model.OnPropertyModified = [this](StringUtils::StringHash catId, StringUtils::StringHash propId) 
			{
				auto events = reinterpret_cast<Events*>(m_control->Handle()->Events.get());
				if (events) 
				{
					PropertyItem handle(&m_module.m_model, catId, propId);
					ArgPropertyGrid arguments(handle);
					
					events->PropertyChanged.Emit(arguments); 
				}
				OnLayoutChanged();
			};
			m_module.m_model.OnPropertySelected = [this](StringUtils::StringHash catId, StringUtils::StringHash propId) 
			{
				auto events = reinterpret_cast<Events*>(m_control->Handle()->Events.get());
				if (events) 
				{
					PropertyItem handle(&m_module.m_model, catId, propId);
					ArgPropertyGrid arguments(handle);
					
					events->SelectionChanged.Emit(arguments); 
				}
			};
			m_module.m_model.OnVisualsChanged = [this]() 
			{
				OnLayoutChanged();
			};
		}

		void Reactor::OnLayoutChanged()
		{
			m_module.m_layout.CalculateLayout(m_module.m_model);

			if (m_control && m_control->Handle()) 
			{
				Window* window = m_control->Handle();
				GUI::UpdateWindow(window);
			}
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
			if (m_module.m_mouseInteraction.m_hoveredCategory)
			{
				m_module.m_mouseInteraction.m_hoveredCategory = nullptr;
				GUI::UpdateWindow(m_module.m_owner);
			}
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			if (!args.ButtonState.LeftButton)
				return;

			Point clickPos = { args.Position.X, args.Position.Y + m_module.m_layout.m_scrollableView->GetScrollOffset().Y };
			m_module.ProcessClickRecursive(m_module.m_model.GetRootCategories(), clickPos, 0);
		}

		void Reactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
		{
			m_module.m_layout.m_scrollableView->HandleMouseWheel(args);
		}

		void Reactor::Resize(Graphics& graphics, const ArgResize& args)
		{
			m_module.m_layout.m_scrollableView->SetViewRect(Rectangle(0, 0, args.NewSize.Width, args.NewSize.Height));
			m_module.m_layout.CalculateLayout(m_module.m_model);
		}

		void Module::Update()
		{
			if (!m_owner->Flags.AutoDraw)
				return;

			GUI::UpdateWindow(m_owner);
		}

		void Module::Draw()
		{
			/*auto& graphics = *m_graphics;
			auto clientRect = m_owner->ClientSize.ToRectangle();
			graphics.FillRectangle(clientRect, m_owner->Appearance->BoxBackground);

			auto one = m_owner->ToScale(1);
			for (auto& category : m_categories)
			{
				Rectangle categoryRect = category.m_area;
				categoryRect.X += m_viewport.m_backgroundRect.X - m_scrollOffset.X;
				categoryRect.Y += m_viewport.m_backgroundRect.Y - m_scrollOffset.Y;

				bool isCategoryHovered = m_mouseInteraction.m_hoveredCategory == &(category);
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
					category.m_isExpanded ? Graphics::ArrowDirection::Downwards : Graphics::ArrowDirection::Right,
					m_appearance->Foreground2nd,
					true,
					category.m_isExpanded ? m_appearance->Foreground2nd : m_appearance->BoxBackground
				);

				Point textOffset = { expanderRect.X + static_cast<int>(expanderRect.Width) + m_viewport.m_categoryTextOffset,static_cast<int>(m_viewport.m_categoryItemHeight) - static_cast<int>(graphics.GetTextExtent().Height) };
				textOffset.Y >>= 1;

				graphics.DrawString({ textOffset.X,categoryRect.Y + textOffset.Y }, category.m_name, m_appearance->Foreground);

				Point scrollOffset{ categoryRect.X,categoryRect.Y };
				scrollOffset.Y += categoryRect.Height;

				for (size_t i = 0; i < category.m_properties.size(); i++)
				{
					auto field = it->m_properties[i].get();
					bool fieldVisible = category.m_isExpanded;
					auto fieldContainer = it->m_fieldContainers[i].get();
					auto fieldSize = field->GetSize();

					bool isSelected = field == m_mouseInteraction.m_lastPropertySelected;
					if (category.m_isExpanded)
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
						fieldContainerArea.X += static_cast<int>(fieldArea.Width >> 1);
						fieldContainerArea.Width -= fieldArea.Width >> 1;

						GUI::MoveWindow(*fieldContainer, fieldContainerArea);

						field->Draw(graphics, fieldArea, m_viewport.m_backgroundRect.Width >> 1, isSelected ? m_appearance->Red : m_appearance->Foreground);
					}
					
					if (category.m_isExpanded)
					{
						scrollOffset.Y += fieldSize;
					}
					GUI::ShowWindow(*fieldContainer, fieldVisible);
				}
			}

			graphics.DrawRectangle(clientRect, m_owner->Appearance->BoxBorderColor);*/
		}

		int Module::ProcessClickRecursive(std::vector<CategoryType>& list, Point pos, int currentY)
		{
			auto width = m_layout.m_scrollableView->GetClientArea().Width;
    
			for (auto& cat : list) {
				auto indent = cat.m_depth * 20;
				Rectangle catRect{ (int)indent, currentY, width - indent, m_appearance->CategoryHeight };

				if (catRect.Contains(pos)) {
					cat.m_isExpanded = !cat.m_isExpanded;
					//OnLayoutChanged();
					return -1; // Detener búsqueda
				}
				currentY += m_appearance->CategoryHeight;

				if (cat.m_isExpanded)
				{
					// Comprobar propiedades...
					for (auto& prop : cat.m_properties) {
						// (Lógica de clic en propiedad similar a la anterior)
						currentY += (int)prop.field->GetHeight();
					}
            
					// Comprobar subcategorías recursivamente
					currentY = ProcessClickRecursive(cat.m_subCategories, pos, currentY);
					if (currentY == -1)
					{
						return -1;
					}
				}
			}
			return currentY;
		}

		/*PropertyItem CategoryItem::Append(PropertyGridFieldBasePtr propGridFieldPtr)
		{
			m_category->m_properties.emplace_back(std::move(propGridFieldPtr));
			auto newField = m_category->m_properties.back().get();

			auto containerPtr = std::make_unique<FieldControlContainer>(m_module->m_owner);
			newField->SetModule(m_module);
			newField->Init(containerPtr->Handle());

			m_category->m_fieldContainers.emplace_back(std::move(containerPtr));

			m_module->CalculateViewport(m_module->m_viewport);
			m_module->BuildItems();
			m_module->UpdateScrollBar();
			
			return { m_module, newField };
		}*/

		/*CategoryItem::operator bool() const
		{
			return m_module != nullptr && m_category != nullptr;
		}*/

		CategoryItem CategoryItem::AppendCategory(std::string_view name)
		{
			// 1. Llamamos al modelo (Mundo Interno)
			CategoryType& rawCategory = m_model->AppendRootCategory(name);

			// 2. Avisamos a la UI que recalcule el scroll
			//m_model->OnLayoutChanged();

			// 3. Envolvemos el resultado en el proxy seguro (Mundo Externo)
			return {m_model, rawCategory.m_id};
		}

		CategoryItem CategoryItem::AppendSubCategory(std::string_view name)
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

		void PropertyGridLayout::Init(Window* owner, const Appearance& config)
		{
			if (m_isInitialized)
			{
				return;
			}
			m_owner = owner;
			m_config = config;
			m_internalScrollManager = std::make_unique<ScrollableView>(m_owner);
			m_scrollableView = m_internalScrollManager.get();

			m_scrollableView->SetScrollStep(m_owner->ToScale(m_config.CategoryHeight), 0);
			m_scrollableView->SetOnScrollChange([this]()
			{
				if (m_owner)
				{
					GUI::MarkAsNeedUpdate(m_owner);
				}
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
			m_scrollableView->SetContentSize({ m_scrollableView->GetClientArea().Width, totalHeight });
		}

		void PropertyGridLayout::Draw(Graphics& graphics, const PropertyGridModel& model, Appearance* appearance)
		{
			Point offset = m_scrollableView->GetScrollOffset();
			int currentY = -offset.Y;

			for (auto& cat : model.GetRootCategories())
			{
				currentY = DrawRecursive(graphics, cat, currentY);
			}
		}

		uint32_t PropertyGridLayout::CalculateCategoryHeight(const CategoryType& cat)
		{
			uint32_t height = m_config.CategoryHeight;
			if (cat.m_isExpanded)
			{
				for (const auto& prop : cat.m_properties)
				{
					height += prop.field->GetHeight();
				}
				
				for (const auto& sub : cat.m_subCategories) {
					height += CalculateCategoryHeight(sub);
				}
			}
			return height;
		}

		int PropertyGridLayout::DrawRecursive(Graphics& graphics, const CategoryType& cat, int y)
		{
			auto width = m_scrollableView->GetClientArea().Width;
			int indent = (int)cat.m_depth * m_owner->ToScale(20);
			auto categoryHeight = m_owner->ToScale(m_config.CategoryHeight);
			
			Rectangle area{ indent, y, width - indent, categoryHeight };
			if (IsVisible(area))
			{
				DrawCategoryHeader(graphics, area, cat, m_config);
			}
			y += (int)categoryHeight;

			if (cat.m_isExpanded)
			{
				for (const auto& prop : cat.m_properties)
				{
					Rectangle propArea{ indent + 10, y, width - (indent + 10), prop.field->GetHeight() };
					if (IsVisible(propArea))
					{
						int labelWidth = (width - indent) / 2;
						prop.field->Draw(graphics, propArea, labelWidth, m_config);
					}
					y += (int)propArea.Height;
				}
				
				for (const auto& sub : cat.m_subCategories)
				{
					y = DrawRecursive(graphics, sub, y);
				}
			}
			return y;
		}

		void PropertyGridLayout::DrawCategoryHeader(Graphics& graphics, const Rectangle& area, const CategoryType& cat, const Appearance& config)
		{
			Color bgColor = m_config.Background;
			graphics.FillRectangle(area, bgColor);

			int offset = m_owner->ToScale(4);
			int expanderSize = m_owner->ToScale(14);
			int centerY = area.Y + ((int)area.Height - expanderSize) / 2;
			Rectangle expanderArea{ area.X + offset, centerY, (uint32_t)expanderSize, (uint32_t)expanderSize };

			int arrowWidth = m_owner->ToScale(4);
			int arrowLength = m_owner->ToScale(2);

			graphics.DrawArrow(expanderArea,
				arrowLength,
				arrowWidth,
				cat.m_isExpanded ? Graphics::ArrowDirection::Downwards : Graphics::ArrowDirection::Right,
				m_config.Foreground2nd,
				true,
				cat.m_isExpanded ? m_config.Foreground2nd : m_config.BoxBackground
			);
			
			// 3. DIBUJAR EL NOMBRE DE LA CATEGORÍA
			// Dejamos un margen después del icono para que respire
			int textX = expanderArea.X + (int)expanderArea.Width + offset;
			Point textPos = { textX, area.Y + ((int)area.Height - (int)graphics.GetTextExtent().Height) / 2 }; // Centrado verticalmente
    
			// Opcional: Podrías dibujar el texto en Negrita si tu Graphics lo soporta
			graphics.DrawString(textPos, cat.m_name, config.Foreground);

			// 4. LÍNEA SEPARADORA INFERIOR (El toque sutil AAA)
			// Dibuja una línea de 1 píxel debajo de la categoría para separarla visualmente del contenido
			Color separatorColor = m_config.BoxBorderColor;
			graphics.DrawLine({ area.X, area.Y + (int)area.Height - 1 }, { area.X + (int)area.Width, area.Y + (int)area.Height - 1 }, separatorColor);
		}

		bool PropertyGridLayout::IsVisible(const Rectangle& area) const
		{
			// Protección por si el control aún no se inicializó
			if (!m_scrollableView)
			{
				return false;
			}
			
			// Obtenemos el rectángulo real que el usuario está viendo en pantalla
			Rectangle visibleRect = m_scrollableView->GetVisibleRect();
			return visibleRect.Intersects(area); 
		}

		PropertyItem::operator bool() const
		{
			return m_model;
		}

		std::string_view PropertyItem::GetLabel() const
		{
			return m_model ? m_model->GetPropertyLabel(m_catId, m_propId) : "";
		}

		PropertyItem& PropertyItem::SetLabel(std::string_view newLabel)
		{
			if (m_model)
			{
				m_model->SetPropertyLabel(m_catId, m_propId, newLabel);
			}
			return *this;
		}

		bool PropertyItem::IsEnabled() const
		{
			if (m_model)
			{
				return m_model->GetPropertyEnabled(m_catId, m_propId);
			}
			return false;
		}

		PropertyItem& PropertyItem::SetEnabled(bool enabled)
		{
			if (m_model)
			{
				m_model->SetPropertyEnabled(m_catId, m_propId, enabled);
			}
			return *this;
		}

		void PropertyGridFieldBase::Init(Window* parent)
		{
			m_parent = parent;
			Create(parent);
			SetEnabled(IsEnabled());
		}

		std::string_view PropertyGridFieldBase::GetLabel() const
		{
			return m_label;
		}

		void PropertyGridFieldBase::SetLabel(std::string_view newLabel)
		{
			if (m_label == newLabel)
				return;

			m_label = newLabel;
		}

		bool PropertyGridFieldBase::IsEnabled() const
		{
			return m_enabled;
		}

		void PropertyGridFieldBase::SetEnabled(bool enabled)
		{
			m_enabled = enabled;
		}

		void PropertyGridFieldBase::Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const LayoutConfig& config)
		{
			Rectangle labelArea = area;
			labelArea.Width = labelWidth;

			DrawLabel(graphics, labelArea, config.Foreground);
		}

		void PropertyGridFieldBase::ScrollToView()
		{
		}

		void PropertyGridFieldBase::Update()
		{
		}

		void PropertyGridFieldBase::DrawLabel(Graphics& graphics, const Rectangle& area, const Color& textColor)
		{
			auto& textExtents = graphics.GetTextExtent();
			Point position = area;
			position.Y += static_cast<int>((area.Height - textExtents.Height) >> 1);

			graphics.DrawString(position, m_label, textColor);
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
				OnSelected();
		}

		FieldControlContainer::FieldControlContainer(Window* parent, const Rectangle& rect) :
			Panel(parent, rect)
		{
		}

		void PropertyGridModel::Init(Window* ownerWindow)
		{
			if (m_ownerWindow == ownerWindow)
			{
				return;
			}
        
			m_ownerWindow = ownerWindow;

			// Inicializamos todas las propiedades que se añadieron ANTES de que
			// el PropertyGrid tuviera una ventana (Escenario 1).
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
			
			if (m_ownerWindow) {
				field->Init(m_ownerWindow); 
			}
			// 2. Buscamos la categoría (ya sea raíz o subcategoría)
			if (CategoryType* cat = FindCategoryById(categoryId))
			{
				// 3. Movemos el ownership (propiedad) del puntero único al vector
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

		void PropertyGridModel::SetPropertyEnabled(StringUtils::StringHash catId, StringUtils::StringHash propId,
			bool enabled)
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
				// Asumiendo que tu PropertyGridFieldBase tiene el método Create(Window*)
				prop.field->Init(m_ownerWindow); 
			}
			
			for (auto& subCat : cat.m_subCategories)
			{
				InitCategoryRecursive(subCat);
			}
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
			
		module.m_mouseInteraction.m_lastPropertySelected = nullptr;
	}

	PropertyGrid::CategoryItem PropertyGrid::Insert(CategoryItem existingCategory, const std::string& categoryName)
	{
		return { nullptr, 0 };
	}

	/*PropertyGrid::CategoryItem PropertyGrid::Find(std::string_view categoryName)
	{
		auto& module = GetReactor().GetModule();
		auto hash = StringUtils::HashString(categoryName);
		module.m_model.FindCategoryById(hash);
		return GetReactor().GetModule().m_model.Find(categoryName);
	}*/
}
