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
	}

	void ThumbListBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		auto backgroundRect = window->Size.ToRectangle();
		auto innerMargin = window->ToScale(3u);
		
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBackground, true);
		backgroundRect.Width -= innerMargin * 2;
		backgroundRect.Height -= innerMargin * 2;

		auto thumbSize = window->ToScale(m_module.ThumbnailSize);
		auto maxCardMargin = window->ToScale(8u);
		auto minCardMargin = window->ToScale(3u);
		uint32_t maxCardWidth = maxCardMargin * 2u + thumbSize + innerMargin;
		auto totalCardsInRow = backgroundRect.Width / maxCardWidth;
		if (totalCardsInRow == 0)
		{
			totalCardsInRow = 1;
		}
		auto marginRemainder = backgroundRect.Width % maxCardWidth;
		auto cardHeight = window->ToScale(m_module.Appearance->ThumbnailCardHeight);

		Point offset{ (int)innerMargin, (int)innerMargin };
		Size cardSize{ thumbSize, thumbSize + cardHeight };
		auto cardMargin = marginRemainder / totalCardsInRow;
		auto cardMarginHalf = cardMargin >> 1;
		Size cardSizeWithMargin{ thumbSize + cardMargin * 2, thumbSize + cardHeight };
		auto totalRows = (uint32_t)(m_module.Items.size()) / totalCardsInRow;
		if ((uint32_t)(m_module.Items.size()) % totalCardsInRow != 0)
		{
			++totalRows;
		}

		auto contentSize = totalRows * (cardSize.Height + innerMargin*2);
		if (contentSize >= backgroundRect.Height)
		{
			if (!m_module.m_scrollBar)
			{
				auto scrollSize = window->ToScale(window->Appereance->ScrollBarSize);
				Rectangle rect{ static_cast<int>(window->Size.Width - scrollSize) - 1, 1, scrollSize, window->Size.Height - 2u };

				m_module.m_scrollBar = std::make_unique<ScrollBar>(window, false, rect);
				backgroundRect.Width -= rect.Width;
			}
			m_module.m_scrollBar->SetMinMax(0, (int)(contentSize - backgroundRect.Height));
			m_module.m_scrollBar->SetStepValue(window->ToScale(20));
		}
		else if (m_module.m_scrollBar)
		{
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
				offset.Y += cardSizeWithMargin.Height + innerMargin;
			}
		}

		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor, false);
	}

	void ThumbListBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		auto window = m_control->Handle();
		if (m_module.m_scrollBar)
		{
			auto scrollSize = window->ToScale(window->Appereance->ScrollBarSize);
			Rectangle rect{ static_cast<int>(window->Size.Width - scrollSize) - 1, 1, scrollSize, window->Size.Height - 2u };

			auto backgroundRect = window->Size.ToRectangle();
			auto innerMargin = window->ToScale(3u);
			backgroundRect.Width -= innerMargin * 2;
			backgroundRect.Height -= innerMargin * 2;
			auto thumbSize = window->ToScale(m_module.ThumbnailSize);
			auto maxCardMargin = window->ToScale(8u);
			auto minCardMargin = window->ToScale(3u);
			uint32_t maxCardWidth = maxCardMargin * 2u + thumbSize;
			auto totalCardsWide = backgroundRect.Width / maxCardWidth;
			if (totalCardsWide == 0)
			{
				totalCardsWide = 1;
			}
			auto marginRemainder = backgroundRect.Width % maxCardWidth;
			auto cardHeight = window->ToScale(m_module.Appearance->ThumbnailCardHeight);

			Point offset{ (int)innerMargin, (int)innerMargin };
			Size cardSize{ thumbSize, thumbSize + cardHeight };
			auto cardMargin = marginRemainder / totalCardsWide;
			auto cardMarginHalf = cardMargin >> 1;
			Size cardSizeWithMargin{ thumbSize + cardMargin * 2, thumbSize + cardHeight };
			auto totalRows = (uint32_t)(m_module.Items.size()) / totalCardsWide;
			if ((uint32_t)(m_module.Items.size()) % totalCardsWide != 0)
			{
				++totalRows;
			}

			auto contentSize = totalRows * (cardSize.Height + innerMargin * 2);
			if (contentSize >= backgroundRect.Height)
			{
				m_module.m_scrollBar->SetMinMax(0, (int)(contentSize - backgroundRect.Height));
				m_module.m_scrollBar->SetStepValue(window->ToScale(20));
				GUI::MoveWindow(m_module.m_scrollBar->Handle(), rect);
			}
			else
			{
				m_module.m_scrollBar.reset();
			}
			
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
		bool needScrollBar = false;
		if (!needScrollBar && m_scrollBar)
		{
			m_scrollBar.reset();
			m_state.m_offset = 0;
		}

		if (!needScrollBar && !m_scrollBar)
		{
			return;
		}
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
