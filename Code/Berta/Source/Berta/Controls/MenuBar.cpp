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
		m_module.m_control = &control;

		m_module.m_owner = control.Handle();
	}

	void MenuBarReactor::Update(Graphics& graphics)
	{
		auto window = m_module.m_owner;
		bool enabled = m_module.m_control->GetEnabled();
		
		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->ButtonBackground : window->Appereance->ButtonDisabledBackground, true);

		auto& items = m_module.m_items;
		auto itemMargin = static_cast<uint32_t>(4u * window->DPIScaleFactor);
		
		for (size_t i = 0; i < items.size(); i++)
		{
			auto& itemData = *(items[i]);

			if (m_module.m_interactionData.m_selectedItemIndex == i)
			{
				graphics.DrawRectangle({ itemData.position.X, itemData.position.Y, itemData.size.Width, itemData.size.Height}, m_module.m_interactionData.m_activeMenu ? window->Appereance->MenuBackground : window->Appereance->HighlightColor, true);
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
		if (m_module.m_interactionData.m_activeMenu)
			return;

		m_module.m_interactionData.m_selectedItemIndex = -1;

		Update(graphics);
		GUI::UpdateDeferred(m_module.m_owner);
	}

	void MenuBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		int selectedItem = m_module.FindItem(args.Position);
		m_module.m_interactionData.m_selectedItemIndex = selectedItem;
		if (selectedItem != -1)
		{
			if (m_module.m_interactionData.m_activeMenu)
			{
				m_module.m_interactionData.m_activeMenu = nullptr;
				//GUI::ReleaseCapture(*m_control);
			}
			else
			{
				m_module.OpenMenu();
				
			}
		}
		else
		{
			if (m_module.m_interactionData.m_activeMenu)
			{
				m_module.m_interactionData.m_activeMenu = nullptr;
			}
		}

		Update(graphics);
		GUI::UpdateDeferred(m_module.m_owner);
	}

	void MenuBarReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		int selectedItem = m_module.FindItem(args.Position);

		if (selectedItem != m_module.m_interactionData.m_selectedItemIndex)
		{
			if (m_module.m_interactionData.m_activeMenu && selectedItem != -1)
			{
				m_module.m_interactionData.m_activeMenu = &m_module.m_items[selectedItem]->menu;
				m_module.m_interactionData.m_selectedItemIndex = selectedItem;
			}
			else if (!m_module.m_interactionData.m_activeMenu)
			{
				m_module.m_interactionData.m_selectedItemIndex = selectedItem;
			}

			Update(graphics);
			GUI::UpdateDeferred(m_module.m_owner);
		}
	}

	void MenuBarReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		//if (m_module.m_interactionData.m_activeMenu)
		{
			//GUI::ReleaseCapture(*m_control);
			//m_interactionData.m_activeMenu = nullptr;
		}
	}

	int MenuBarReactor::Module::FindItem(const Point& position)
	{
		auto window = m_control->Handle();
		auto& items = m_items;

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

	void MenuBarReactor::Module::OpenMenu()
	{
		auto window = m_owner;
		auto itemData = m_items[m_interactionData.m_selectedItemIndex];
		m_interactionData.m_activeMenu = &itemData->menu;
		auto pointInScreen = GUI::GetPointClientToScreen(window, window->Position);

		Point boxPosition = pointInScreen;
		boxPosition.X += itemData->position.X;
		boxPosition.Y += itemData->position.Y + (int)itemData->size.Height;

		m_interactionData.m_activeMenu->m_destroyCallback = [this]()
		{
			m_interactionData.m_activeMenu = nullptr;
			m_interactionData.m_selectedItemIndex = -1;

			m_control->Handle()->Renderer.Update();
			GUI::UpdateDeferred(*m_control);	
		};
		m_interactionData.m_activeMenu->ShowPopup(m_owner, boxPosition);
	}

	Menu& MenuBarReactor::Module::PushBack(const std::wstring& text)
	{
		auto newItem = m_items.emplace_back(new MenuBarReactor::MenuBarItemData{ text });
		BuildItems();
		return newItem->menu;
	}

	void MenuBarReactor::Module::BuildItems()
	{
		auto itemMargin = static_cast<uint32_t>(4u * m_owner->DPIScaleFactor);
		Point offset{ 0, (int)itemMargin };

		for (size_t i = 0; i < m_items.size(); i++)
		{
			auto& itemData = *m_items[i];
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

	MenuBar::MenuBar(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}

	Menu& MenuBar::PushBack(const std::wstring& itemName)
	{
		return m_reactor.GetModule().PushBack(itemName);
	}
}