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
		}

		void Reactor::Update(Graphics& graphics)
		{
			m_module.m_layout.Draw(graphics, m_module.m_model);
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

			Point scrollOffset = m_module.m_layout.m_scrollableView->GetScrollOffset();
			int clickX = args.Position.X;
			int clickY = args.Position.Y + scrollOffset.Y; 

			int currentY = 0;
			const auto& config = m_module.m_layout.GetConfig();
			auto width = m_module.m_layout.m_scrollableView->GetClientArea().Width;

			for (auto& category : m_module.m_model.GetCategories())
			{
				Rectangle catRect{ 0, currentY, width, config.CategoryHeight };

				// 1. ¿Clic en la Categoría?
				if (catRect.Contains({ clickX, clickY }))
				{
					category.m_isExpanded = !category.m_isExpanded;
					m_module.m_layout.CalculateLayout(m_module.m_model);
					//if (auto owner = m_control->Handle()) {
						//GUI::InvalidateWindowArea(owner, m_module.m_layout.m_scrollableView->GetClientArea());
					//}
					return; 
				}

				currentY += config.CategoryHeight;

				// 2. ¿Clic en alguna Propiedad?
				if (category.m_isExpanded)
				{
					for (auto& item : category.m_properties)
					{
						Rectangle propRect{ 0, currentY, width, config.PropertyHeight };

						if (propRect.Contains({ clickX, clickY }))
						{
							// Traducimos el clic global a coordenadas locales de la propiedad
							// Esto facilita saber si se hizo clic en la etiqueta o en el control
							Point localClick = { clickX - propRect.X, clickY - propRect.Y };

							// Le delegamos la acción al control real
							item.field->OnMouseClick(localClick, config.LabelWidth);

							return; // Salimos temprano, ya procesamos el clic
						}

						currentY += config.PropertyHeight;
					}
				}
			}
			
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

		CategoryItem Module::Find(std::string_view categoryName)
		{
			auto it = std::find_if(m_model.GetCategories().begin(), m_model.GetCategories().end(), 
			[categoryName](const CategoryType& cat) { return cat.m_name == categoryName; });

			if (it != m_model.GetCategories().end())
			{
				return { this, &(*it) };
			}
			return {};
		}

		void Module::Clear()
		{
			//m_listModule.Clear();
			m_layout.CalculateLayout(m_model);
			UpdateScrollBar();
			
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
			if (m_mouseInteraction.m_lastPropertySelected == item.m_propGridField)
				return;

			ArgPropertyGrid args(item);
			m_events->SelectionChanged.Emit(args);

			m_mouseInteraction.m_lastPropertySelected = item.m_propGridField;
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
			auto& graphics = *m_graphics;
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

			graphics.DrawRectangle(clientRect, m_owner->Appearance->BoxBorderColor);
		}

		CategoryType* Module::GetCategoryOnMouse(const Point& mousePosition)
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
		}

		void Module::ScrollToView(PropertyGridFieldBase* propGridField)
		{
            if (!m_scrollBar)
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
			
			GUI::UpdateWindow(m_owner);
		}

		CategoryType* ListModule::CreateCategory(const std::string& categoryName)
		{
			m_categories.emplace_back(categoryName);
			return &m_categories.back();
		}

		std::vector<CategoryType>::iterator ListModule::Begin()
		{
			return m_categories.begin();
		}

		std::vector<CategoryType>::const_iterator ListModule::Begin() const
		{
			return m_categories.cbegin();
		}

		std::vector<CategoryType>::iterator ListModule::End()
		{
			return m_categories.end();
		}

		std::vector<CategoryType>::const_iterator ListModule::End() const
		{
			return m_categories.cend();
		}

		void ListModule::Clear()
		{
			m_categories.clear();
		}

		PropertyItem CategoryItem::Append(PropertyGridFieldBasePtr propGridFieldPtr)
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
		}

		CategoryItem::operator bool() const
		{
			return m_module != nullptr && m_category != nullptr;
		}

		PropertyGridLayout::PropertyGridLayout(Window* owner, const LayoutConfig& config)
        : m_owner(owner), m_config(config)
    {
        m_internalScrollManager = std::make_unique<ScrollableView>(m_owner);
        m_scrollableView = m_internalScrollManager.get();

        // Configuración inicial del scroll
        m_scrollableView->SetScrollStep(m_config.CategoryHeight, 0);
        
        m_scrollableView->SetOnScrollChange([this]() {
            if (m_owner) {
                // Invalidamos para repintar al scrollear
                //GUI::InvalidateWindowArea(m_owner, m_scrollableView->GetClientArea()); 
            }
        });
    }

    void PropertyGridLayout::CalculateLayout(const PropertyGridModel& model)
    {
        uint32_t totalHeight = 0;
        uint32_t maxWidth = m_scrollableView->GetClientArea().Width; // O un ancho fijo si no quieres scroll horizontal

        for (const auto& category : model.GetCategories())
        {
            // La categoría siempre ocupa espacio
            totalHeight += m_config.CategoryHeight;

            // Si está expandida, sumamos el espacio de sus propiedades
            if (category.m_isExpanded)
            {
                // Asumiendo que todas las propiedades tienen el mismo alto. 
                // Si tienen alto dinámico, deberás preguntar a cada item.
                totalHeight += static_cast<uint32_t>(category.m_properties.size()) * m_config.PropertyHeight;
            }
        }

        // Le informamos al scroll el tamaño total del lienzo interno
        m_scrollableView->SetContentSize({ static_cast<int>(maxWidth), static_cast<int>(totalHeight) });
    }

    void PropertyGridLayout::Draw(Graphics& graphics, const PropertyGridModel& model)
    {
        Point offset = m_scrollableView->GetScrollOffset();
        Rectangle visibleRect = m_scrollableView->GetVisibleRect();

        // Empezamos a dibujar en negativo según el scroll
        int currentY = -offset.Y; 
        int width = visibleRect.Width;

        for (const auto& category : model.GetCategories())
        {
            // --- DIBUJAR CATEGORÍA ---
            Rectangle catArea{ 0, currentY, width, static_cast<int>(m_config.CategoryHeight) };
            
            // Culling (Optimización: solo dibujamos si está dentro de la pantalla)
            if (catArea.Y + catArea.Height > 0 && catArea.Y < visibleRect.Height)
            {
                // Aquí dibujas el fondo de la categoría, el triángulo de expandir y el texto
                // graphics.FillRectangle(catArea, Color::DarkGray);
                // graphics.DrawString({15, currentY}, category.m_name, m_config.TextColor);
            }
            
            currentY += m_config.CategoryHeight;

            // --- DIBUJAR PROPIEDADES ---
            if (category.m_isExpanded)
            {
                for (const auto& item : category.m_properties)
                {
                    Rectangle propArea{ 0, currentY, width, static_cast<int>(m_config.PropertyHeight) };

                    if (propArea.Y + propArea.Height > 0 && propArea.Y < visibleRect.Height)
                    {
                        // Inyectamos el área calculada a la propiedad para que se dibuje
                        item.field->Draw(graphics, propArea, m_config.LabelWidth, m_config.TextColor);
                    }
                    currentY += m_config.PropertyHeight;
                }
            }
        }
    }

		PropertyItem::operator bool() const
		{
			return m_module && m_propGridField;
		}

		std::string PropertyItem::GetLabel() const
		{
			return m_propGridField->GetLabel();
		}

		PropertyItem& PropertyItem::SetLabel(const std::string& label)
		{
			m_propGridField->SetLabel(label);
			m_module->Update();

			return *this;
		}

		bool PropertyItem::IsEnabled() const
		{
			return m_propGridField->IsEnabled();
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

		void PropertyGridFieldBase::SetLabel(const std::string& label)
		{
			if (m_label == label)
				return;

			m_label = label;
		}

		bool PropertyGridFieldBase::IsEnabled() const
		{
			return m_enabled;
		}

		void PropertyGridFieldBase::SetEnabled(bool enabled)
		{
			m_enabled = enabled;
		}

		void PropertyGridFieldBase::Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor)
		{
			Rectangle labelArea = area;
			labelArea.Width = labelWidth;

			DrawLabel(graphics, labelArea, textColor);
		}

		void PropertyGridFieldBase::EmitEvent()
		{
			m_module->EmitEvent(PropertyItem{ m_module, this });
		}

		void PropertyGridFieldBase::EmitSelectionEvent()
		{
			m_module->EmitSelectionEvent(PropertyItem{ m_module, this });
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

		CategoryItem PropertyGridModel::AppendCategory(std::string_view categoryName)
		{
			StringUtils::StringHash hash = StringUtils::HashString(categoryName);
            
			// Si no existe, la creamos
			if (!FindCategoryById(hash)) {
				m_categories.emplace_back(categoryName); // CategoryType se crea con el hash
			}

			return CategoryItem(this, hash);
		}

		CategoryType* PropertyGridModel::FindCategoryById(StringUtils::StringHash id)
		{
			for (auto& cat : m_categories) {
				if (cat.m_id == id) return &cat;
			}
			return nullptr;
		}

		void PropertyGridModel::AppendPropertyToCategory(StringUtils::StringHash catId,
			std::unique_ptr<PropertyGridFieldBase> field)
		{
		}

		void PropertyGridModel::SetPropertyEnabled(StringUtils::StringHash catId, StringUtils::StringHash propId,
			bool enabled)
		{
		}

		std::string PropertyGridModel::GetPropertyValueAsString(StringUtils::StringHash catId,
			StringUtils::StringHash propId)
		{
			return "";
		}

		void PropertyGridModel::Clear()
		{
			m_categories.clear();
		}

		CategoryItem Module::Append(std::string_view categoryName)
		{
			auto category = m_model.AppendCategory(categoryName);
			

			CategoryItem newCategory = { this, m_listModule.CreateCategory(categoryName) };
			CalculateViewport(m_viewport);
			BuildItems();
			UpdateScrollBar();

			return newCategory;
		}
	}

	PropertyGrid::PropertyGrid(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "PropertyGrid";
#endif
	}

	PropertyGrid::CategoryItem PropertyGrid::Append(const std::string& categoryName)
	{
		return GetReactor().GetModule().Append(categoryName);
	}

	void PropertyGrid::Clear()
	{
		GetReactor().GetModule().Clear();
	}

	PropertyGrid::CategoryItem PropertyGrid::Insert(CategoryItem existingCategory, const std::string& categoryName)
	{
		return { nullptr, 0 };
	}

	PropertyGrid::CategoryItem PropertyGrid::Find(const std::string& categoryName)
	{
		return GetReactor().GetModule().Find(categoryName);
	}
}
