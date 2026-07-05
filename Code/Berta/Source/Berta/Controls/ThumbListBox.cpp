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
	void ThumbListBoxReactor::DoOnInit()
	{
		m_module.m_window = m_control->Handle();
		m_module.m_control = m_control;
		GUI::SetWindowBorderless(*m_control, false);
		
		m_module.m_events = reinterpret_cast<ThumbListBoxEvents*>(m_control->Handle()->Events.get());
		
		m_module.InitScrollableView();
		
		m_module.m_rangeResolver = [](const size_t& anchor, const size_t& current)
		{
			std::vector<size_t> range;
			size_t start = std::min<size_t>(anchor, current);
			size_t end = std::max<size_t>(anchor, current);
			for (size_t i = start; i <= end; ++i)
			{
				range.emplace_back(i);
			}
			return range;
		};
	}

	void ThumbListBoxReactor::Update(Graphics& graphics)
	{
		auto appearance = reinterpret_cast<ThumbListBoxAppearance*>(m_control->Handle()->Appearance.get());
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		Rectangle globalRect = window->ClientSize.ToRectangle();
		graphics.FillRectangle(globalRect, window->Appearance->BoxBackground);

		Rectangle clientArea = m_module.m_scrollableView->GetClientArea(); //m_module.m_control->GetClientArea();
		if (clientArea.Width == 0 || clientArea.Height == 0) return;
		if (m_module.m_items.empty() || !m_module.m_scrollableView) return;
	
		auto scrollOffset = m_module.m_scrollableView->GetScrollOffset();
		Point offset{ clientArea.X - scrollOffset.X, clientArea.Y - scrollOffset.Y };

		auto firstBounds = m_module.GetItemBounds(0);
		int cardWidth = static_cast<int>(firstBounds.Width);
		int cardHeight = static_cast<int>(firstBounds.Height);
		if (cardWidth == 0 || cardHeight == 0) return;

		int availableWidth = static_cast<int>(clientArea.Width);
		int columns = std::max<int>(1, availableWidth / cardWidth);

		int gapY = window->ToScale(10);
		int totalRowHeight = cardHeight + gapY; 
	
		int firstVisibleRow = std::max<int>(0, scrollOffset.Y / totalRowHeight);
		int visibleRows = (static_cast<int>(clientArea.Height) + totalRowHeight - 1) / totalRowHeight + 2;
		
		size_t startIndex = static_cast<size_t>(firstVisibleRow) * static_cast<size_t>(columns);
		if (startIndex >= m_module.m_items.size())
		{
			return;
		}

		auto thumbSizeScale = window->ToScale(m_module.m_thumbnailSize);
		Size thumbFrameSize{ thumbSizeScale, thumbSizeScale };
		size_t endIndex = std::min<size_t>(m_module.m_items.size(), startIndex + static_cast<size_t>(visibleRows * columns));
		
		for (size_t i = startIndex; i < endIndex; i++)
		{
			auto& item = m_module.m_items[i];
			Rectangle cardRect = m_module.GetItemBounds(i);
			
			cardRect.X += offset.X;
			cardRect.Y += offset.Y;

			Rectangle thumbnailRect = { cardRect.X, cardRect.Y, thumbFrameSize.Width, thumbFrameSize.Height };
		
			const bool& isSelected = m_module.m_selectionController.IsSelected(i);
			bool isFocused = m_module.m_focusedIndex == i;
			bool isHovered = (i == m_module.m_hoveredIndex);
			
			Color backColor = window->Appearance->ButtonBackground;
			if (isSelected)
			{
				backColor = window->Appearance->HighlightColor;
			}
			else if (isHovered)
			{
				backColor = window->Appearance->ButtonHighlightBackground;
			}
			
			if (clientArea.Intersects(cardRect))
			{
				graphics.FillRectangle(cardRect, backColor);
			}
			
			if (clientArea.Intersects(thumbnailRect))
			{
				graphics.FillRectangle(thumbnailRect, window->Appearance->Background);
			}

			Image cachedImage;
			if (item.m_hasThumbnail && m_module.m_imageCache.TryGet(item.m_id, cachedImage))
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

				if (clientArea.Intersects(thumbnailImageRect))
				{
					cachedImage.Paste(graphics, thumbnailImageRect);
				}
			}
		
			if (isSelected)
			{
				graphics.FillRectangle({ cardRect.X , cardRect.Y + (int)thumbSizeScale, cardRect.Width, cardHeight - thumbSizeScale }, window->Appearance->HighlightColor);
			}
			m_module.DrawItemText(graphics, item, { cardRect.X, cardRect.Y + (int)thumbSizeScale, cardRect.Width, cardRect.Height });
		
			auto lineColor = enabled ? (isFocused ? window->Appearance->Foreground : (isSelected ? window->Appearance->BoxBorderHighlightColor : window->Appearance->BoxBorderColor)) : window->Appearance->BoxBorderDisabledColor;
			graphics.DrawRectangle(cardRect, lineColor);
			graphics.DrawLine({ cardRect.X, cardRect.Y + (int)thumbSizeScale }, { cardRect.X + (int)cardRect.Width - 1, cardRect.Y + (int)thumbSizeScale }, lineColor);
		}
	
		if (m_module.m_lassoSelection.IsActive())
		{
			Rectangle lassoRect = m_module.m_lassoSelection.GetRect();
			if (lassoRect.Width > 0 && lassoRect.Height > 0)
			{
				Graphics selectionBox(m_module.m_lassoSelection.GetRect(), m_module.m_window->DPI, m_module.m_window->RootPaintHandle);
				m_module.m_lassoSelection.Draw(graphics, selectionBox, appearance->SelectionHighlightColor, appearance->SelectionBorderHighlightColor);
			}
		}

		graphics.DrawRectangle(window->ClientSize.ToRectangle(), enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor);
	}

	void ThumbListBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.UpdateScrollMetrics();
		m_module.TriggerVisibilityEvent();
	}

	void ThumbListBoxReactor::DblClick(Graphics& graphics, const ArgMouse& args)
	{
		if (!args.ButtonState.LeftButton)
		{
			return;
		}
		
		auto clickedIndex = m_module.GetItemIndexAtMousePosition(args.Position);
		if (clickedIndex.has_value())
		{
			if (m_module.m_events)
			{
				ArgThumbListBoxItem arguments { clickedIndex.value() };
				m_module.m_events->ItemDblClick.Emit(arguments);
			}
		}
	}

	void ThumbListBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (!args.ButtonState.LeftButton)
		{
			return;
		}

		auto clickedIndex = m_module.GetItemIndexAtMousePosition(args.Position);
		if (clickedIndex.has_value())
		{
			m_module.m_focusedIndex = clickedIndex;
			
			bool selectionChanged = m_module.m_selectionController.Select
			(
				clickedIndex.value(), 
				args.CtrlPressed, 
				args.ShiftPressed, 
				m_module.m_rangeResolver
			);

			if (selectionChanged && m_module.m_events)
			{
				m_module.TriggerSelectionChanged();
			}
		}
		else
		{
			if (!args.CtrlPressed && !args.ShiftPressed)
			{
				m_module.m_selectionController.Clear();
				if (m_module.m_events)
				{
					m_module.TriggerSelectionChanged();
				}
			}
			if (m_module.m_selectionController.IsMultiSelect())
			{
				GUI::Capture(m_module.m_window);
				m_module.m_selectionController.SaveSnapshot();
				m_module.m_lassoSelection.Start(args.Position);
			}
		}
		
		GUI::MarkAsNeedUpdate(*m_control);
	}

	void ThumbListBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_lassoSelection.IsActive())
		{
			if (m_module.m_lassoSelection.Update(args.Position))
			{
				Rectangle newLassoRect = m_module.m_lassoSelection.GetRect();
				auto scrollOffset = m_module.m_scrollableView->GetScrollOffset();

				Rectangle lassoWorldRect =
				{ 
					newLassoRect.X + scrollOffset.X, 
					newLassoRect.Y + scrollOffset.Y, 
					newLassoRect.Width, 
					newLassoRect.Height 
				};

				auto firstBounds = m_module.GetItemBounds(0);
				if (firstBounds.Width > 0 && firstBounds.Height > 0)
				{
					int availableWidth = (int)m_module.m_scrollableView->GetClientArea().Width;

					int cardWidth = firstBounds.Width;
					int cardHeight = firstBounds.Height;
					int columns = std::max<int>(1, availableWidth / cardWidth);
					
					int gapY = m_module.m_window->ToScale(10);
					int remainingWidth = availableWidth - (columns * cardWidth);
					int gapX = remainingWidth / (columns + 1);

					int totalCellWidth = cardWidth + gapX;
					int totalCellHeight = cardHeight + gapY;

					int startCol = std::max<int>(0, (lassoWorldRect.X - gapX) / totalCellWidth);
					int endCol = std::min<int>(columns - 1, (lassoWorldRect.X + static_cast<int>(lassoWorldRect.Width)) / totalCellWidth);
					
					int startRow = std::max<int>(0, (lassoWorldRect.Y - gapY) / totalCellHeight);
					int endRow = (lassoWorldRect.Y + static_cast<int>(lassoWorldRect.Height)) / totalCellHeight;

					if (!args.CtrlPressed)
					{
						m_module.m_selectionController.Clear();
					}

					for (int row = startRow; row <= endRow; ++row) 
					{
						for (int col = startCol; col <= endCol; ++col) 
						{
							size_t index = (row * columns) + col;
							if (index < m_module.m_items.size()) 
							{
								Rectangle itemRect = m_module.GetItemBounds(index);
								if (lassoWorldRect.Intersects(itemRect)) 
								{
									m_module.m_selectionController.SetSelected(index, true);
									
									GUI::MarkAsNeedUpdate(*m_control, &itemRect);
								}
							}
						}
					}
				}
				
				GUI::MarkAsNeedUpdate(*m_control); 
			}
			return;
		}

		if (args.ButtonState.NoButtonsPressed())
		{
			auto currentIndex = m_module.GetItemIndexAtMousePosition(args.Position);
			if (currentIndex != m_module.m_hoveredIndex)
			{
				m_module.m_hoveredIndex = currentIndex;
				GUI::MarkAsNeedUpdate(*m_control);
			}
		}
	}

	void ThumbListBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_lassoSelection.IsActive())
		{
			bool actuallyChanged = m_module.m_selectionController.HasSelectionChangedSinceSnapshot();
			
			m_module.m_lassoSelection.End();
			m_module.m_selectionController.ClearSnapshot();
			GUI::MarkAsNeedUpdate(*m_control);
			
			if (actuallyChanged)
			{
				m_module.TriggerSelectionChanged();
			}
			GUI::ReleaseCapture(m_module.m_window);
		}
	}

	void ThumbListBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_hoveredIndex.has_value())
		{
			m_module.m_hoveredIndex.reset();
			GUI::MarkAsNeedUpdate(*m_control);
		}
	}

	void ThumbListBoxReactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
	{
		if (m_module.m_scrollableView)
		{
			m_module.m_scrollableView->HandleMouseWheel(args);
		}
	}

	void ThumbListBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		if (m_module.m_items.empty() || !m_module.m_scrollableView)
		{
			return;
		}
		
		auto firstBounds = m_module.GetItemBounds(0);
		if (firstBounds.Width == 0)
		{
			return;
		}
		
		int availableWidth = m_module.GetLayoutWidth();
		int columns = std::max<int>(1, static_cast<int>(availableWidth / firstBounds.Width));

		size_t currentIndex = m_module.m_focusedIndex.value_or(0);
		size_t newIndex = currentIndex;

		switch (args.Key)
		{
			case KeyboardKey::ArrowRight: 
				if (currentIndex + 1 < m_module.m_items.size())
				{
					newIndex = currentIndex + 1;
				}
				break;

			case KeyboardKey::ArrowLeft:
				if (currentIndex > 0)
				{
					newIndex = currentIndex - 1;
				}
				break;

			case KeyboardKey::ArrowDown:
				if (currentIndex + columns < m_module.m_items.size())
				{
					newIndex = currentIndex + columns;
				}
				else
				{
					newIndex = m_module.m_items.size() - 1;
				}
				break;

			case KeyboardKey::ArrowUp:
				if (currentIndex >= static_cast<size_t>(columns))
				{
					newIndex = currentIndex - columns;
				}
				else 
				{
					newIndex = 0;
				}
				break;
			
			case KeyboardKey::Space:
				m_module.m_focusedIndex = newIndex;
				m_module.m_selectionController.Select(newIndex, args.ButtonState.Ctrl, args.ButtonState.Shift, m_module.m_rangeResolver);
			
				break;

			default:
				return;
		}

		if (newIndex != currentIndex)
		{
			m_module.m_focusedIndex = newIndex;
			if (!args.ButtonState.Ctrl && !args.ButtonState.Shift)
			{
				m_module.m_selectionController.Clear();
			}
			if (!args.ButtonState.Ctrl)
			{
				bool selectionChanged = m_module.m_selectionController.Select
				(
					newIndex, 
					args.ButtonState.Ctrl, 
					args.ButtonState.Shift, 
					m_module.m_rangeResolver
				);
				if (selectionChanged)
				{
					GUI::MarkAsNeedUpdate(m_module.m_window);
				}
			}
			
			Rectangle targetBounds = m_module.GetItemBounds(newIndex);
			if (m_module.m_scrollableView->EnsureVisibility(targetBounds))
			{
				m_module.TriggerVisibilityEvent();
				GUI::MarkAsNeedUpdate(m_module.m_window);
			}

			if (m_module.m_events)
			{
				//m_module.m_events->Selected.Fire({ newIndex });
			}

			GUI::MarkAsNeedUpdate(*m_control);
		}
	}

	ThumbListBoxItem ThumbListBoxReactor::Module::At(size_t index)
	{
		if (index <= m_items.size())
		{
			return ThumbListBoxItem{ index, this };
		}
		return { 0, nullptr };
	}

	bool ThumbListBoxReactor::Module::AddItem(const std::wstring& text)
	{
		auto& newItem = m_items.emplace_back();
		newItem.m_id = m_idCounter++;
		newItem.m_text = text;

		UpdateScrollMetrics();

		return true;
	}

	bool ThumbListBoxReactor::Module::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		auto& newItem = m_items.emplace_back();
		newItem.m_id = m_idCounter++;
		newItem.m_text = text;
		newItem.m_hasThumbnail = true;

		m_imageCache.Put(newItem.m_id, thumbnail);

		UpdateScrollMetrics();

		return true;
	}

	bool ThumbListBoxReactor::Module::Clear()
	{
		bool needUpdate = !m_items.empty();
		m_items.clear();
		m_imageCache.Clear();
		m_selectionController.Clear();

		m_lastVisibleStart = 0;
		m_lastVisibleEnd = 0;
		
		m_focusedIndex = std::nullopt;
		m_hoveredIndex = std::nullopt;
		
		UpdateScrollMetrics();
		
		return needUpdate;
	}

	void ThumbListBoxReactor::Module::Erase(size_t index)
	{
		if (index >= m_items.size())
		{
			return;
		}
		m_imageCache.Erase(m_items[index].m_id);
		m_items.erase(m_items.begin() + index);
		
		m_selectionController.Clear();
		m_focusedIndex = std::nullopt;
		
		UpdateScrollMetrics();
		TriggerVisibilityEvent();
		
		GUI::MarkAsNeedUpdate(m_window);
	}

	void ThumbListBoxReactor::Module::SetThumbnailSize(uint32_t size)
	{
		if (m_thumbnailSize == size)
		{
			return;
		}

		m_thumbnailSize = size;
		UpdateScrollMetrics();

		TriggerVisibilityEvent();
		
		GUI::MarkAsNeedUpdate(m_window);
	}

	bool ThumbListBoxReactor::Module::IsEnabledMultiselection() const
	{
		return m_selectionController.IsMultiSelect();
	}

	bool ThumbListBoxReactor::Module::EnableMultiselection(bool enabled)
	{
		bool needUpdate = m_selectionController.IsMultiSelect() != enabled;
		m_selectionController.SetMultiSelect(enabled);
		
		return needUpdate;
	}

	std::optional<size_t> ThumbListBoxReactor::Module::GetItemIndexAtMousePosition(const Point& position)
	{
		auto appearance = reinterpret_cast<ThumbListBoxAppearance*>(m_control->Handle()->Appearance.get());
		
		auto scrollOffset = m_scrollableView->GetScrollOffset();
		Point absolutePt = { position.X + scrollOffset.X, position.Y + scrollOffset.Y };
		
		uint32_t thumbSizeScale = m_window->ToScale(m_thumbnailSize);
		uint32_t textHeightScale = m_window->ToScale(appearance->ThumbnailCardHeight);
		int cardWidth = thumbSizeScale;
		int cardHeight = thumbSizeScale + textHeightScale;
		
		int clientWidth = GetLayoutWidth();
		
		int gapY = m_window->ToScale(10);
		int columns = std::max<int>(1, clientWidth / cardWidth);
		
		int remainingWidth = clientWidth - (columns * cardWidth);
		int gapX = remainingWidth / (columns + 1);
		if (absolutePt.X < gapX || absolutePt.Y < gapY) return -1;
		
		int col = (absolutePt.X - gapX) / (cardWidth + gapX);
		int row = (absolutePt.Y - gapY) / (cardHeight + gapY);

		if (col >= columns) return -1;
		
		int itemStartX = gapX + col * (cardWidth + gapX);
		int itemStartY = gapY + row * (cardHeight + gapY);

		if (absolutePt.X > itemStartX + cardWidth || absolutePt.Y > itemStartY + cardHeight)
		{
			return std::nullopt; // Clic en el espacio entre elementos
		}
		
		int index = (row * columns) + col;
		if (index >= 0 && index < static_cast<int>(m_items.size()))
		{
			return index;
		}
		return std::nullopt;
	}

	int ThumbListBoxReactor::Module::GetLayoutWidth() const
	{
		if (m_scrollableView)
		{
			int width = static_cast<int>(m_scrollableView->GetClientArea().Width);
			if (width > 0)
			{
				return width;
			}
		}

		return static_cast<int>(m_window->ClientSize.Width);
	}

	Rectangle ThumbListBoxReactor::Module::GetItemBounds(size_t index, int overrideWidth) const
	{
		auto appearance = reinterpret_cast<ThumbListBoxAppearance*>(m_control->Handle()->Appearance.get());
		
		uint32_t thumbSizeScale = m_window->ToScale(m_thumbnailSize);
		uint32_t textHeightScale = m_window->ToScale(appearance->ThumbnailCardHeight);
		
		uint32_t cardWidth = thumbSizeScale;
		uint32_t cardHeight = thumbSizeScale + textHeightScale;
		
		int clientWidth = (overrideWidth == -1) ? GetLayoutWidth() : overrideWidth;

		if (cardWidth == 0 || clientWidth <= 0) return {};

		int gapY = m_window->ToScale(10); 
		int columns = std::max<int>(1, clientWidth / static_cast<int>(cardWidth));
		
		int remainingWidth = clientWidth - (columns * static_cast<int>(cardWidth));
		int gapX = remainingWidth / (columns + 1); 

		int row = (int)index / columns;
		int col = (int)index % columns;
		
		int x = gapX + col * (cardWidth + gapX);
		int y = gapY + row * (cardHeight + gapY);

		return { x, y, cardWidth, cardHeight };
	}

	void ThumbListBoxReactor::Module::TriggerVisibilityEvent()
	{
		if (!m_events || !m_scrollableView || m_items.empty()) return;

		auto clientArea = m_control->GetClientArea();
		if (clientArea.Width <= 1 || clientArea.Height <= 1) return;

		auto scrollOffset = m_scrollableView->GetScrollOffset();
		auto firstBounds = GetItemBounds(0);
		if (firstBounds.Width == 0 || firstBounds.Height == 0) return;
		
		int availableWidth = (int)m_scrollableView->GetClientArea().Width;
		if (availableWidth <= 0) 
		{
			availableWidth = (int)clientArea.Width;
		}
		
		int cardWidth = (int)firstBounds.Width;
		int cardHeight = (int)firstBounds.Height;
		int columns = std::max<int>(1, availableWidth / cardWidth);
		
		int gapY = m_window->ToScale(10);
		int totalRowHeight = cardHeight + gapY;

		int firstVisibleRow = std::max<int>(0, scrollOffset.Y / totalRowHeight);
		//int visibleRows = static_cast<int>(std::ceil(clientArea.Height / static_cast<float>(totalRowHeight))) + 2;
		int visibleRows = static_cast<int>(std::ceil(clientArea.Height / static_cast<float>(totalRowHeight)));

		size_t currentStart = static_cast<size_t>(firstVisibleRow) * columns;
		size_t currentEnd = std::min<size_t>(m_items.size(), currentStart + static_cast<size_t>(visibleRows * columns));

		if (currentStart == m_lastVisibleStart && currentEnd == m_lastVisibleEnd)
		{
			return;
		}
		
		for (size_t i = m_lastVisibleStart; i < m_lastVisibleEnd; ++i)
		{
			if (i < currentStart || i >= currentEnd)
			{
				ArgThumbListBoxItemVisibility arguments{ i, false };
				BT_CORE_DEBUG << " - visibility item = " << i << ". visible=false. "<< std::endl;
				m_events->ItemVisibility.Emit(arguments);
			}
		}
		
		for (size_t i = currentStart; i < currentEnd; ++i)
		{
			if (i < m_lastVisibleStart || i >= m_lastVisibleEnd)
			{
				ArgThumbListBoxItemVisibility arguments{ i, true };
				BT_CORE_DEBUG << " - visibility item = " << i << ". visible=true" << std::endl;
				m_events->ItemVisibility.Emit(arguments);
			}
		}

		m_lastVisibleStart = currentStart;
		m_lastVisibleEnd = currentEnd;
	}

	void ThumbListBoxReactor::Module::TriggerSelectionChanged()
	{
		ArgThumbListBox arguments { GetSelectedItems() };
		auto events = reinterpret_cast<ThumbListBoxEvents*>(m_window->Events.get());
		events->SelectionChanged.Emit(arguments);
	}

	void ThumbListBoxReactor::Module::Draw() const
	{
		GUI::MarkAsNeedUpdate(m_window);
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

	std::vector<ThumbListBoxItem> ThumbListBoxReactor::Module::GetSelectedItems()
	{
		auto selectedItems = m_selectionController.GetSelectedItems();
		std::vector<ThumbListBoxItem> result;
		result.reserve(selectedItems.size());
		
		for (auto& item : selectedItems)
		{
			result.emplace_back(item, this);
		}
		return result;
	}

	void ThumbListBoxReactor::Module::InitScrollableView()
	{
		m_scrollableView = std::make_unique<ScrollableView>(m_window);
		
		m_scrollableView->SetScrollStep(20, 20);
		m_scrollableView->SetOnScrollChange([this]()
		{
			TriggerVisibilityEvent();
			GUI::MarkAsNeedUpdate(m_window);
		});
	}
	
	void ThumbListBoxReactor::Module::EnsureVisibility(size_t index)
	{
		if (m_items.empty() || !m_scrollableView || index >= m_items.size()) 
		{
			return;
		}
		
		Rectangle targetBounds = GetItemBounds(index);
		if (m_scrollableView->EnsureVisibility(targetBounds))
		{
			TriggerVisibilityEvent();
			
			GUI::MarkAsNeedUpdate(*m_control);
		}
	}

	void ThumbListBoxReactor::Module::UpdateScrollMetrics()
	{
		if (!m_scrollableView || m_items.empty())
		{
			if (m_scrollableView)
			{
				m_scrollableView->SetContentSize({ m_window->ClientSize.Width, 0 });	
			}
			return;
		}
		
		auto clientArea = m_control->GetClientArea();
		m_scrollableView->SetViewRect(clientArea);
		
		int currentWidth = static_cast<int>(clientArea.Width);
		auto lastItemBounds = GetItemBounds(m_items.size() - 1, currentWidth);
		
		int gapY = m_window->ToScale(10);
		uint32_t totalHeight = lastItemBounds.Y + lastItemBounds.Height + gapY;
		m_scrollableView->SetContentSize({ static_cast<uint32_t>(currentWidth), totalHeight });

		int newWidth = GetLayoutWidth();
		if (newWidth != currentWidth)
		{
			lastItemBounds = GetItemBounds(m_items.size() - 1, newWidth);
			totalHeight = lastItemBounds.Y + lastItemBounds.Height + gapY;
			
			m_scrollableView->SetContentSize({ static_cast<uint32_t>(newWidth), totalHeight });
		}
	}
	
	void ThumbListBoxItem::SetText(const std::wstring& text)
	{
		if (m_logicalIndex >= m_module->m_items.size())
		{
			return;
		}

		m_module->m_items[m_logicalIndex].m_text = text;
		
		Rectangle itemBounds = m_module->GetItemBounds(m_logicalIndex);
		GUI::MarkAsNeedUpdate(m_module->m_window, &itemBounds);
	}

	void ThumbListBoxItem::SetIcon(const Image& image)
	{
		if (m_logicalIndex >= m_module->m_items.size())
		{
			return;
		}

		auto& item = m_module->m_items[m_logicalIndex];
		item.m_hasThumbnail = image;
		if (item.m_hasThumbnail)
		{
			m_module->m_imageCache.Put(item.m_id, image);
		}
		else
		{
			m_module->m_imageCache.Erase(item.m_id);
		}
		
		Rectangle itemBounds = m_module->GetItemBounds(m_logicalIndex);
		GUI::MarkAsNeedUpdate(m_module->m_window, &itemBounds);
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
		auto& module = GetReactor().GetModule();
		if (module.AddItem(text) && IsAutoDraw())
		{
			module.Draw();
		}
	}

	void ThumbListBox::AddItem(const std::string& text)
	{
		auto& module = GetReactor().GetModule();
		if (module.AddItem(StringUtils::UTF8ToWide(text)) && IsAutoDraw())
		{
			module.Draw();
		}
	}

	void ThumbListBox::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		auto& module = GetReactor().GetModule();
		if (module.AddItem(text, thumbnail) && IsAutoDraw())
		{
			module.Draw();
		}
	}

	void ThumbListBox::AddItem(const std::string& text, const Image& thumbnail)
	{
		auto& module = GetReactor().GetModule();
		if (module.AddItem(StringUtils::UTF8ToWide(text), thumbnail) && IsAutoDraw())
		{
			module.Draw();
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
			module.m_selectionController.Clear();
			module.Draw();
		}
	}

	std::vector<ThumbListBoxItem> ThumbListBox::GetSelected()
	{
		return GetReactor().GetModule().GetSelectedItems();
	}

	void ThumbListBox::SetCacheCapacity(size_t maxImages)
	{
		auto& module = GetReactor().GetModule();
		module.m_imageCache.SetCapacity(maxImages);
		if (IsAutoDraw())
		{
			module.Draw();
		}
	}

	void ThumbListBox::ScrollTo(size_t index)
	{
		GetReactor().GetModule().EnsureVisibility(index);
	}
}
