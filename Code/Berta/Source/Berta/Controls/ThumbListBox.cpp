/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ThumbListBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/ControlAppearance.h"
#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	void ThumbListBoxReactor::Init(ControlBase& control, Graphics* graphics)
	{
		m_control = &control;
		m_module.m_events = reinterpret_cast<ThumbListBoxEvents*>(control.Handle()->Events.get());

		m_module.m_window = control.Handle();
		m_module.m_control = m_control;
		
		m_module.InitScrollableView();
	}

	void ThumbListBoxReactor::Update(Graphics& graphics)
	{
		//BT_CORE_TRACE << "  - ThumbListBoxReactor::Update " << std::endl;
		auto appearance = reinterpret_cast<ThumbListBoxAppearance*>(m_control->Handle()->Appearance.get());
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();

		graphics.DrawRectangle(window->ClientSize.ToRectangle(), window->Appearance->BoxBackground, true);

		if (m_module.m_items.empty() || !m_module.m_scrollableView) return;
		
		auto scrollOffset = m_module.m_scrollableView->GetScrollOffset();
		Point offset{ -scrollOffset.X, -scrollOffset.Y };

		auto firstBounds = m_module.GetItemBounds(0);
		int cardWidth = firstBounds.Width;
		int cardHeight = firstBounds.Height;
		if (cardWidth == 0 || cardHeight == 0) return;
		int columns = std::max<int>(1, static_cast<int>(window->ClientSize.Width / cardWidth));
		
		int firstVisibleRow = std::max<int>(0, scrollOffset.Y / cardHeight);
		int visibleRows = static_cast<int>(std::ceil(window->ClientSize.Height / static_cast<float>(cardHeight))) + 2;
		
		size_t startIndex = static_cast<size_t>(firstVisibleRow) * columns;
		size_t endIndex = std::min<size_t>(m_module.m_items.size(), startIndex + static_cast<size_t>(visibleRows * columns));
		auto thumbSizeScale = window->ToScale(m_module.m_thumbnailSize);
		Size thumbFrameSize{ thumbSizeScale, thumbSizeScale };
		for (size_t i = startIndex; i < endIndex; i++)
		{
			auto& item = m_module.m_items[i];
			Rectangle cardRect = m_module.GetItemBounds(i);
			cardRect.X += offset.X;
			cardRect.Y += offset.Y;

			Rectangle thumbnailRect = { cardRect.X, cardRect.Y, thumbFrameSize.Width, thumbFrameSize.Height };
			
			const bool& isSelected = m_module.m_selectionController.IsSelected(i);;
			bool isLastSelected = (int)i == 3323;
			graphics.DrawRectangle(cardRect, window->Appearance->ButtonBackground, true);
			graphics.DrawRectangle(thumbnailRect, window->Appearance->Background, true);

			// INTEGRACIÓN DE CACHÉ: Intentar obtener la imagen
			Image cachedImage;
			if (item.m_hasThumbnail && m_module.m_imageCache.TryGet(i, cachedImage))
			{
				Size imageSize = window->ToScale(cachedImage.GetSize());
				Rectangle thumbnailImageRect;
				if (imageSize.Width > thumbFrameSize.Width || imageSize.Height > thumbFrameSize.Height)
				{
					thumbnailImageRect = { cardRect.X, cardRect.Y, thumbFrameSize.Width, thumbFrameSize.Height };
				}
				else
				{
					Point center = thumbFrameSize;
					center -= imageSize;
					center /= 2;
					thumbnailImageRect = { cardRect.X + center.X, cardRect.Y + center.Y, imageSize.Width, imageSize.Height };
				}

				cachedImage.Paste(graphics, thumbnailImageRect);
			}
			/*if (item.m_thumbnail)
			{
				Size imageSize = window->ToScale(item.m_thumbnail.GetSize());
				Rectangle thumbnailImageRect;
				if (imageSize.Width > thumbFrameSize.Width || imageSize.Height > thumbFrameSize.Height)
				{
					thumbnailImageRect = { cardRect.X, cardRect.Y, thumbFrameSize.Width, thumbFrameSize.Height };
				}
				else
				{
					Point center = thumbFrameSize;
					center -= imageSize;
					center /= 2;

					thumbnailImageRect = { cardRect.X + center.X, cardRect.Y + center.Y, imageSize.Width, imageSize.Height };
				}

				item.m_thumbnail.Paste(graphics, thumbnailImageRect);
			}*/
			
			if (isSelected)
			{
				graphics.DrawRectangle({ cardRect.X , cardRect.Y + (int)thumbSizeScale, cardRect.Width, cardHeight - thumbSizeScale }, window->Appearance->HighlightColor, true);
				//graphics.DrawRectangle({ cardRect.X , cardRect.Y + (int)thumbSize, cardRect.Width, cardHeight }, window->Appearance->HighlightColor, true);
			}
			m_module.DrawItemText(graphics, item, { cardRect.X, cardRect.Y + (int)thumbSizeScale, cardRect.Width, cardRect.Height });
			auto lineColor = enabled ? (isLastSelected ? window->Appearance->Foreground : (isSelected ? window->Appearance->BoxBorderHighlightColor : window->Appearance->BoxBorderColor)) : window->Appearance->BoxBorderDisabledColor;
			graphics.DrawRectangle(cardRect, lineColor, false);
			graphics.DrawLine({ cardRect.X, cardRect.Y + (int)thumbSizeScale }, { cardRect.X + (int)cardRect.Width - 1, cardRect.Y + (int)thumbSizeScale }, lineColor);
		}
		
		if (m_module.m_lassoSelection.IsActive())
		{
			Rectangle lassoRect = m_module.m_lassoSelection.GetRect();
			if (lassoRect.Width > 0 && lassoRect.Height > 0)
			{
				Color blendColor = window->Appearance->SelectionHighlightColor;
				Color borderColor = window->Appearance->SelectionBorderHighlightColor;

				// Creamos la superficie gráfica temporal para el lazo
				Graphics selectionBox(lassoRect, window->DPI, window->RootPaintHandle);
				selectionBox.Begin();

				// Tu propia clase se encarga de aplicar los rectángulos y hacer el Flush
				m_module.m_lassoSelection.Draw(graphics, selectionBox, blendColor, borderColor);
			}
		}

		graphics.DrawRectangle(window->ClientSize.ToRectangle(), enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);
	}

	void ThumbListBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.UpdateScrollContentSize();
	}

	void ThumbListBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (!args.ButtonState.LeftButton) return;

		int clickedIndex = m_module.GetItemIndexAtMousePosition(args.Position);

		if (clickedIndex != -1)
		{
			// Hemos hecho clic en un elemento válido.
			
			// 3. EL RESOLVEDOR DE RANGOS: Le enseñamos al controlador cómo llenar huecos numéricos
			auto rangeResolver = [](const size_t& anchor, const size_t& current)
			{
				std::vector<size_t> range;
				size_t start = std::min<size_t>(anchor, current);
				size_t end = std::max<size_t>(anchor, current);
				for (size_t i = start; i <= end; ++i) {
					range.push_back(i);
				}
				return range;
			};

			// 4. DELEGACIÓN: El controlador se encarga de toda la lógica compleja
			bool selectionChanged = m_module.m_selectionController.Select
			(
				static_cast<size_t>(clickedIndex), 
				m_module.m_ctrlPressed, 
				m_module.m_shiftPressed, 
				rangeResolver
			);

			if (selectionChanged && m_module.m_events) {
				//m_module.m_events->Selected.Emit({ static_cast<size_t>(clickedIndex) });
			}
		}
		else
		{
			// Hicimos clic en el fondo vacío
			if (!m_module.m_ctrlPressed && !m_module.m_shiftPressed) {
				m_module.m_selectionController.Clear();
				if (m_module.m_events)
				{
					//m_module.m_events->Selected.Fire({ (size_t)-1 });
				}
			}

			// Iniciar selección por arrastre (Drag Box)
			m_module.m_isDraggingSelection = true;
			m_module.m_dragStartPosition = args.Position;
			m_module.m_dragEndPosition = args.Position;
			m_module.m_lassoSelection.Start(args.Position);
		}
		
		GUI::MarkAsNeedUpdate(*m_control);
	}

	void ThumbListBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_lassoSelection.IsActive())
		{
			if (m_module.m_lassoSelection.Update(args.Position))
			{
				// Si el ratón se movió, calculamos qué elementos quedaron dentro del rectángulo
				Rectangle lassoScreenRect = m_module.m_lassoSelection.GetRect();
				auto scrollOffset = m_module.m_scrollableView->GetScrollOffset();

				// Convertimos el lazo a coordenadas absolutas de la cuadrícula
				Rectangle lassoWorldRect = { 
					lassoScreenRect.X + scrollOffset.X, 
					lassoScreenRect.Y + scrollOffset.Y, 
					lassoScreenRect.Width, 
					lassoScreenRect.Height 
				};

				auto firstBounds = m_module.GetItemBounds(0);
				if (firstBounds.Width > 0 && firstBounds.Height > 0)
				{
					int columns = std::max<int>(1, static_cast<int>(m_module.m_window->ClientSize.Width / firstBounds.Width));

					// Calculamos matemáticamente qué filas y columnas interceptan con el lazo
					int startCol = std::max<int>(0, lassoWorldRect.X / static_cast<int>(firstBounds.Width));
					int endCol = std::min<int>(columns - 1, (lassoWorldRect.X + static_cast<int>(lassoWorldRect.Width)) / static_cast<int>(firstBounds.Width));
					
					int startRow = std::max<int>(0, lassoWorldRect.Y / static_cast<int>(firstBounds.Height));
					int endRow = (lassoWorldRect.Y + static_cast<int>(lassoWorldRect.Height)) / static_cast<int>(firstBounds.Height);

					// Limpiamos selecciones previas si no presionamos Control
					if (!m_module.m_ctrlPressed) {
						m_module.m_selectionController.Clear();
					}

					// Seleccionamos instantáneamente los elementos en ese bloque de la matriz
					bool selectionChanged = false;
					for (int row = startRow; row <= endRow; ++row) {
						for (int col = startCol; col <= endCol; ++col) {
							size_t index = (row * columns) + col;
							if (index < m_module.m_items.size()) {
								m_module.m_selectionController.SetSelected(index, true);
								selectionChanged = true;
							}
						}
					}

					if (selectionChanged && m_module.m_events) {
						// Disparamos evento del último seleccionado, o envías un vector
						//m_module.m_events->Selected.Fire({ (size_t)-1 }); 
					}
				}
				
				GUI::MarkAsNeedUpdate(*m_control);
			}
		}
	}

	void ThumbListBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_lassoSelection.IsActive())
		{
			m_module.m_lassoSelection.End();
			GUI::MarkAsNeedUpdate(*m_control);
		}
	}

	void ThumbListBoxReactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
	{
		if (m_module.m_scrollableView) {
			m_module.m_scrollableView->HandleMouseWheel(args);
		}
	}

	void ThumbListBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		m_module.m_shiftPressed = m_module.m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_module.m_ctrlPressed = m_module.m_ctrlPressed || args.Key == KeyboardKey::Control;

		/*bool needUpdate = false;
		if (args.Key == KeyboardKey::ArrowLeft || args.Key == KeyboardKey::ArrowRight || args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::ArrowDown)
		{
			if (args.Key == KeyboardKey::ArrowLeft || args.Key == KeyboardKey::ArrowRight)
			{
				auto direction = args.Key == KeyboardKey::ArrowLeft ? -1 : 1;
				auto pivot = (m_module.m_mouseSelection.m_selectedIndex == -1 ? (direction == -1 ? (int)m_module.m_items.size() : -1) : m_module.m_mouseSelection.m_selectedIndex);
				auto newItemIndex = pivot + direction;
				if (newItemIndex >= 0 && newItemIndex < static_cast<int>(m_module.m_items.size()))
				{
					if (!m_module.m_ctrlPressed)
					{
						m_module.ClearSelection();
					}

					if (m_module.m_multiselection && m_module.m_shiftPressed && m_module.m_mouseSelection.m_pivotIndex != -1)
					{
						int endIndex = newItemIndex;
						int startIndex = m_module.m_mouseSelection.m_pivotIndex;
						int minIndex = (std::min)(startIndex, endIndex);
						int maxIndex = (std::max)(startIndex, endIndex);

						for (int current = minIndex; current <= maxIndex; ++current)
						{
							m_module.m_items[current].m_isSelected = true;
							m_module.m_mouseSelection.m_selections.push_back(current);
						}
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
					}
					else if (m_module.m_ctrlPressed)
					{
						m_module.m_mouseSelection.m_selectedIndex += direction;
					}
					else
					{
						m_module.m_items[newItemIndex].m_isSelected = true;
						m_module.m_mouseSelection.m_selections.push_back(newItemIndex);
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
						m_module.m_mouseSelection.m_pivotIndex = newItemIndex;
					}
					m_module.EnsureVisibility(m_module.m_mouseSelection.m_selectedIndex);

					needUpdate = true;
				}
			}
			else if (args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::ArrowDown)
			{
				auto direction = args.Key == KeyboardKey::ArrowUp ? -1 : 1;
				auto pivot = (m_module.m_mouseSelection.m_selectedIndex == -1 ? (direction == -1 ? (int)m_module.m_items.size() : -1) : m_module.m_mouseSelection.m_selectedIndex);
				auto newItemIndex = pivot + direction * m_module.m_viewport.m_totalCardsInRow;
				if (newItemIndex >= 0 && newItemIndex < (int)m_module.m_items.size())
				{
					if (!m_module.m_ctrlPressed)
					{
						m_module.ClearSelection();
					}

					if (m_module.m_multiselection && m_module.m_shiftPressed && m_module.m_mouseSelection.m_pivotIndex != -1)
					{
						int endIndex = newItemIndex;
						int startIndex = m_module.m_mouseSelection.m_pivotIndex;
						int minIndex = (std::min)(startIndex, endIndex);
						int maxIndex = (std::max)(startIndex, endIndex);

						for (int current = minIndex; current <= maxIndex; ++current)
						{
							m_module.m_items[current].m_isSelected = true;
							m_module.m_mouseSelection.m_selections.push_back(current);
						}
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
					}
					else if (m_module.m_ctrlPressed)
					{
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
					}
					else
					{
						m_module.m_items[newItemIndex].m_isSelected = true;
						m_module.m_mouseSelection.m_selections.push_back(newItemIndex);
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
						m_module.m_mouseSelection.m_pivotIndex = newItemIndex;
					}
					m_module.EnsureVisibility(m_module.m_mouseSelection.m_selectedIndex);
					needUpdate = true;
				}
			}
		}
		else if (args.Key == KeyboardKey::Space && m_module.m_ctrlPressed)
		{
			if (m_module.m_mouseSelection.m_selectedIndex != -1)
			{
				if (!m_module.m_multiselection && !m_module.m_mouseSelection.m_selections.empty())
				{
					m_module.m_items[m_module.m_mouseSelection.m_selections[0]].m_isSelected = false;
					m_module.m_mouseSelection.m_selections.clear();
				}

				auto& isSelected = m_module.m_items[m_module.m_mouseSelection.m_selectedIndex].m_isSelected;
				isSelected = !isSelected;
				if (isSelected)
				{
					m_module.m_mouseSelection.Select(m_module.m_mouseSelection.m_selectedIndex);
				}
				else
				{
					m_module.m_mouseSelection.Deselect(m_module.m_mouseSelection.m_selectedIndex);
				}

				if (m_module.m_multiselection)
				{
					m_module.m_mouseSelection.m_pivotIndex = m_module.m_mouseSelection.m_selectedIndex;
				}
				needUpdate = true;
			}
		}

		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(*m_control);
		}*/
	}

	void ThumbListBoxReactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
	{
		if (args.Key == KeyboardKey::Shift) m_module.m_shiftPressed = false;
		if (args.Key == KeyboardKey::Control) m_module.m_ctrlPressed = false;
	}

	ThumbListBoxItem ThumbListBoxReactor::Module::At(size_t index)
	{
		return ThumbListBoxItem{ m_items[index], *this };
	}

	bool ThumbListBoxReactor::Module::AddItem(const std::wstring& text)
	{
		auto& newItem = m_items.emplace_back();
		newItem.m_text = text;

		UpdateScrollContentSize();

		return true;
	}

	bool ThumbListBoxReactor::Module::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		auto& newItem = m_items.emplace_back();
		newItem.m_text = text;
		newItem.m_hasThumbnail = true;

		m_imageCache.Put(m_items.size() - 1, thumbnail);

		UpdateScrollContentSize();

		return true;
	}

	bool ThumbListBoxReactor::Module::Clear()
	{
		bool needUpdate = !m_items.empty();
		m_items.clear();
		m_imageCache.Clear();
		m_selectionController.Clear();
		m_isDraggingSelection = false;
		UpdateScrollContentSize();
		return needUpdate;
	}

	void ThumbListBoxReactor::Module::Erase(size_t index)
	{
		if (m_items.size() <= index)
		{
			return;
		}
		m_imageCache.Erase(index);
		m_items.erase(m_items.begin() + index);
		
		m_selectionController.Clear();
		UpdateScrollContentSize();

		GUI::UpdateWindow(m_window);
	}

	void ThumbListBoxReactor::Module::SetThumbnailSize(uint32_t size)
	{
		if (m_thumbnailSize == size)
		{
			return;
		}

		m_thumbnailSize = size;
		UpdateScrollContentSize();

		GUI::UpdateWindow(m_window);
	}

	void ThumbListBoxReactor::Module::UpdateScrollContentSize()
	{
		if (!m_scrollableView || m_items.empty())
		{
			if (m_scrollableView)
				m_scrollableView->SetContentSize({ m_window->ClientSize.Width, 0 });
			return;
		}

		auto lastItemBounds = GetItemBounds(m_items.size() - 1);
		uint32_t totalHeight = lastItemBounds.Y + lastItemBounds.Height;
		
		m_scrollableView->SetContentSize({ m_window->ClientSize.Width, totalHeight });
		m_scrollableView->SetViewSize(m_window->ClientSize);
	}

	bool ThumbListBoxReactor::Module::IsEnabledMultiselection() const
	{
		return m_multiselection;
	}

	bool ThumbListBoxReactor::Module::EnableMultiselection(bool enabled)
	{
		bool needUpdate = m_multiselection != enabled;
		m_multiselection = enabled;
		return needUpdate;
	}

	int ThumbListBoxReactor::Module::GetItemIndexAtMousePosition(const Point& position)
	{
		auto scrollOffset = m_scrollableView->GetScrollOffset();
		Point absolutePt = { position.X + scrollOffset.X, position.Y + scrollOffset.Y };
		
		auto bounds = GetItemBounds(0);
		if (bounds.Width == 0 || bounds.Height == 0) return -1;

		int columns = std::max<int>(1, static_cast<int>(m_window->ClientSize.Width / bounds.Width));
		int col = absolutePt.X / bounds.Width;
		int row = absolutePt.Y / bounds.Height;
		
		if (col >= columns) return -1;
		
		int index = (row * columns) + col;
		if (index >= 0 && index < static_cast<int>(m_items.size()))
		{
			return index;
		}
		return -1;
	}

	Rectangle ThumbListBoxReactor::Module::GetItemBounds(size_t index) const
	{
		auto appearance = reinterpret_cast<ThumbListBoxAppearance*>(m_control->Handle()->Appearance.get());
		
		uint32_t thumbSizeScale = m_window->ToScale(m_thumbnailSize);
		uint32_t textHeightScale = m_window->ToScale(appearance->ThumbnailCardHeight);
		
		uint32_t cardWidth = thumbSizeScale;
		uint32_t cardHeight = thumbSizeScale + textHeightScale;
		
		if (cardWidth == 0) return {};

		int columns = std::max<int>(1, static_cast<int>(m_window->ClientSize.Width / cardWidth));
		int row = (int)index / columns;
		int col = (int)index % columns;
		
		return { 
			static_cast<int>(col * cardWidth), 
			static_cast<int>(row * cardHeight), 
			static_cast<uint32_t>(cardWidth), 
			static_cast<uint32_t>(cardHeight) 
		};
	}

	void ThumbListBoxReactor::Module::EmitVisibilityEvent(size_t index, bool visible) const
	{
		ArgThumbListBoxItemVisibility args;
		args.Index = index;
		args.Visible = visible;
		BT_CORE_DEBUG << " - visibility item = " << index << ". visible=" << visible << std::endl;
		m_events->ItemVisibility.Emit(args);
	}

	void ThumbListBoxReactor::Module::Draw() const
	{
		GUI::UpdateWindow(m_window);
	}

	void ThumbListBoxReactor::Module::DrawItem(Graphics& graphics, ItemType& item, Point& offset)
	{
	}

	void ThumbListBoxReactor::Module::DrawItemText(Graphics& graphics, ItemType& item, const Rectangle& cardRect)
	{
		auto wholeExtent = graphics.GetTextExtent(item.m_text);
		auto appearance = reinterpret_cast<ThumbListBoxAppearance*>(m_control->Handle()->Appearance.get());
		
		if (wholeExtent.Width < cardRect.Width)
		{
			Point targetSub{ cardRect.X, cardRect.Y };
			targetSub.X += static_cast<int>(cardRect.Width - wholeExtent.Width) >> 1;
			graphics.DrawString(targetSub, item.m_text, appearance->Foreground);
			return;
		}

		auto words = StringUtils::Split(item.m_text, ' ');
		Point offset{};
		for (size_t i = 0; i < words.size(); i++)
		{
			auto& word = words[i];
			auto wordExtent = graphics.GetTextExtent(word);

			if (wordExtent.Width + offset.X >= cardRect.Width)
			{
				for (size_t j = word.size() - 1; j > 0; --j)
				{
					auto subExtent = graphics.GetTextExtent(word, j);
					if (subExtent.Width + offset.X < cardRect.Width)
					{
						Point targetSub{ offset.X + cardRect.X, offset.Y + cardRect.Y};
						targetSub.X += static_cast<int>(cardRect.Width - subExtent.Width) >> 1;
						graphics.DrawString(targetSub, word.substr(0, j), appearance->Foreground);

						offset.X += static_cast<int>(subExtent.Width);
						break;
					}
				}
			}
			else
			{
				Point targetSub{ offset.X + cardRect.X, offset.Y + cardRect.Y };
				targetSub.X += static_cast<int>(cardRect.Width - wordExtent.Width) >> 1;
				graphics.DrawString(targetSub, word, appearance->Foreground);
				offset.Y += static_cast<int>(wordExtent.Height);

				offset.X = 0;
			}
		}
	}

	std::vector<size_t> ThumbListBoxReactor::Module::GetSelectedItems() const
	{
		return m_selectionController.GetSelectedItems();
	}

	bool ThumbListBoxReactor::Module::EnsureVisibility(int lastSelectedIndex)
	{
		return true;
	}

	void ThumbListBoxReactor::Module::UpdateItem(const ItemType& item) const
	{
		/*auto itemBounds = item.m_bounds;
		itemBounds.Y -= m_state.m_offset;

		if (!m_viewport.m_backgroundRect.Intersect(itemBounds))
		{
			return;
		}*/

		GUI::UpdateWindow(m_window);
	}

	void ThumbListBoxReactor::Module::InitScrollableView()
	{
		m_scrollableView = std::make_unique<ScrollableView>(m_window);
		
		m_scrollableView->SetScrollStep(20, 20);
		m_scrollableView->SetOnScrollChange([this]()
			{
				//m_module.EmitVisibilityEvent();
				GUI::MarkAsNeedUpdate(m_window);
			});
	}


	void ThumbListBoxItem::SetText(const std::wstring& text)
	{
		if (m_target.m_text == text)
		return;

		m_target.m_text = text;
		m_module.UpdateItem(m_target);
	}

	void ThumbListBoxItem::SetIcon(const Image& image)
	{
		/*if (m_target.m_thumbnail == image)
			return;

		m_target.m_thumbnail = image;
		m_module.UpdateItem(m_target);*/
	}

	ThumbListBox::ThumbListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "ThumbListBox";
#endif
	}

	void ThumbListBox::AddItem(const std::wstring& text)
	{
		if (GetReactor().GetModule().AddItem(text) && IsAutoDraw())
		{
			GetReactor().GetModule().Draw();
		}
	}

	void ThumbListBox::AddItem(const std::string& text)
	{
		if (GetReactor().GetModule().AddItem(StringUtils::UTF8ToWide(text)) && IsAutoDraw())
		{
			GetReactor().GetModule().Draw();
		}
	}

	void ThumbListBox::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		if (GetReactor().GetModule().AddItem(text, thumbnail) && IsAutoDraw())
		{
			GetReactor().GetModule().Draw();
		}
	}

	void ThumbListBox::AddItem(const std::string& text, const Image& thumbnail)
	{
		if (GetReactor().GetModule().AddItem(StringUtils::UTF8ToWide(text), thumbnail) && IsAutoDraw())
		{
			GetReactor().GetModule().Draw();
		}
	}

	ThumbListBoxItem ThumbListBox::At(size_t index)
	{
		return GetReactor().GetModule().At(index);
	}

	void ThumbListBox::Clear()
	{
		auto& module = GetReactor().GetModule();
		if (module.Clear() && IsAutoDraw())
		{
			module.Draw();
		}
	}

	void ThumbListBox::Erase(size_t index)
	{
		GetReactor().GetModule().Erase(index);
	}

	void ThumbListBox::SetThumbnailSize(uint32_t size)
	{
		GetReactor().GetModule().SetThumbnailSize(size);
	}

	bool ThumbListBox::IsEnabledMultiselection() const
	{
		return GetReactor().GetModule().IsEnabledMultiselection();
	}

	void ThumbListBox::EnableMultiselection(bool enabled)
	{
		auto& module = GetReactor().GetModule();
		if (module.EnableMultiselection(enabled))
		{
			//module.ClearSelection();
			module.Draw();
		}
	}

	std::vector<size_t> ThumbListBox::GetSelected() const
	{
		return GetReactor().GetModule().GetSelectedItems();
	}
}
