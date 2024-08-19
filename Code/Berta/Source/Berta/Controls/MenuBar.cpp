/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "MenuBar.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	void MenuBarReactor::Init(ControlBase& control)
	{
		m_module.m_control = reinterpret_cast<MenuBar*>(&control);

		m_module.m_owner = control.Handle();
		m_module.m_rootMenuItemReactor = this;

		m_module.m_owner->Events->Focus.Connect([&](const ArgFocus& args)
		{
			if (!args.Focused && m_module.IsMenuOpen())
			{
				GUI::DisposeMenu();
			}
		});
	}

	void MenuBarReactor::Update(Graphics& graphics)
	{
		auto window = m_module.m_owner;
		bool enabled = m_module.m_control->GetEnabled();

		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->ButtonBackground : window->Appereance->ButtonDisabledBackground, true);

		auto& items = m_module.m_items;
		auto itemMargin = window->ToScale(4u);

		for (size_t i = 0; i < items.size(); i++)
		{
			auto& itemData = *(items[i]);

			if (m_module.m_interactionData.m_selectedItemIndex == (int)i)
			{
				graphics.DrawRectangle({ itemData.position.X, itemData.position.Y, itemData.size.Width, itemData.size.Height }, m_module.IsMenuOpen() ? window->Appereance->MenuBackground : window->Appereance->HighlightColor, true);

				if (m_module.IsMenuOpen())
				{
					graphics.DrawLine({ itemData.position.X, itemData.position.Y }, { itemData.position.X + (int)itemData.size.Width, itemData.position.Y }, window->Appereance->BoxBorderColor);
					graphics.DrawLine({ itemData.position.X, itemData.position.Y }, { itemData.position.X, itemData.position.Y + (int)itemData.size.Height }, window->Appereance->BoxBorderColor);
					graphics.DrawLine({ itemData.position.X + (int)itemData.size.Width, itemData.position.Y }, { itemData.position.X + (int)itemData.size.Width, itemData.position.Y + (int)itemData.size.Height }, window->Appereance->BoxBorderColor);
				}
				graphics.DrawString({ itemData.position.X + (int)itemData.center.Width, itemData.position.Y + (int)itemData.center.Height }, itemData.text, m_module.IsMenuOpen() ? window->Appereance->Foreground : window->Appereance->HighlightTextColor);
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
		if (m_module.IsMenuOpen())
			return;

		m_module.SelectIndex(-1);

		Update(graphics);
		GUI::UpdateDeferred(m_module.m_owner);
	}

	void MenuBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (!args.ButtonState.LeftButton)
		{
			return;
		}

		int selectedItem = m_module.FindItem(args.Position);
		m_module.SelectIndex(selectedItem);
		if (selectedItem != -1)
		{
			m_module.OpenMenu();
			if (m_module.IsMenuOpen())
			{
				m_next = m_module.m_interactionData.m_activeMenu->m_menuBox->GetItemReactor();
				GUI::SetMenu(this);
			}

			Update(graphics);
			GUI::UpdateDeferred(m_module.m_owner);
		}
	}

	void MenuBarReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		int selectedItem = m_module.FindItem(args.Position);

		if (m_module.IsMenuOpen())
		{
			if (selectedItem != -1 && selectedItem != m_module.m_interactionData.m_selectedItemIndex && m_module.m_lastMousePosition != args.Position) // check last mouse position, or might be better if the keyboard is captured ?
			{
				if (m_module.GetActiveMenuBox())
				{
					GUI::DisposeMenu(m_module.GetActiveMenuBox()->GetItemReactor());
				}

				m_module.SelectIndex(selectedItem);
				m_module.OpenMenu(args.ButtonState.LeftButton);
				m_next = m_module.GetActiveMenuBox()->GetItemReactor();
				GUI::SetMenu(this);

				Update(m_module.m_owner->Renderer.GetGraphics());
				GUI::RefreshWindow(m_module.m_owner);
			}
		}
		else
		{
			if (selectedItem != m_module.m_interactionData.m_selectedItemIndex)
			{
				m_module.SelectIndex(selectedItem);

				Update(graphics);
				GUI::UpdateDeferred(m_module.m_owner);
			}
		}
		m_module.m_lastMousePosition = args.Position;
	}

	void MenuBarReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.BuildItems();
	}

	void MenuBarReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		if (m_module.IsMenuOpen())
		{
			auto lastMenuItem = GetLastMenuItem();
			if (args.Key == KeyboardKey::ArrowUp)
			{
				lastMenuItem->MoveToNextItem(true);
			}
			else if (args.Key == KeyboardKey::ArrowDown)
			{
				lastMenuItem->MoveToNextItem(false);
			}
			else if (args.Key == KeyboardKey::ArrowLeft)
			{
				if (!lastMenuItem->ExitSubMenu())
				{
					MoveToNextItem(true);
				}
			}
			else if (args.Key == KeyboardKey::ArrowRight)
			{
				if (!lastMenuItem->EnterSubMenu())
				{
					MoveToNextItem(false);
				}
			}
			else if(args.Key == KeyboardKey::Enter)
			{
				lastMenuItem->Select();
			}
			else if (args.Key == KeyboardKey::Escape)
			{
				lastMenuItem->Quit();
			}
		}
	}

	void MenuBarReactor::MoveToNextItem(bool upwards)
	{
		if (!m_module.IsMenuOpen())
		{
			return;
		}
		int direction = upwards ? -1 : 1;
		int selectedItem = m_module.m_interactionData.m_selectedItemIndex;
		int totalItems = static_cast<int>(m_module.m_items.size());
		selectedItem = (selectedItem + direction + totalItems) % totalItems;
		GUI::DisposeMenu(m_module.GetActiveMenuBox()->GetItemReactor());

		m_module.SelectIndex(selectedItem);
		m_module.OpenMenu(false);
		m_next = m_module.GetActiveMenuBox()->GetItemReactor();
		GUI::SetMenu(this);

		Update(m_module.m_owner->Renderer.GetGraphics());
		GUI::RefreshWindow(m_module.m_owner);
	}

	void MenuBarReactor::Select()
	{
	}

	void MenuBarReactor::Quit()
	{
	}

	Window* MenuBarReactor::Owner() const
	{
		return m_module.m_owner;
	}

	MenuItemReactor* MenuBarReactor::GetLastMenuItem() const
	{
		auto activeMenuItemReactor = (MenuItemReactor*)this;
		while (activeMenuItemReactor->Next() != nullptr)
		{
			activeMenuItemReactor = activeMenuItemReactor->Next();
		}
		return activeMenuItemReactor;
	}

	int MenuBarReactor::Module::FindItem(const Point& position)
	{
		auto& items = m_items;

		for (size_t i = 0; i < items.size(); i++)
		{
			auto& itemData = *(items[i]);

			if (Rectangle{ itemData.position, itemData.size}.IsInside(position))
			{
				return (int)i;
			}
		}
		return -1;
	}

	void MenuBarReactor::Module::OpenMenu(bool ignoreFirstMouseUp)
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
			SelectIndex(-1);

			m_rootMenuItemReactor->Clear();
			m_control->Handle()->Renderer.Update();
			GUI::UpdateDeferred(*m_control);
		};
		Rectangle menuBarItemRect{ 0,0,m_items[m_interactionData.m_selectedItemIndex]->size.Width, m_items[m_interactionData.m_selectedItemIndex]->size.Height };
		m_interactionData.m_activeMenu->ShowPopup(m_owner, boxPosition, nullptr, ignoreFirstMouseUp, menuBarItemRect);
	}

	void MenuBarReactor::Module::SelectIndex(int index)
	{
		m_interactionData.m_selectedItemIndex = index;
	}

	MenuBox* MenuBarReactor::Module::GetActiveMenuBox() const
	{
		return m_interactionData.m_activeMenu->m_menuBox;
	}

	void MenuBarReactor::Module::BuildItems(size_t startIndex)
	{
		if (startIndex >= m_items.size())
		{
			return;
		}

		auto itemMargin = m_owner->ToScale(4u);
		Point offset{ 0, (int)itemMargin };

		if (startIndex > 0)
		{
			offset.X = m_items[startIndex - 1]->position.X + (int)m_items[startIndex - 1]->size.Width;
		}

		for (size_t i = startIndex; i < m_items.size(); i++)
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

	Menu& MenuBarReactor::Module::PushBack(const std::wstring& text)
	{
		auto startIndex = m_items.size();
		auto& newItem = m_items.emplace_back(new MenuBarReactor::MenuBarItemData{ text });
		BuildItems(startIndex);

		return newItem->menu;
	}

	MenuBar::MenuBar(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}

	size_t MenuBar::GetCount() const
	{
		return m_reactor.GetModule().m_items.size();
	}

	Menu& MenuBar::PushBack(const std::wstring& itemName)
	{
		return m_reactor.GetModule().PushBack(itemName);
	}
}