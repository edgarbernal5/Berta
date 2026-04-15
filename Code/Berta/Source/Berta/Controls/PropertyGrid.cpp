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
		}

		void Reactor::Update(Graphics& graphics)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_module.m_owner->Appearance.get());
			m_module.m_layout.Draw(graphics, m_module.m_model, appearance);
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
			/*auto category = m_module.GetCategoryOnMouse(args.Position);
			bool needRefresh = category != m_module.m_mouseInteraction.m_selectedCategory;
			m_module.m_mouseInteraction.m_selectedCategory = category;
			PropertyGridFieldBase* lastPropertySelected = nullptr;

			if (!category)
			{
				lastPropertySelected = m_module.GetCategoryPropertyOnMouse(args.Position);
			}
			needRefresh |= m_module.m_mouseInteraction.m_lastPropertySelected != lastPropertySelected;
			m_module.m_mouseInteraction.m_lastPropertySelected = lastPropertySelected;

			if (needRefresh)
			{
				GUI::MarkAsNeedUpdate(m_module.m_owner);
			}*/
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			/*if (args.ButtonState.LeftButton)
				return;

			auto category = m_module.GetCategoryOnMouse(args.Position);
			if (category != m_module.m_mouseInteraction.m_hoveredCategory)
			{
				m_module.m_mouseInteraction.m_hoveredCategory = category;
				GUI::UpdateWindow(m_module.m_owner);
			}*/
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			if (!args.ButtonState.LeftButton)
				return;

			Point clickPos = { args.Position.X, args.Position.Y + m_module.m_layout.m_scrollableView->GetScrollOffset().Y };
    
			// Iniciamos la búsqueda recursiva de colisión
			m_module.ProcessClickRecursive(m_module.m_model.GetRootCategories(), clickPos, 0);
			
			/*if (m_module.m_mouseInteraction.m_selectedCategory)
			{
				m_module.m_mouseInteraction.m_selectedCategory->m_isExpanded = !m_module.m_mouseInteraction.m_selectedCategory->m_isExpanded;
				m_module.m_mouseInteraction.m_selectedCategory = nullptr;

				
				m_module.UpdateScrollBar();

				GUI::UpdateWindow(m_module.m_owner);
			}*/
		}

		void Reactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
		{
			m_module.m_layout.m_scrollableView->HandleMouseWheel(args);
		}

		void Reactor::Resize(Graphics& graphics, const ArgResize& args)
		{
			m_module.m_layout.m_scrollableView->SetViewRect(Rectangle(0, 0, args.NewSize.Width, args.NewSize.Height));
		
			m_module.m_layout.CalculateLayout(m_module.m_model);
			m_module.UpdateScrollBar();
		}

		/*void Module::BuildItems()
		{
			auto one = m_owner->ToScale(1);
			Point offset{};
			for (auto& category : m_categories)
			{
				Rectangle categoryRect{ offset.X + m_viewport.m_backgroundRect.X,
					offset.Y + m_viewport.m_backgroundRect.Y,
					m_viewport.m_backgroundRect.Width - m_viewport.m_backgroundRect.X * 2, m_viewport.m_categoryItemHeight };

				category.m_area = categoryRect;
				offset.Y += static_cast<int>(categoryRect.Height);

				if (category.m_isExpanded)
				{
					for (size_t i = 0; i < category.m_properties.size(); i++)
					{
						auto field = category.m_properties[i].get();
						auto fieldSize = field->GetSize();

						offset.Y += static_cast<int>(fieldSize);
					}
				}
				offset.Y += one;
			}
		}*/

		void Module::Clear()
		{
			m_model.Clear();
			m_layout.CalculateLayout(m_model);
			
			m_mouseInteraction.m_lastPropertySelected = nullptr;
		}

		/*void Module::CalculateViewport(ViewportData& viewportData)
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
			if (viewportData.m_needVerticalScroll)
			{
				viewportData.m_backgroundRect.Width -= m_owner->ToScale(m_owner->Appearance->ScrollBarSize);
			}
		}*/

		/*void Module::CalculateContentSize(ViewportData& viewportData)
		{
			viewportData.m_contentSize = viewportData.m_categoryItemHeight * static_cast<uint32_t>(m_listModule.Size());

			for (auto& category : m_categories)
			{
				if (category.m_isExpanded)
				{
					for (size_t i = 0; i < category.m_properties.size(); i++)
					{
						auto field = category.m_properties[i].get();
						auto fieldSize = field->GetSize();

						viewportData.m_contentSize += fieldSize;
					}
				}
			}
		}*/

		void Module::EmitEvent(PropertyItem item) const
		{
			ArgPropertyGrid args(item);
			m_events->PropertyChanged.Emit(args);
		}

		void Module::EmitSelectionEvent(PropertyItem item)
		{
			/*if (m_mouseInteraction.m_lastPropertySelected == item.m_propGridField)
				return;

			ArgPropertyGrid args(item);
			m_events->SelectionChanged.Emit(args);

			m_mouseInteraction.m_lastPropertySelected = item.m_propGridField;*/
		}

		void Module::Update()
		{
			if (!m_owner->Flags.AutoDraw)
				return;

			GUI::UpdateWindow(m_owner);
		}

		void Module::UpdateScrollBar()
		{
			/*auto scrollSize = m_owner->ToScale(m_owner->Appearance->ScrollBarSize);
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

				m_scrollBar->SetMinMax(0, static_cast<int>(m_viewport.m_contentSize - m_viewport.m_backgroundRect.Height));
				m_scrollBar->SetPageStepValue(m_viewport.m_backgroundRect.Height);
				m_scrollBar->SetStepValue(m_owner->ToScale(24));

				m_scrollOffset.Y = m_scrollBar->GetValue();
			}
			else if (m_scrollBar)
			{
				m_scrollBar.reset();
				m_scrollOffset.Y = 0;
			}*/
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

		/*CategoryType* Module::GetCategoryOnMouse(const Point& mousePosition)
		{
			Point offsetPosition = mousePosition + m_scrollOffset;
			for (auto& category : m_categories)
			{
				if (category.m_area.Contains(offsetPosition))
					return &category;
			}
			return nullptr;
		}

		PropertyGridFieldBase* Module::GetCategoryPropertyOnMouse(const Point& mousePosition)
		{
			for (auto& category : m_categories)
			{
				Rectangle& categoryRect = category.m_area;

				Point scrollOffset{ categoryRect.X,categoryRect.Y };
				scrollOffset.Y += static_cast<int>(categoryRect.Height);
				scrollOffset.Y -= m_scrollOffset.Y;

				if (!category.m_isExpanded)
				{
					continue;
				}

				for (size_t i = 0; i < category.m_properties.size(); i++)
				{
					auto field = category.m_properties[i].get();
					auto fieldSizeInt = static_cast<int>(field->GetSize());
					if (mousePosition.Y >= scrollOffset.Y && mousePosition.Y < scrollOffset.Y + fieldSizeInt)
					{
						return field;
					}

					scrollOffset.Y += fieldSizeInt;
				}
			}
			return nullptr;
		}*/

		void Module::ScrollToView(PropertyGridFieldBase* propGridField)
		{
            /*if (!m_scrollBar)
            {
                return;
            }
            
			Rectangle itemBounds{ m_viewport.m_backgroundRect.X, - m_scrollOffset.Y,
				m_viewport.m_backgroundRect.Width, 
				0
			};
			bool found = false;
			for (auto& category : m_categories)
			{
				const Rectangle& categoryRect = category.m_area;
				itemBounds.Y += static_cast<int>(categoryRect.Height);

				if (!category.m_isExpanded)
				{
					continue;
				}

				for (size_t i = 0; i < category.m_properties.size(); i++)
				{
					auto field = category.m_properties[i].get();
					if (field == propGridField)
					{
						itemBounds.Height = field->GetSize();
						found = true;
						break;
					}
					
					itemBounds.Y += static_cast<int>(field->GetSize());
				}
				
				if (found)
				{
					break;
				}
			}
			
			if (itemBounds.Y >= 0 && itemBounds.Y +  static_cast<int>(itemBounds.Height) <= static_cast<int>(m_viewport.m_backgroundRect.Height))
			{
				return;
			}

			if (found)
			{
				int offsetAdjustment;
				if (itemBounds.Y + static_cast<int>(itemBounds.Height) >= static_cast<int>(m_viewport.m_backgroundRect.Height))
				{
					offsetAdjustment = itemBounds.Y + static_cast<int>(itemBounds.Height - m_viewport.m_backgroundRect.Height);
				}
				else
				{
					offsetAdjustment = itemBounds.Y;
				}
				m_scrollOffset.Y = std::clamp(m_scrollOffset.Y + offsetAdjustment, m_scrollBar->GetMin(), m_scrollBar->GetMax());
		
				m_scrollBar->SetValue(m_scrollOffset.Y);
			}
			
			GUI::UpdateWindow(m_owner);*/
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
				// El modelo retorna CategoryType* (Puntero crudo)
				CategoryType* rawSubCat = m_model->AppendSubCategory(m_id, name);
        
				if (rawSubCat) {
					// Lo envolvemos inmediatamente en un Handle seguro
					return CategoryItem(m_model, rawSubCat->m_id);
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

			// Configuración inicial del scroll
			m_scrollableView->SetScrollStep(m_config.CategoryHeight, 0);
			m_scrollableView->SetOnScrollChange([this]()
			{
				if (m_owner) {
					// Invalidamos para repintar al scrollear
					//GUI::InvalidateWindowArea(m_owner, m_scrollableView->GetClientArea()); 
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

			for (auto& cat : model.GetRootCategories()) {
				currentY = DrawRecursive(graphics, cat, currentY);
			}
			/*
			Point offset = m_scrollableView->GetScrollOffset();
			Rectangle visibleRect = m_scrollableView->GetVisibleRect();

			// Empezamos a dibujar en negativo según el scroll
			int currentX = -offset.X; 
			int currentY = -offset.Y; 
			auto width = visibleRect.Width;

			for (const auto& category : model.GetCategories())
			{
				// --- DIBUJAR CATEGORÍA ---
				Rectangle catArea{ currentX, currentY, width, m_config.CategoryHeight };
            
				// Culling (Optimización: solo dibujamos si está dentro de la pantalla)
				if (catArea.Y + catArea.Height > 0 && catArea.Y < visibleRect.Height)
				{
					// Aquí dibujas el fondo de la categoría, el triángulo de expandir y el texto
					graphics.DrawRoundRectBox(catArea, appearance->ButtonBackground, appearance->BoxBorderColor, true);
				}
            
				currentY += (int)m_config.CategoryHeight;

				// --- DIBUJAR PROPIEDADES ---
				if (category.m_isExpanded)
				{
					for (const auto& item : category.m_properties)
					{
						auto propHeight = item.field->GetHeight();
						Rectangle propArea{ currentX, currentY, width, propHeight };

						if (propArea.Y + propArea.Height > 0 && propArea.Y < visibleRect.Height)
						{
							item.field->Draw(graphics, propArea, 120, *appearance);
						}
						currentY += (int)propHeight;
					}
				}
			}*/
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
			int indent = cat.m_depth * 20; // 20px de sangría por nivel

			// 1. Dibujar Header de la categoría
			Rectangle area{ indent, y, width - indent, m_config.CategoryHeight };
			if (IsVisible(area))
			{
				DrawCategoryHeader(graphics, area, cat, m_config);
			}
			y += m_config.CategoryHeight;

			if (cat.m_isExpanded)
			{
				// 2. Dibujar Propiedades
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

				// 3. Dibujar Subcategorías (Recursión)
				for (const auto& sub : cat.m_subCategories)
				{
					y = DrawRecursive(graphics, sub, y);
				}
			}
			return y;
		}

		void PropertyGridLayout::DrawCategoryHeader(Graphics& graphics, const Rectangle& area, const CategoryType& cat,
			const Appearance& config)
		{
			Color bgColor = m_config.Background;
			graphics.FillRectangle(area, bgColor);

			// 2. DIBUJAR EL ICONO DE EXPANSIÓN (El "Expander")
			// Calculamos una pequeña cajita a la izquierda, centrada verticalmente.
			int expanderSize = 14;
			int centerY = area.Y + (area.Height - expanderSize) / 2;
			Rectangle expanderArea{ area.X + 4, centerY, (uint32_t)expanderSize, (uint32_t)expanderSize };

			// Si tu librería soporta caracteres Unicode, puedes usar "▼" y "▶"
			// Si usas fuentes ASCII simples, "+" y "-" son el estándar seguro.
			const char* expanderIcon = cat.m_isExpanded ? "-" : "+"; 
    
			// (Opcional) Si tu API tiene para dibujar geometría, un triángulo queda súper pro:
			// if (cat.m_isExpanded) graphics.DrawTriangleDown(...);
			// else graphics.DrawTriangleRight(...);
			
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
			int textX = expanderArea.X + (int)expanderArea.Width + 4;
			Point textPos = { textX, area.Y + ((int)area.Height - 14) / 2 }; // Centrado verticalmente
    
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
                
				// IMPORTANTE: Avisar al Layout que necesita redibujarse
				// (Depende de cómo hayas conectado tu modelo con tu reactor, 
				// podrías necesitar emitir un evento OnLayoutChanged aquí o en el modelo).
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

			//DrawLabel(graphics, labelArea, textColor);
		}

		void PropertyGridFieldBase::EmitEvent()
		{
			//m_module->EmitEvent(PropertyItem{ m_module, this });
		}

		void PropertyGridFieldBase::EmitSelectionEvent()
		{
			//m_module->EmitSelectionEvent(PropertyItem{ m_module, this });
		}

		void PropertyGridFieldBase::ScrollToView()
		{
			m_module->ScrollToView(this);
		}

		void PropertyGridFieldBase::Update()
		{
			m_module->Update();
		}

		void PropertyGridFieldBase::DrawLabel(Graphics& graphics, const Rectangle& area, const Color& textColor)
		{
			auto& textExtents = graphics.GetTextExtent();
			Point position = area;
			position.Y += static_cast<int>((area.Height - textExtents.Height) >> 1);

			graphics.DrawString(position, m_label, textColor);
		}

		void PropertyGridFieldBase::SetModule(Module* module)
		{
			m_module = module;
		}

		FieldControlContainer::FieldControlContainer(Window* parent, const Rectangle& rect) :
			Panel(parent, rect)
		{
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
			// 1. Protección vital: Si el Pool o el make_unique falló, no hacemos nada
			if (!field)
			{
				return;
			}
			
			// 2. Buscamos la categoría (ya sea raíz o subcategoría)
			if (CategoryType* cat = FindCategoryById(categoryId))
			{
				StringUtils::StringHash permanentId = StringUtils::HashString(field->GetLabel());
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
			}
		}

		std::string PropertyGridModel::GetPropertyValueAsString(StringUtils::StringHash catId,
		                                                        StringUtils::StringHash propId)
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

		CategoryItem Module::Append(std::string_view categoryName)
		{
			auto& newCategory = m_model.AppendRootCategory(categoryName);
			m_layout.CalculateLayout(m_model);

			return {&m_model, newCategory.m_id};
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
