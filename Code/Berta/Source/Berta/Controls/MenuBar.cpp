/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "MenuBar.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void MenuBarReactor::Init(ControlBase& control)
	{
		m_control = &control;

		m_interactionData.m_owner = *m_control;
	}

	void MenuBarReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		
		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->ButtonBackground : window->Appereance->ButtonDisabledBackground, true);

		auto& items = m_interactionData.m_menuCollectionData.m_items;
		auto itemMargin = static_cast<uint32_t>(4u * window->DPIScaleFactor);
		
		for (size_t i = 0; i < items.size(); i++)
		{
			auto& itemData = *(items[i]);

			if (m_interactionData.m_hoveredItemIndex == i)
			{
				graphics.DrawRectangle({ itemData.position.X, itemData.position.Y, itemData.size.Width, itemData.size.Height}, window->Appereance->HighlightColor, true);
				graphics.DrawString({ itemData.position.X + (int)itemData.center.Width, itemData.position.Y + (int)itemData.center.Height }, itemData.text, window->Appereance->HighlightTextColor);
			}
			else
			{
				graphics.DrawString({ itemData.position.X + (int)itemData.center.Width, itemData.position.Y + (int)itemData.center.Height }, itemData.text, enabled ? window->Appereance->Foreground : window->Appereance->BoxBorderDisabledColor);
			}
		}
	}

	void MenuBarReactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
	}

	void MenuBarReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		m_interactionData.m_hoveredItemIndex = -1;

		Update(graphics);
		GUI::UpdateDeferred(*m_control);
	}

	void MenuBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		GUI::Capture(*m_control);
	}

	void MenuBarReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		int selectedItem = FindItem(args.Position);

		if (selectedItem != m_interactionData.m_hoveredItemIndex)
		{
			m_interactionData.m_hoveredItemIndex = selectedItem;

			Update(graphics);
			GUI::UpdateDeferred(*m_control);
		}
	}

	void MenuBarReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		GUI::ReleaseCapture(*m_control);
	}

	int MenuBarReactor::FindItem(const Point& position)
	{
		auto window = m_control->Handle();
		auto& items = m_interactionData.m_menuCollectionData.m_items;

		for (size_t i = 0; i < items.size(); i++)
		{
			auto& itemData = *(items[i]);

			if (position.X >= itemData.position.X && position.X <= itemData.position.X + (int)itemData.size.Width &&
				position.Y >= itemData.position.Y && position.Y <= itemData.position.Y + (int)itemData.size.Height)
			{
				return (int)i;
			}
		}
		return -1;
	}

	MenuBar::MenuBar(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}

	void MenuBar::PushBack(const std::wstring& itemName)
	{
		m_reactor.GetInteractionData().PushBack(itemName);
	}

	void MenuBarReactor::InteractionData::PushBack(const std::wstring& text)
	{
		m_menuCollectionData.m_items.emplace_back(new MenuBarReactor::MenuBarItemData{ text });
		BuildItems();
	}

	void MenuBarReactor::InteractionData::BuildItems()
	{
		auto itemMargin = static_cast<uint32_t>(4u * m_owner->DPIScaleFactor);
		Point offset{ 0, (int)itemMargin };

		int selectedItem = -1;
		for (size_t i = 0; i < m_menuCollectionData.m_items.size(); i++)
		{
			auto& itemData = *m_menuCollectionData.m_items[i];
			auto textSize = m_owner->Renderer.GetGraphics().GetTextExtent(itemData.text);
			Size itemSize{ textSize.Width + itemMargin * 4, m_owner->Size.Height - itemMargin * 2 };

			auto center = itemSize - textSize;
			center = center * 0.5f;

			Point itemPos = offset;
			itemData.position = itemPos;
			itemData.size = itemSize;
			itemData.center = center;

			offset.X += (int)itemSize.Width;
		}
	}
}