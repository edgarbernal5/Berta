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
	void ThumbListBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_module.Appearance = reinterpret_cast<ThumbListBoxAppearance*>(control.Handle()->Appearance.get());

		m_module.m_window = control.Handle();
		m_module.CalculateViewport(m_module.m_viewport);
	}

	void ThumbListBoxReactor::Update(Graphics& graphics)
	{
		BT_CORE_TRACE << "  - ThumbListBoxReactor::Update " << std::endl;
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();

		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->BoxBackground, true);

		if (m_module.m_viewport.ContentSize > static_cast<int>(m_module.m_viewport.BackgroundRect.Height))
		{
			if (!m_module.m_scrollBar)
			{
				auto scrollSize = window->ToScale(window->Appearance->ScrollBarSize);
				Rectangle scrollRect{ static_cast<int>(window->Size.Width - scrollSize) - 1, 1, scrollSize, window->Size.Height - 2u };

				m_module.m_scrollBar = std::make_unique<ScrollBar>(window, false, scrollRect);
				m_module.m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						m_module.m_state.m_offset = args.Value;

						m_control->Handle()->Renderer.Update();
						GUI::RefreshWindow(m_control->Handle());
					});
			}
			m_module.m_scrollBar->SetMinMax(0, m_module.m_viewport.ContentSize - static_cast<int>(m_module.m_viewport.BackgroundRect.Height));
			m_module.m_scrollBar->SetPageStepValue(m_module.m_viewport.BackgroundRect.Height);
			m_module.m_scrollBar->SetStepValue(m_module.m_viewport.CardSize.Height);

			//m_module.m_state.m_offset = m_module.m_scrollBar->GetValue();
		}
		else if (m_module.m_scrollBar)
		{
			m_module.m_state.m_offset = 0;
			m_module.m_scrollBar.reset();
		}
		Point offset{ m_module.m_viewport.BackgroundRect.X + (int)m_module.m_viewport.InnerMargin, m_module.m_viewport.BackgroundRect.Y + (int)m_module.m_viewport.InnerMargin - m_module.m_state.m_offset };

		auto thumbSize = m_module.m_window->ToScale(m_module.ThumbnailSize);
		auto cardHeight = m_module.m_window->ToScale(m_module.Appearance->ThumbnailCardHeight);
		for (size_t i = 0, k = 1; i < m_module.Items.size(); i++, ++k)
		{
			auto& item = m_module.Items[i];

			Rectangle cardRect{ offset.X + (int)m_module.m_viewport.CardMarginHalf, offset.Y, m_module.m_viewport.CardSize.Width, m_module.m_viewport.CardSize.Height };
			if (cardRect.Y <= static_cast<int>(m_module.m_viewport.BackgroundRect.Height + m_module.m_viewport.InnerMargin) && cardRect.Y + static_cast<int>(cardRect.Height) >= 0)
			{
				bool isSelected = item.IsSelected;
				bool isLastSelected = (int)i == m_module.m_mouseSelection.m_pressedIndex;
				//graphics.DrawRectangle(cardRect, isSelected ? window->Appearance->HighlightColor : window->Appearance->Background, true);
				graphics.DrawRectangle(cardRect, window->Appearance->Background, true);

				Size imageSize = window->ToScale(item.Thumbnail.GetSize());
				Size thumbFrameSize{ thumbSize, thumbSize };
				auto center = thumbFrameSize - imageSize;
				center *= 0.5f;

				Rectangle thumbnailRect{ cardRect.X + (int)center.Width, cardRect.Y + (int)center.Height, imageSize.Width, imageSize.Height };
				item.Thumbnail.Paste(graphics, thumbnailRect);

				{
					Size cardTextSize{ thumbSize, cardHeight };
					auto center = cardTextSize - graphics.GetTextExtent(item.Text);
					center *= 0.5f;

					if (isSelected)
					{
						graphics.DrawRectangle({ cardRect.X , cardRect.Y + (int)thumbSize ,cardRect.Width, cardHeight }, window->Appearance->HighlightColor, true);
					}
					graphics.DrawString({ cardRect.X + (int)center.Width, cardRect.Y + (int)thumbSize + (int)center.Height }, item.Text, isSelected ? window->Appearance->HighlightTextColor : window->Appearance->Foreground);
				}

				auto lineColor = enabled ? (isLastSelected ? window->Appearance->Foreground : (isSelected ? window->Appearance->BoxBorderHighlightColor : window->Appearance->BoxBorderColor)) : window->Appearance->BoxBorderDisabledColor;
				graphics.DrawRectangle(cardRect, lineColor, false);
				graphics.DrawLine({ cardRect.X, cardRect.Y + (int)thumbSize }, { cardRect.X + (int)m_module.m_viewport.CardSize.Width, cardRect.Y + (int)thumbSize }, lineColor);
			}

			offset.X += m_module.m_viewport.CardMargin + m_module.m_viewport.CardSize.Width + m_module.m_viewport.InnerMargin * 2u;
			if (k == m_module.m_viewport.TotalCardsInRow)
			{
				k = 0;
				offset.X = m_module.m_viewport.BackgroundRect.X + (int)m_module.m_viewport.InnerMargin;
				offset.Y += m_module.m_viewport.CardSize.Height + m_module.m_viewport.InnerMargin * 2u;
			}
		}

		if (m_module.m_mouseSelection.m_started && m_module.m_mouseSelection.m_startPosition != m_module.m_mouseSelection.m_endPosition)
		{
			Point startPoint{ 
				(std::min)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
				(std::min)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y) 
			};
			
			Point endPoint{ 
				(std::max)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
				(std::max)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y) 
			};

			Size boxSize{ (uint32_t)(endPoint.X - startPoint.X), (uint32_t)(endPoint.Y - startPoint.Y) };
			Color blendColor = window->Appearance->HighlightColor;
			Graphics selectionBox(boxSize);
			selectionBox.DrawRectangle(blendColor, true);
			selectionBox.DrawRectangle(window->Appearance->BoxBorderColor, false);

			Rectangle blendRect{ startPoint.X, startPoint.Y + m_module.m_state.m_offset, boxSize.Width, boxSize.Height};
			graphics.Blend(blendRect, selectionBox, { 0,0 }, 0.5f);
		}

		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);
	}

	void ThumbListBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.CalculateViewport(m_module.m_viewport);

		m_module.UpdateScrollBar();
		m_module.BuildGridCards();
		m_module.BuildItems();

		if (m_module.m_scrollBar)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			Rectangle scrollRect{ static_cast<int>(m_module.m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_module.m_window->Size.Height - 2u };
			GUI::MoveWindow(m_module.m_scrollBar->Handle(), scrollRect);
		}
	}

	void ThumbListBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		auto itemIndexAtPosition = m_module.GetItemIndexAtMousePosition(args.Position);
		bool hitOnBlank = itemIndexAtPosition == -1;

		bool needUpdate = false;
		m_module.m_mouseSelection.m_pressedIndex = itemIndexAtPosition;

		if (!m_module.m_multiselection)
		{
			if (hitOnBlank)
			{
				needUpdate = m_module.ClearSingleSelection();
			}
			else
			{
				needUpdate = m_module.UpdateSingleSelection(itemIndexAtPosition);
			}
		}
		else
		{
			if (hitOnBlank)
			{
				m_module.StartSelectionRectangle(args.Position);
				needUpdate = m_module.ClearSelectionIfNeeded();
			}
			else
			{
				needUpdate = m_module.HandleMultiSelection(itemIndexAtPosition, args);
			}
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ThumbListBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		bool needUpdate = false;

		if (m_module.m_mouseSelection.m_started)
		{
			auto logicalPosition = args.Position;
			logicalPosition.Y -= m_module.m_state.m_offset;
			m_module.m_mouseSelection.m_endPosition = logicalPosition;

			Point startPoint{
				(std::min)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
				(std::min)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y)
			};

			Point endPoint{
				(std::max)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
				(std::max)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y)
			};

			Size boxSize{ (uint32_t)(endPoint.X - startPoint.X), (uint32_t)(endPoint.Y - startPoint.Y) };

			needUpdate |= (boxSize.Width > 0 && boxSize.Height > 0);
			if ((boxSize.Width > 0 && boxSize.Height > 0))
			{
				Rectangle selectionRect{ startPoint.X, startPoint.Y + m_module.m_state.m_offset * 2, boxSize.Width, boxSize.Height};
				for (size_t i = 0; i < m_module.Items.size(); i++)
				{
					auto& item = m_module.Items[i];
					bool intersection = item.Bounds.Intersect(selectionRect);
					bool alreadySelected = std::find(m_module.m_mouseSelection.m_alreadySelected.begin(), m_module.m_mouseSelection.m_alreadySelected.end(), i) != m_module.m_mouseSelection.m_alreadySelected.end();

					if (m_module.m_mouseSelection.m_inverseSelection)
					{
						if (intersection && !alreadySelected || !intersection && alreadySelected)
						{
							item.IsSelected = true;
						}
						else if (intersection && alreadySelected || !intersection && !alreadySelected)
						{
							item.IsSelected = false;
						}
					}
					else
					{
						item.IsSelected = intersection || alreadySelected;
					}
				}
			}
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ThumbListBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		bool needUpdate = false;

		if (m_module.m_mouseSelection.m_started)
		{
			needUpdate = true;
			m_module.m_mouseSelection.m_started = false;
			m_module.m_mouseSelection.m_selections.clear();
			for (size_t i = 0; i < m_module.Items.size(); i++)
			{
				if (m_module.Items[i].IsSelected)
				{
					m_module.m_mouseSelection.m_selections.push_back(i);
				}
			}
			GUI::ReleaseCapture(m_module.m_window);
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ThumbListBoxReactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
	{
		if (!m_module.m_scrollBar)
		{
			return;
		}

		int direction = args.WheelDelta > 0 ? -1 : 1;
		direction *= m_module.m_scrollBar->GetStepValue();
		int newOffset = std::clamp(m_module.m_state.m_offset + direction, (int)m_module.m_scrollBar->GetMin(), (int)m_module.m_scrollBar->GetMax());

		if (newOffset != m_module.m_state.m_offset)
		{
			m_module.m_state.m_offset = newOffset;
			m_module.m_scrollBar->SetValue(m_module.m_state.m_offset);

			m_module.m_scrollBar->Handle()->Renderer.Update();
			GUI::MarkAsUpdated(m_module.m_scrollBar->Handle());

			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ThumbListBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		m_module.m_shiftPressed = m_module.m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_module.m_ctrlPressed = m_module.m_ctrlPressed || args.Key == KeyboardKey::Control;

		bool needUpdate = false;
		if (args.Key == KeyboardKey::ArrowLeft || args.Key == KeyboardKey::ArrowRight || args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::ArrowDown)
		{
			if (args.Key == KeyboardKey::ArrowLeft || args.Key == KeyboardKey::ArrowRight)
			{
				auto direction = args.Key == KeyboardKey::ArrowLeft ? -1 : 1;
				auto pivot = (m_module.m_mouseSelection.m_pressedIndex == -1 ? (direction == -1 ? (int)m_module.Items.size() : -1) : m_module.m_mouseSelection.m_pressedIndex);
				auto newItemIndex= pivot + direction;
				if (newItemIndex >= 0 && newItemIndex < (int)m_module.Items.size())
				{
					if (!m_module.m_multiselection || (!m_module.m_shiftPressed && !m_module.m_ctrlPressed))
					{
						m_module.ClearSelection();
					}
					if (m_module.m_shiftPressed || !m_module.m_ctrlPressed)
					{
						m_module.Items[newItemIndex].IsSelected = true;
						m_module.m_mouseSelection.m_pressedIndex = newItemIndex;
						m_module.m_mouseSelection.m_selections.push_back(newItemIndex);
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
					}
					else if (m_module.m_ctrlPressed)
					{
						m_module.m_mouseSelection.m_pressedIndex += direction;
					}
					m_module.EnsureVisibility(m_module.m_mouseSelection.m_pressedIndex);

					needUpdate = true;
				}
			}
			else if (args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::ArrowDown)
			{
				auto direction = args.Key == KeyboardKey::ArrowUp ? -1 : 1;
				auto pivot = (m_module.m_mouseSelection.m_pressedIndex == -1 ? (direction == -1 ? (int)m_module.Items.size() : -1) : m_module.m_mouseSelection.m_pressedIndex);
				auto newItemIndex = pivot + direction * m_module.m_viewport.TotalCardsInRow;
				if (newItemIndex >= 0 && newItemIndex < (int)m_module.Items.size())
				{
					if (!m_module.m_multiselection || !m_module.m_shiftPressed && !m_module.m_ctrlPressed)
					{
						m_module.ClearSelection();
					}

					if (m_module.m_multiselection && m_module.m_shiftPressed)
					{
						int end = newItemIndex + direction;
						int start = m_module.m_mouseSelection.m_pressedIndex;
						int current = start + direction;
						while (current != end)
						{

							m_module.Items[current].IsSelected = true;
							m_module.m_mouseSelection.m_selections.push_back(current);
							current += direction;
						}
						m_module.m_mouseSelection.m_pressedIndex = newItemIndex;
					}
					else if (m_module.m_ctrlPressed)
					{
						m_module.m_mouseSelection.m_pressedIndex = newItemIndex;
					}
					else
					{
						m_module.Items[newItemIndex].IsSelected = true;
						m_module.m_mouseSelection.m_selections.push_back(newItemIndex);
						m_module.m_mouseSelection.m_pressedIndex = newItemIndex;
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
					}
					m_module.EnsureVisibility(m_module.m_mouseSelection.m_pressedIndex);
					needUpdate = true;
				}
			}
		}
		else if (args.Key == KeyboardKey::Space && m_module.m_ctrlPressed)
		{
			if (m_module.m_mouseSelection.m_pressedIndex != -1)
			{
				auto& isSelected = m_module.Items[m_module.m_mouseSelection.m_pressedIndex].IsSelected;
				isSelected = !isSelected;
				if (isSelected)
				{
					m_module.m_mouseSelection.m_selections.push_back(m_module.m_mouseSelection.m_pressedIndex);
				}
				else {
					auto it = std::find(m_module.m_mouseSelection.m_selections.begin(), m_module.m_mouseSelection.m_selections.end(), m_module.m_mouseSelection.m_pressedIndex);
					if (it != m_module.m_mouseSelection.m_selections.end())
					{
						m_module.m_mouseSelection.m_selections.erase(it);
					}
				}
				needUpdate = true;
			}
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ThumbListBoxReactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
	{
		if (args.Key == KeyboardKey::Shift) m_module.m_shiftPressed = false;
		if (args.Key == KeyboardKey::Control) m_module.m_ctrlPressed = false;
	}

	void ThumbListBoxReactor::Module::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		auto& newItem = Items.emplace_back();
		newItem.Text = text;
		newItem.Thumbnail = thumbnail;

		BuildItems();
		BuildGridCards();
	}

	void ThumbListBoxReactor::Module::CalculateViewport(ViewportData& viewportData)
	{
		viewportData.BackgroundRect = m_window->Size.ToRectangle();
		viewportData.InnerMargin = m_window->ToScale(3u);

		viewportData.BackgroundRect.Width -= viewportData.InnerMargin * 2u;
		viewportData.BackgroundRect.Height -= viewportData.InnerMargin * 2u;
		viewportData.BackgroundRect.X = viewportData.InnerMargin;
		viewportData.BackgroundRect.Y = viewportData.InnerMargin;

		auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
		
		auto thumbSize = m_window->ToScale(ThumbnailSize);
		auto cardHeight = m_window->ToScale(Appearance->ThumbnailCardHeight);
		viewportData.CardSize = { thumbSize, thumbSize + cardHeight };
		
		uint32_t cardWidthWithMargin = viewportData.CardSize.Width + viewportData.InnerMargin * 2u;
		viewportData.TotalCardsInRow = viewportData.BackgroundRect.Width / cardWidthWithMargin;
		if (viewportData.TotalCardsInRow == 0)
		{
			viewportData.TotalCardsInRow = 1;
		}
		viewportData.TotalRows = (uint32_t)(Items.size()) / viewportData.TotalCardsInRow;
		if ((uint32_t)(Items.size()) % viewportData.TotalCardsInRow != 0)
		{
			++viewportData.TotalRows;
		}
		viewportData.ContentSize = static_cast<int>(viewportData.TotalRows * (viewportData.CardSize.Height + viewportData.InnerMargin * 2u));
		if (viewportData.ContentSize > static_cast<int>(viewportData.BackgroundRect.Height))
		{
			viewportData.BackgroundRect.Width -= scrollSize;
			viewportData.TotalCardsInRow = viewportData.BackgroundRect.Width / cardWidthWithMargin;
			if (viewportData.TotalCardsInRow == 0)
			{
				viewportData.TotalCardsInRow = 1;
			}
			
			viewportData.TotalRows = (uint32_t)(Items.size()) / viewportData.TotalCardsInRow;
			if ((uint32_t)(Items.size()) % viewportData.TotalCardsInRow != 0)
			{
				++viewportData.TotalRows;
			}
			viewportData.ContentSize = static_cast<int>(viewportData.TotalRows * (viewportData.CardSize.Height + viewportData.InnerMargin * 2u));
		}
		auto marginRemainder = viewportData.BackgroundRect.Width % cardWidthWithMargin;

		viewportData.CardMargin = marginRemainder / viewportData.TotalCardsInRow;
		viewportData.CardMarginHalf = viewportData.CardMargin >> 1;
	}

	void ThumbListBoxReactor::Module::BuildGridCards()
	{

	}

	void ThumbListBoxReactor::Module::Clear()
	{
	}

	void ThumbListBoxReactor::Module::SetThumbnailSize(uint32_t size)
	{
		if (ThumbnailSize == size)
		{
			return;
		}

		ThumbnailSize = size;

		UpdateScrollBar();
		BuildGridCards();
		BuildItems();

		m_window->Renderer.Update();
		GUI::RefreshWindow(m_window);
	}

	void ThumbListBoxReactor::Module::UpdateScrollBar()
	{
		bool needScrollBar = m_viewport.ContentSize > static_cast<int>(m_viewport.BackgroundRect.Height);
		if (!needScrollBar)
		{
			m_scrollBar.reset();
			m_state.m_offset = 0;
			return;
		}

		if (!m_scrollBar)
		{
			auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
			Rectangle scrollRect{ static_cast<int>(m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_window->Size.Height - 2u };

			m_scrollBar = std::make_unique<ScrollBar>(m_window, false, scrollRect);
			m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
				{
					m_state.m_offset = args.Value;

					m_window->Renderer.Update();
					GUI::RefreshWindow(m_window);
				});
		}
		m_scrollBar->SetMinMax(0, (int)(m_viewport.ContentSize - m_viewport.BackgroundRect.Height));
		m_scrollBar->SetPageStepValue(m_viewport.BackgroundRect.Height);
		m_scrollBar->SetStepValue(m_viewport.CardSize.Height);

		m_state.m_offset = m_scrollBar->GetValue();
	}

	void ThumbListBoxReactor::Module::OnMouseDown(const ArgMouse& args)
	{
		//m_mouseDownPosition = args.Position;
	}

	void ThumbListBoxReactor::Module::EnableMultiselection(bool enabled)
	{
		m_multiselection = enabled;
	}

	int ThumbListBoxReactor::Module::GetItemIndexAtMousePosition(const Point& position)
	{
		auto offsetPosition = position;
		offsetPosition.Y += m_state.m_offset;
		for (size_t i = 0; i < Items.size(); i++)
		{
			if (Items[i].Bounds.IsInside(offsetPosition))
			{
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	void ThumbListBoxReactor::Module::ClearSelection()
	{
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			Items[m_mouseSelection.m_selections[i]].IsSelected = false;
		}
		m_mouseSelection.m_selections.clear();
	}

	bool ThumbListBoxReactor::Module::ClearSingleSelection()
	{
		if (m_mouseSelection.m_selectedIndex != -1)
		{
			Items[m_mouseSelection.m_selectedIndex].IsSelected = false;
			m_mouseSelection.m_selections.clear();
			m_mouseSelection.m_selectedIndex = -1;
			return true;
		}
		return false;
	}

	bool ThumbListBoxReactor::Module::UpdateSingleSelection(int newItemIndex)
	{
		bool needUpdate = (m_mouseSelection.m_selectedIndex != newItemIndex);
		if (needUpdate)
		{
			ClearSingleSelection();
			SelectItem(newItemIndex);
		}
		return needUpdate;
	}

	void ThumbListBoxReactor::Module::SelectItem(int index)
	{
		Items[index].IsSelected = true;
		m_mouseSelection.m_selections.push_back(index);
		m_mouseSelection.m_selectedIndex = index;
		EnsureVisibility(index);
	}

	void ThumbListBoxReactor::Module::StartSelectionRectangle(const Point& mousePosition)
	{
		auto logicalPosition = mousePosition;
		logicalPosition.Y -= m_state.m_offset;
		m_mouseSelection.m_started = true;
		m_mouseSelection.m_startPosition = logicalPosition;
		m_mouseSelection.m_endPosition = logicalPosition;

		m_mouseSelection.m_selections.clear();
		m_mouseSelection.m_alreadySelected.clear();

		for (size_t i = 0; i < Items.size(); i++)
		{
			if (Items[i].IsSelected)
			{
				m_mouseSelection.m_selections.push_back(i);
				m_mouseSelection.m_alreadySelected.push_back(i);
			}
		}

		GUI::Capture(m_window);
	}

	bool ThumbListBoxReactor::Module::ClearSelectionIfNeeded()
	{
		if (!m_ctrlPressed && !m_shiftPressed)
		{
			if (!m_mouseSelection.m_selections.empty())
			{
				for (const auto& index : m_mouseSelection.m_selections)
				{
					Items[index].IsSelected = false;
				}
				m_mouseSelection.m_selections.clear();
				m_mouseSelection.m_alreadySelected.clear();
				return true;
			}
		}
		return false;
	}

	void ThumbListBoxReactor::Module::ToggleItemSelection(int itemIndexAtPosition)
	{
		auto& item = Items[itemIndexAtPosition];
		item.IsSelected = !item.IsSelected;

		if (item.IsSelected)
		{
			m_mouseSelection.m_selections.push_back(itemIndexAtPosition);
		}
		else
		{
			auto it = std::remove(m_mouseSelection.m_selections.begin(), m_mouseSelection.m_selections.end(), itemIndexAtPosition);
			m_mouseSelection.m_selections.erase(it, m_mouseSelection.m_selections.end());
		}
	}

	void ThumbListBoxReactor::Module::PerformRangeSelection(int itemIndexAtPosition)
	{
		int minIndex = (std::min)(m_mouseSelection.m_selectedIndex, itemIndexAtPosition);
		int maxIndex = (std::max)(m_mouseSelection.m_selectedIndex, itemIndexAtPosition);
		for (int i = minIndex; i <= maxIndex; ++i)
		{
			Items[i].IsSelected = true;
			if (std::find(m_mouseSelection.m_selections.begin(), m_mouseSelection.m_selections.end(), i) == m_mouseSelection.m_selections.end())
			{
				m_mouseSelection.m_selections.push_back(i);
			}
		}
	}

	bool ThumbListBoxReactor::Module::HandleMultiSelection(int itemIndexAtPosition, const ArgMouse& args)
	{
		bool needUpdate = false;

		if (!m_ctrlPressed && !m_shiftPressed)
		{
			if (!args.ButtonState.RightButton || !Items[itemIndexAtPosition].IsSelected)
			{
				ClearSelection();
			}
			if (!Items[itemIndexAtPosition].IsSelected)
			{
				SelectItem(itemIndexAtPosition);
				needUpdate = true;
			}
		}
		else if (m_shiftPressed && m_mouseSelection.m_pressedIndex != -1)
		{
			PerformRangeSelection(itemIndexAtPosition);
			needUpdate = true;
		}
		else
		{
			ToggleItemSelection(itemIndexAtPosition);
			needUpdate = true;
		}

		EnsureVisibility(itemIndexAtPosition);
		return needUpdate;
	}

	std::vector<size_t> ThumbListBoxReactor::Module::GetSelectedItems() const
	{
		return m_mouseSelection.m_selections;
	}

	void ThumbListBoxReactor::Module::EnsureVisibility(int lastSelectedIndex)
	{
		if (!m_scrollBar)
		{
			return;
		}

		auto itemBounds = Items[lastSelectedIndex].Bounds;
		itemBounds.Y -= m_state.m_offset;

		if (m_viewport.BackgroundRect.Contains(itemBounds))
		{
			return;
		}

		int itemHeight = static_cast<int>(itemBounds.Height);
		int viewportHeight = static_cast<int>(m_viewport.BackgroundRect.Height);
		int innerMarginDouble = static_cast<int>(m_viewport.InnerMargin * 2u);
		auto offsetAdjustment = 0;
		if (itemBounds.Y >= viewportHeight)
		{
			offsetAdjustment = itemHeight + innerMarginDouble;
		}
		else if (itemBounds.Y + itemHeight <= 0)
		{
			offsetAdjustment = -itemHeight - innerMarginDouble;
		}
		else if (itemBounds.Y + itemHeight > 0 && itemBounds.Y + itemHeight < viewportHeight)
		{
			offsetAdjustment = (itemBounds.Y + itemHeight) - itemHeight - innerMarginDouble;
		}
		else
		{
			offsetAdjustment = (itemBounds.Y + itemHeight) - viewportHeight;
		}
		m_state.m_offset = std::clamp(m_state.m_offset + offsetAdjustment, m_scrollBar->GetMin(), m_scrollBar->GetMax());
		m_scrollBar->SetValue(m_state.m_offset);

		m_scrollBar->Handle()->Renderer.Update();
		GUI::RefreshWindow(m_scrollBar->Handle());
	}

	void ThumbListBoxReactor::Module::BuildItems()
	{
		Point offset{ m_viewport.BackgroundRect.X + (int)m_viewport.InnerMargin, m_viewport.BackgroundRect.Y + (int)m_viewport.InnerMargin };
		for (size_t i = 0, k = 1; i < Items.size(); i++, ++k)
		{
			auto& item = Items[i];

			item.Bounds = { offset.X + (int)m_viewport.CardMarginHalf, offset.Y, m_viewport.CardSize.Width, m_viewport.CardSize.Height };

			offset.X += m_viewport.CardMargin + m_viewport.CardSize.Width + m_viewport.InnerMargin * 2u;
			if (k == m_viewport.TotalCardsInRow)
			{
				k = 0;
				offset.X = m_viewport.BackgroundRect.X + (int)m_viewport.InnerMargin;
				offset.Y += m_viewport.CardSize.Height + m_viewport.InnerMargin * 2u;
			}
		}
	}

	ThumbListBox::ThumbListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "ThumbListBox";
#endif
	}

	void ThumbListBox::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		m_reactor.GetModule().AddItem(text, thumbnail);
	}

	void ThumbListBox::Clear()
	{
		m_reactor.GetModule().Clear();
	}

	void ThumbListBox::SetThumbnailSize(uint32_t size)
	{
		m_reactor.GetModule().SetThumbnailSize(size);
	}

	void ThumbListBox::EnableMultiselection(bool enabled)
	{
		m_reactor.GetModule().EnableMultiselection(enabled);
	}

	std::vector<size_t> ThumbListBox::GetSelected() const
	{
		return m_reactor.GetModule().GetSelectedItems();
	}
}
