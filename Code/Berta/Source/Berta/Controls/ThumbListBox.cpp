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
		if (m_module.m_scrollBar)
		{
			backgroundRect.Width -= m_module.m_scrollBar->GetSize().Width;
		}
		graphics.DrawRectangle(backgroundRect, window->Appereance->BoxBackground, true);
		backgroundRect.Width -= innerMargin * 2;

		auto thumbSize = window->ToScale(m_module.ThumbnailSize);
		auto maxCardMargin = window->ToScale(18u);
		auto minCardMargin = window->ToScale(3u);
		uint32_t maxCardWidth = maxCardMargin * 2u + thumbSize;
		auto totalCards = backgroundRect.Width / maxCardWidth;
		auto marginRemainder = backgroundRect.Width % maxCardWidth;
		auto cardHeight = window->ToScale(m_module.Appearance->ThumbnailCardHeight);

		Point offset{ (int)innerMargin, (int)innerMargin };
		Size cardSize{ thumbSize, thumbSize + cardHeight };
		if (totalCards == 0) totalCards = 1;

		auto cardMargin = marginRemainder / totalCards;
		auto cardMarginHalf = cardMargin >> 1;
		Size cardSizeWithMargin{ thumbSize + cardMargin * 2, thumbSize + cardHeight };

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

			offset.X += cardMargin + cardSize.Width;
			if (offset.X + cardMargin + cardSize.Width >= (int)window->Size.Width)
			{
				offset.X = innerMargin;
				offset.Y += cardSizeWithMargin.Height + innerMargin;
			}
		}

		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor, false);
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
