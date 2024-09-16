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
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBackground, true);

		auto cardMargin = window->ToScale(10u);
		auto cardHeight = window->ToScale(m_module.Appearance->ThumbnailCardHeight);
		auto thumbSize = window->ToScale(m_module.ThumbnailSize);

		Point offset{ (int)cardMargin, (int)cardMargin };
		Size cardSize{ thumbSize, thumbSize + cardHeight };
		Size cardSizeWithMargin{ thumbSize + cardMargin * 2, thumbSize + cardHeight + cardMargin * 2 };
		
		for (size_t i = 0; i < m_module.Items.size(); i++)
		{
			auto& item = m_module.Items[i];

			Rectangle cardRect{ offset.X, offset.Y, cardSize.Width, cardSize.Height};
			graphics.DrawRectangle(cardRect, window->Appereance->Background, true);

			auto lineColor = enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor;
			graphics.DrawRectangle(cardRect, lineColor, false);
			graphics.DrawLine({ cardRect.X, cardRect.Y + (int)thumbSize }, { cardRect.X + (int)cardSize.Width, cardRect.Y + (int)thumbSize }, lineColor);

			Size imageSize = item.Thumbnail.GetSize();
			imageSize *= window->DPIScaleFactor;
			Size thumbFrameSize{ thumbSize, thumbSize };
			auto center = thumbFrameSize - imageSize;
			center *= 0.5f;

			item.Thumbnail.Paste(graphics, { cardRect.X + (int)center.Width, cardRect.Y + (int)center.Height });

			graphics.DrawString({ cardRect.X + (int)cardMargin, cardRect.Y + (int)(thumbSize + cardMargin) }, item.Text, window->Appereance->Foreground);
			offset.X += (int)cardSizeWithMargin.Width;
			if (offset.X >= (int)window->Size.Width)
			{
				offset.X = (int)cardMargin;
				offset.Y += cardSizeWithMargin.Height;
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
