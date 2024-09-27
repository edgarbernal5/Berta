/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ThumbListBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/ControlAppearance.h"

namespace Berta
{

	void ThumbListBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_module.Appearance = reinterpret_cast<ThumbListBoxAppearance*>(control.Handle()->Appereance.get());

		m_module.m_window = control.Handle();
	}

	void ThumbListBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		auto backgroundRect = window->Size.ToRectangle();
		auto innerMargin = window->ToScale(3u);
		
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBackground, true);
		backgroundRect.Width -= innerMargin * 2u;
		backgroundRect.Height -= innerMargin * 2u;

		auto thumbSize = window->ToScale(m_module.ThumbnailSize);
		auto maxCardMargin = window->ToScale(8u);
		uint32_t maxCardWidth = maxCardMargin * 2u + thumbSize + innerMargin;
		auto totalCardsInRow = backgroundRect.Width / maxCardWidth;
		if (totalCardsInRow == 0)
		{
			totalCardsInRow = 1;
		}
		auto marginRemainder = backgroundRect.Width % maxCardWidth;
		auto cardHeight = window->ToScale(m_module.Appearance->ThumbnailCardHeight);

		Point offset{ (int)innerMargin, (int)innerMargin - m_module.m_state.m_offset };
		Size cardSize{ thumbSize, thumbSize + cardHeight };
		auto cardMargin = marginRemainder / totalCardsInRow;
		auto cardMarginHalf = cardMargin >> 1;
		auto totalRows = (uint32_t)(m_module.Items.size()) / totalCardsInRow;
		if ((uint32_t)(m_module.Items.size()) % totalCardsInRow != 0)
		{
			++totalRows;
		}

		auto contentSize = totalRows * (cardSize.Height + innerMargin) - innerMargin;
		//BT_CORE_TRACE << " -- content size " << contentSize << " totalRows " << totalRows << " backgroundRect " << backgroundRect.Height << " max " << (int)(contentSize - backgroundRect.Height) << std::endl;
		if (contentSize >= backgroundRect.Height)
		{
			if (!m_module.m_scrollBar)
			{
				auto scrollSize = window->ToScale(window->Appereance->ScrollBarSize);
				Rectangle rect{ static_cast<int>(window->Size.Width - scrollSize) - 1, 1, scrollSize, window->Size.Height - 2u };

				m_module.m_scrollBar = std::make_unique<ScrollBar>(window, false, rect);
				m_module.m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						//BT_CORE_TRACE << " - thumb scroll value = " << args.Value << ". max = " << m_module.m_scrollBar->GetMax() << ". page step = " << m_module.m_scrollBar->GetPageStepValue() << std::endl;
						m_module.m_state.m_offset = args.Value;

						m_control->Handle()->Renderer.Update();
						GUI::RefreshWindow(m_control->Handle());
					});
				backgroundRect.Width -= rect.Width;
			}
			m_module.m_scrollBar->SetMinMax(0, (int)(contentSize - backgroundRect.Height));
			m_module.m_scrollBar->SetPageStepValue(backgroundRect.Height);
			m_module.m_scrollBar->SetStepValue(cardSize.Height);
		}
		else if (m_module.m_scrollBar)
		{
			m_module.m_state.m_offset = 0;
			m_module.m_scrollBar.reset();
		}

		for (size_t i = 0; i < m_module.Items.size(); i++)
		{
			auto& item = m_module.Items[i];

			Rectangle cardRect{ offset.X + (int)cardMarginHalf, offset.Y, cardSize.Width, cardSize.Height };
			graphics.DrawRectangle(cardRect, window->Appereance->Background, true);

			auto lineColor = enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor;
			graphics.DrawRectangle(cardRect, lineColor, false);
			graphics.DrawLine({ cardRect.X, cardRect.Y + (int)thumbSize }, { cardRect.X + (int)cardSize.Width, cardRect.Y + (int)thumbSize }, lineColor);

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

				graphics.DrawString({ cardRect.X + (int)center.Width, cardRect.Y + (int)thumbSize + (int)center.Height }, item.Text, window->Appereance->Foreground);
			}

			offset.X += cardMargin + cardSize.Width + innerMargin;
			if (offset.X + cardMargin + cardSize.Width >= (int)window->Size.Width)
			{
				offset.X = innerMargin;
				offset.Y += cardSize.Height + innerMargin;
			}
		}

		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor, false);
	}

	void ThumbListBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.UpdateScrollBar();
		if (m_module.m_scrollBar)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appereance->ScrollBarSize);
			Rectangle rect{ static_cast<int>(m_module.m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_module.m_window->Size.Height - 2u };
			GUI::MoveWindow(m_module.m_scrollBar->Handle(), rect);
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
			GUI::UpdateDeferred(m_module.m_scrollBar->Handle());

			Update(graphics);
			GUI::UpdateDeferred(*m_control);
		}
	}

	void ThumbListBoxReactor::Module::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		auto& newItem = Items.emplace_back();
		newItem.Text = text;
		newItem.Thumbnail = thumbnail;
	}

	void ThumbListBoxReactor::Module::Clear()
	{
	}

	void ThumbListBoxReactor::Module::SetThumbnailSize(uint32_t size)
	{
		ThumbnailSize = size;
	}

	void ThumbListBoxReactor::Module::UpdateScrollBar()
	{
		auto window = m_window;
		auto backgroundRect = window->Size.ToRectangle();
		auto innerMargin = window->ToScale(3u);

		backgroundRect.Width -= innerMargin * 2u;
		backgroundRect.Height -= innerMargin * 2u;

		auto thumbSize = window->ToScale(ThumbnailSize);
		auto maxCardMargin = window->ToScale(8u);
		uint32_t maxCardWidth = maxCardMargin * 2u + thumbSize + innerMargin;
		auto totalCardsInRow = backgroundRect.Width / maxCardWidth;
		if (totalCardsInRow == 0)
		{
			totalCardsInRow = 1;
		}
		auto marginRemainder = backgroundRect.Width % maxCardWidth;
		auto cardHeight = window->ToScale(Appearance->ThumbnailCardHeight);

		Point offset{ (int)innerMargin, (int)innerMargin + m_state.m_offset };
		Size cardSize{ thumbSize, thumbSize + cardHeight };
		auto cardMargin = marginRemainder / totalCardsInRow;
		auto cardMarginHalf = cardMargin >> 1;
		auto totalRows = (uint32_t)(Items.size()) / totalCardsInRow;
		if ((uint32_t)(Items.size()) % totalCardsInRow != 0)
		{
			++totalRows;
		}

		auto contentSize = totalRows * (cardSize.Height + innerMargin) - innerMargin;
		
		bool needScrollBar = contentSize >= backgroundRect.Height;
		if (!needScrollBar && m_scrollBar)
		{
			m_scrollBar.reset();
			m_state.m_offset = 0;
		}

		if (!needScrollBar && !m_scrollBar)
		{
			return;
		}

		if (!m_scrollBar)
		{
			auto scrollSize = window->ToScale(window->Appereance->ScrollBarSize);
			Rectangle rect{ static_cast<int>(window->Size.Width - scrollSize) - 1, 1, scrollSize, window->Size.Height - 2u };

			m_scrollBar = std::make_unique<ScrollBar>(window, false, rect);
			m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
				{
					//BT_CORE_TRACE << " - thumb scroll value = " << args.Value << ". max = " << m_scrollBar->GetMax() << ". page step = " << m_scrollBar->GetPageStepValue() << std::endl;
					m_state.m_offset = args.Value;

					m_window->Renderer.Update();
					GUI::RefreshWindow(m_window);
				});
			backgroundRect.Width -= rect.Width;
		}
		m_scrollBar->SetMinMax(0, (int)(contentSize - backgroundRect.Height));
		m_scrollBar->SetPageStepValue(backgroundRect.Height);
		m_scrollBar->SetStepValue(cardSize.Height);
	}

	void ThumbListBoxReactor::Module::BuildItems()
	{
	}

	ThumbListBox::ThumbListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
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
}
