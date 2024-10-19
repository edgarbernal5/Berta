/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Menu.h"

#include "Berta/GUI/Interface.h"
#include "Berta/Controls/Menu.h"
#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
#if BT_DEBUG
	int MenuBox::g_globalId = 0;
#endif

	void Menu::Append(const std::wstring& text, ClickCallback onClick)
	{
		auto& newItem = m_items.emplace_back(new Menu::Item{ text , onClick });
	}

	void Menu::AppendSeparator()
	{
		m_items.emplace_back(new Menu::Item());
	}

	void Menu::ShowPopup(Window* owner, const Point& position, Menu* parentMenu, bool ignoreFirstMouseUp, Rectangle menuBarItem)
	{
		m_parentWindow = owner;

		auto boxSize = GetMenuBoxSize(owner);//TODO: meter esto dentro de la clase MenuBox
		m_menuBox = new MenuBox(owner, { position.X, position.Y, boxSize.Width, boxSize.Height });
		m_menuBox->Init(this, m_items, menuBarItem);
		m_menuBox->SetIgnoreFirstMouseUp(ignoreFirstMouseUp);

		m_menuBox->GetEvents().Destroy.Connect([this](const ArgDestroy& argDestroy)
		{
			//BT_CORE_TRACE << "   - menu box destroy callback..." << std::endl;
			delete m_menuBox;
			m_menuBox = nullptr;

			if (m_destroyCallback)
			{
				m_destroyCallback();
			}
		});

		m_menuBox->Popup();
	}

	void Menu::ShowPopup(Window* owner, const ArgMouse& args)
	{
		if (!args.ButtonState.RightButton)
		{
			return;
		}
		auto screenPosition = GUI::GetPointClientToScreen(owner, args.Position);
		ShowPopup(owner, screenPosition);
		//m_menuBox->GetEvents().Destroy.Connect([this](const ArgDestroy& argDestroy)
		//{
		//	//BT_CORE_TRACE << "   - menu box destroy callback..." << std::endl;			
		//});
		
		//TODO: Fix this! ConnectOnce
		/*owner->Events->Focus.ConnectOnce([&](const ArgFocus& args)
		{
			if (!args.Focused)
			{
				GUI::DisposeMenu();
			}
		});*/
		GUI::SetMenu(m_menuBox->GetItemReactor());
	}

	Menu* Menu::CreateSubMenu(std::size_t index)
	{
		if (index >= 0 && index < m_items.size())
		{
			auto& menuItem = m_items.at(index);
			if (!menuItem->m_subMenu)
			{
				menuItem->m_subMenu = new Menu();
			}

			return menuItem->m_subMenu;
		}
		return nullptr;
	}

	void Menu::SetImage(size_t index, const Image& image)
	{
		m_items.at(index)->m_image = image;
	}

	void Menu::SetEnabled(size_t index, bool enabled)
	{
		m_items.at(index)->isEnabled = enabled;
	}

	void Menu::CloseMenuBox()
	{
		if (!m_menuBox)
			return;

		GUI::ReleaseCapture(m_menuBox->Handle());
		m_menuBox->Dispose();
		m_menuBox = nullptr;
	}

	Size Menu::GetMenuBoxSize(Window* parent)
	{
		Size size;
		uint32_t separators = 0;
		uint32_t maxWidth = 0;
		bool hasSubmenu = false;
		for (size_t i = 0; i < m_items.size(); i++)
		{
			if (m_items[i]->isSpearator)
			{
				++separators;
			}
			else
			{
				auto textSize = parent->Renderer.GetGraphics().GetTextExtent((m_items[i]->text));
				maxWidth = (std::max)(maxWidth, textSize.Width);
				hasSubmenu |= m_items[i]->m_subMenu != nullptr;
			}
		}

		auto menuBoxLeftPaneWidth = parent->ToScale(parent->Appereance->MenuBoxLeftPaneWidth);
		auto itemTextPadding = parent->ToScale(ItemTextPadding);
		auto menuBoxSubMenuArrowWidth = hasSubmenu ? parent->ToScale(parent->Appereance->MenuBoxSubMenuArrowWidth) : 0;
		auto separatorHeight = parent->ToScale(SeparatorHeight);
		auto menuBoxItemHeight = parent->ToScale(parent->Appereance->MenuBoxItemHeight);
		auto menuBoxShortcutWidth = parent->ToScale(parent->Appereance->MenuBoxShortcutWidth);

		return { 
			2 + menuBoxLeftPaneWidth + maxWidth + itemTextPadding * 2u + menuBoxSubMenuArrowWidth + menuBoxShortcutWidth,
			2 + itemTextPadding * 2u + (uint32_t)(m_items.size() - separators) * (menuBoxItemHeight) + separators * separatorHeight
		};
	}

	MenuBoxReactor::~MenuBoxReactor()
	{
	}

	void MenuBoxReactor::Init(ControlBase& control)
	{
		m_control = reinterpret_cast<MenuBox*>(&control);

		m_subMenuTimer.SetOwner(m_control->Handle());
		m_subMenuTimer.SetInterval(400);
		m_subMenuTimer.Connect([this](const ArgTimer& args)
		{
			//opens submenu
			if (m_selectedSubMenuIndex != -1 && m_items->at(m_selectedSubMenuIndex)->m_subMenu)
			{
				if (m_openedSubMenuIndex != m_selectedIndex && m_openedSubMenuIndex >= 0)
				{
					auto subMenu = m_items->at(m_openedSubMenuIndex)->m_subMenu;

					GUI::DisposeMenu(m_next);
					m_next = nullptr;
				}
				auto subMenu = m_items->at(m_selectedSubMenuIndex)->m_subMenu;
				if (!subMenu->m_menuBox)
				{
					m_openedSubMenuIndex = m_selectedSubMenuIndex;
					OpenSubMenu(subMenu, m_menuOwner, m_openedSubMenuIndex, false);
					m_subMenuTimer.Stop();
				}
			}
			else if (m_selectedSubMenuIndex == -1 && m_openedSubMenuIndex >= 0)
			{
				auto subMenu = m_items->at(m_openedSubMenuIndex)->m_subMenu;
				GUI::DisposeMenu(m_next);
				m_next = nullptr;
				m_openedSubMenuIndex = -1;
			}
			m_subMenuTimer.Stop();
		});
	}

	void MenuBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();

		graphics.DrawRectangle(window->Appereance->MenuBackground, true);

		auto menuBoxLeftPaneWidth = window->ToScale(window->Appereance->MenuBoxLeftPaneWidth);
		auto itemTextPadding = window->ToScale(ItemTextPadding);
		auto menuBoxItemHeight = window->ToScale(window->Appereance->MenuBoxItemHeight);
		auto menuBoxSubMenuArrowWidth = window->ToScale(window->Appereance->MenuBoxSubMenuArrowWidth);
		auto separatorHeight = window->ToScale(SeparatorHeight);

		if (m_items)
		{
			int offsetY = 1 + (int)itemTextPadding;
			for (size_t i = 0; i < m_items->size(); i++)
			{
				auto& item = *(m_items->at(i));
				if (item.isSpearator)
				{
					int separatorCenterOffset = (separatorHeight >> 1) - 1;
					graphics.DrawLine({ 1 + (int)menuBoxLeftPaneWidth - 4, offsetY + separatorCenterOffset + 1 }, { (int)window->Size.Width - 2, offsetY + separatorCenterOffset + 1 }, window->Appereance->BoxBorderColor);
					offsetY += separatorHeight;
				}
				else
				{
					auto textSize = graphics.GetTextExtent();
					int center = (int)menuBoxItemHeight - (int)textSize.Height;
					center >>= 1;

					bool isItemSelected = m_selectedIndex == (int)i;
					if (isItemSelected)
					{
						graphics.DrawRectangle({ 1 + (int)(itemTextPadding), offsetY, window->Size.Width - 2u - itemTextPadding * 2u, menuBoxItemHeight }, window->Appereance->HighlightColor, true);
					}
					if (item.m_image)
					{
						Point paneSize{ (int)menuBoxLeftPaneWidth, (int)menuBoxItemHeight };

						Size scaleImageSize= item.m_image.GetSize();
						scaleImageSize.Width = window->ToScale(scaleImageSize.Width);
						scaleImageSize.Height = window->ToScale(scaleImageSize.Height);
						Point imageSize{ (int)scaleImageSize.Width, (int)scaleImageSize.Height };
						Point centerImage = paneSize - imageSize;
						centerImage /= 2;
						Rectangle destRect{ { 1 + centerImage.X + (int)itemTextPadding, offsetY + centerImage.Y }, scaleImageSize };
						item.m_image.Paste(item.m_image.GetSize().ToRectangle(), graphics, destRect);
					}
					graphics.DrawString({ 1 + (int)(menuBoxLeftPaneWidth + itemTextPadding), offsetY + center}, item.text, item.isEnabled ? ( isItemSelected ? window->Appereance->HighlightTextColor : window->Appereance->Foreground) : window->Appereance->BoxBorderDisabledColor);
					
					if (item.m_subMenu)
					{
						int arrowWidth = window->ToScale(4);
						int arrowLength = window->ToScale(2);
						graphics.DrawArrow({ static_cast<int>(window->Size.Width - menuBoxSubMenuArrowWidth) , offsetY, menuBoxSubMenuArrowWidth, menuBoxItemHeight },
							arrowLength,
							arrowWidth,
							item.isEnabled ? (isItemSelected ? window->Appereance->HighlightTextColor : window->Appereance->Foreground) : window->Appereance->BoxBorderDisabledColor,
							Graphics::ArrowDirection::Right,
							true);
					}

					offsetY += menuBoxItemHeight;
				}
			}
		}

		graphics.DrawRectangle(window->Appereance->BoxBorderColor, false);
		if (m_menuBarItemRect.Width > 0)
		{
			graphics.DrawLine({ 1,0 }, { (int)m_menuBarItemRect.Width,0 }, window->Appereance->MenuBackground);
		}
	}

	void MenuBoxReactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
		//BT_CORE_TRACE << "   - Menu box mouse enter" << std::endl;
	}

	void MenuBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		//BT_CORE_TRACE << "   - Menu box mouse leave" << std::endl;
		bool changes = MouseMoveInternal(args);
		if (changes)
		{
			auto window = m_control->Handle();
			Update(graphics);
			GUI::MarkAsUpdated(window);
		}
	}

	void MenuBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (!args.ButtonState.LeftButton)
		{
			return;
		}

		if (m_selectedIndex != -1 && m_items->at(m_selectedIndex)->m_subMenu)
		{
			auto subMenu = m_items->at(m_selectedIndex)->m_subMenu;
			if (!subMenu->m_menuBox)
			{
				m_selectedSubMenuIndex = m_selectedIndex;
				m_openedSubMenuIndex = m_selectedIndex;
				OpenSubMenu(subMenu, m_menuOwner, m_selectedIndex);
				m_subMenuTimer.Stop();
			}
		}
	}

	void MenuBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		bool changes = MouseMoveInternal(args);
		if (changes)
		{
			auto window = m_control->Handle();
			Update(graphics);
			GUI::MarkAsUpdated(window);
		}
	}

	void MenuBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		BT_CORE_DEBUG << " MenuBoxReactor MouseUp(). " << m_ignoreFirstMouseUp << std::endl;
		if (m_ignoreFirstMouseUp)
		{
			m_ignoreFirstMouseUp = false;
			return;
		}

		if (m_selectedIndex == -1)
		{
			bool found = false;
			
			auto current = m_prev;
			while (current)
			{
				auto clicScreen = GUI::GetPointClientToScreen(m_control->Handle(), args.Position);
				auto localPosition = GUI::GetPointScreenToClient(current->Owner(), clicScreen);
				ArgMouse argMouse;
				argMouse.Position = localPosition;
				if (current->OnClickSubMenu(argMouse))
				{
					return;
				}
				current = current->Prev();
			}
		}

		size_t selectedIndex = static_cast<size_t>(m_selectedIndex);
		if (selectedIndex >= m_items->size())
		{
			GUI::DisposeMenu();
			return;
		}

		if (!args.ButtonState.LeftButton)
		{
			return;
		}

		auto& item = m_items->at(selectedIndex);
		if (item->m_subMenu)
		{
			return;
		}

		if (!item->isSpearator && item->onClick)
		{
			MenuItem menuItem(item);
			item->onClick(menuItem);
		}

		GUI::DisposeMenu();
	}

	void MenuBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
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
			lastMenuItem->ExitSubMenu();
		}
		else if (args.Key == KeyboardKey::ArrowRight)
		{
			lastMenuItem->EnterSubMenu();
		}
		else if (args.Key == KeyboardKey::Enter)
		{
			lastMenuItem->Select();
		}
		else if (args.Key == KeyboardKey::Escape)
		{
			lastMenuItem->Quit();
		}
	}

	MenuItemReactor* MenuBoxReactor::GetLastMenuItem() const
	{
		auto activeMenuItemReactor = (MenuItemReactor*)this;
		while (activeMenuItemReactor->Next() != nullptr)
		{
			activeMenuItemReactor = activeMenuItemReactor->Next();
		}
		return activeMenuItemReactor;
	}

	bool MenuBoxReactor::OnClickSubMenu(const ArgMouse& args)
	{
		int selectedIndex = -1;
		for (size_t i = 0; i < m_itemSizePositions.size(); i++)
		{
			auto& item = m_itemSizePositions[i];
			if (!m_items->at(i)->isSpearator && Rectangle { item.m_position, item.m_size }.IsInside(args.Position))
			{
				selectedIndex = static_cast<int>(i);
				break;
			}
		}

		if (selectedIndex == -1)
		{
			return false;
		}

		return (m_items->at(selectedIndex)->m_subMenu != nullptr);
	}

	Window* MenuBoxReactor::Owner() const
	{
		return m_control->Handle();
	}

	void MenuBoxReactor::MoveToNextItem(bool upwards)
	{
		if (m_items->empty())
		{
			return;
		}
		auto selectedIndex = m_selectedIndex;
		int direction = upwards ? -1 : 1;
		int totalItems = static_cast<int>(m_items->size());
		if (selectedIndex == -1)
		{
			selectedIndex = upwards ? totalItems - 1 : 0;
		}
		else
		{
			selectedIndex = ((selectedIndex + direction + totalItems) % totalItems);
		}
		auto savedIndex = selectedIndex;
		auto item = m_items->at(selectedIndex);
		while (selectedIndex >= 0 && (!item->isEnabled || item->isSpearator))
		{
			selectedIndex = ((selectedIndex + direction + totalItems) % totalItems);
			if (selectedIndex == savedIndex)
				break;

			item = m_items->at(selectedIndex);
		}

		if (m_selectedIndex != selectedIndex)
		{
			m_selectedIndex = selectedIndex;

			Update(m_control->Handle()->Renderer.GetGraphics());
			GUI::RefreshWindow(m_control->Handle());
		}
	}

	bool MenuBoxReactor::ExitSubMenu()
	{
		m_subMenuTimer.Stop();
		if (m_menuOwner->m_parentMenu == nullptr && (m_items->empty() || m_selectedIndex == -1))
		{
			return false;
		}

		if (m_menuOwner->m_parentMenu)
		{
			GUI::DisposeMenu(this);
			return true;
		}

		return false;
	}

	bool MenuBoxReactor::EnterSubMenu()
	{
		if (m_items->empty() || m_selectedIndex == -1)
		{
			return false;
		}
		auto& item = m_items->at(m_selectedIndex);
		if (!item->m_subMenu)
		{
			return false;
		}
		m_openedSubMenuIndex = m_selectedIndex;
		m_selectedSubMenuIndex = m_selectedIndex;
		m_subMenuTimer.Stop();
		OpenSubMenu(item->m_subMenu, m_menuOwner, m_openedSubMenuIndex, false);

		return true;
	}

	void MenuBoxReactor::Select()
	{
		size_t selectedIndex = static_cast<size_t>(m_selectedIndex);
		if (selectedIndex >= m_items->size())
		{
			return;
		}

		auto& item = m_items->at(selectedIndex);
		if (item->m_subMenu)
		{
			if (!item->m_subMenu->m_menuBox)
			{
				m_selectedSubMenuIndex = m_selectedIndex;
				m_openedSubMenuIndex = m_selectedIndex;
				OpenSubMenu(item->m_subMenu, m_menuOwner, m_selectedIndex);
				m_subMenuTimer.Stop();
			}
			return;
		}

		if (!item->isSpearator && item->onClick)
		{
			MenuItem menuItem(item);
			item->onClick(menuItem);
		}

		GUI::DisposeMenu();
	}

	void MenuBoxReactor::Quit()
	{
		GUI::DisposeMenu(this);
	}

	void MenuBoxReactor::SetItems(std::vector<Menu::Item*>& items)
	{
		m_items = &items;
		m_itemSizePositions.clear();

		auto window = m_control->Handle();
		auto menuBoxLeftPaneWidth = window->ToScale(window->Appereance->MenuBoxLeftPaneWidth);
		auto itemTextPadding = window->ToScale(ItemTextPadding);
		auto menuBoxItemHeight = window->ToScale(window->Appereance->MenuBoxItemHeight);
		auto separatorHeight = window->ToScale(SeparatorHeight);

		Point position{ 1 + (int)itemTextPadding, 1 + (int)itemTextPadding };
		uint32_t separators = 0;
		uint32_t maxWidth = 0;
		uint32_t sizeOfNormalItem = window->Size.Width - 2u - itemTextPadding * 2u;

		for (size_t i = 0; i < m_items->size(); i++)
		{
			auto& item = m_items->at(i);
			auto& itemSizePosition = m_itemSizePositions.emplace_back();
			if (item->isSpearator)
			{
				itemSizePosition.m_position = position;
				itemSizePosition.m_size = { sizeOfNormalItem , menuBoxItemHeight };

				position.Y += separatorHeight;
			}
			else
			{
				itemSizePosition.m_position = position;
				itemSizePosition.m_size = { sizeOfNormalItem , menuBoxItemHeight };

				position.Y += menuBoxItemHeight;
			}
		}
	}

	void MenuBoxReactor::SetMenuOwner(Menu* menuOwner)
	{
		m_menuOwner = menuOwner;
	}

	Size MenuBoxReactor::GetMenuBoxSize()
	{
		auto parent = m_control->Handle();

		Size size;
		uint32_t separators = 0;
		uint32_t maxWidth = 0;
		bool hasSubmenu = false;
		for (size_t i = 0; i < m_items->size(); i++)
		{
			if (m_items->at(i)->isSpearator)
			{
				++separators;
			}
			else
			{
				auto textSize = parent->Renderer.GetGraphics().GetTextExtent((m_items->at(i)->text));
				maxWidth = (std::max)(maxWidth, textSize.Width);
				hasSubmenu |= m_items->at(i)->m_subMenu != nullptr;
			}
		}

		auto menuBoxLeftPaneWidth = parent->ToScale(parent->Appereance->MenuBoxLeftPaneWidth);
		auto itemTextPadding = parent->ToScale(ItemTextPadding);
		auto menuBoxSubMenuArrowWidth = hasSubmenu ? parent->ToScale(parent->Appereance->MenuBoxSubMenuArrowWidth) : 0;
		auto separatorHeight = parent->ToScale(SeparatorHeight);
		auto menuBoxItemHeight = parent->ToScale(parent->Appereance->MenuBoxItemHeight);
		auto menuBoxShortcutWidth = parent->ToScale(parent->Appereance->MenuBoxShortcutWidth);

		return {
			2 + menuBoxLeftPaneWidth + maxWidth + itemTextPadding * 2u + menuBoxSubMenuArrowWidth + menuBoxShortcutWidth,
			2 + itemTextPadding * 2u + (uint32_t)(m_items->size() - separators) * (menuBoxItemHeight)+separators * separatorHeight
		};
	}

	void MenuBoxReactor::OpenSubMenu(Menu* subMenu, Menu* parentMenu, int selectedIndex, bool ignoreFirstMouseUp)
	{
		auto window = m_control->Handle();
		int two = window->ToScale(2);
		int four = window->ToScale(4);
		auto pointInScreen = GUI::GetPointClientToScreen(window, window->Position);

		subMenu->m_parentMenu = parentMenu;

		Point position
		{
			pointInScreen.X + (int)m_control->GetSize().Width - four,
			pointInScreen.Y + m_itemSizePositions[selectedIndex].m_position.Y
		};
		subMenu->ShowPopup(window, position, m_menuOwner, ignoreFirstMouseUp);

		m_next = subMenu->m_menuBox->GetItemReactor();
		m_next->Prev(this);
		subMenu->m_menuBox->GetEvents().Destroy.Connect([this](const ArgDestroy& args)
		{
			m_openedSubMenuIndex = -1;
			m_next = nullptr;
		});
	}

	int MenuBoxReactor::FindItem(const ArgMouse& args)
	{
		if (!Rectangle{ m_control->Handle()->Size}.IsInside(args.Position))
		{
			return -1;
		}

		for (size_t i = 0; i < m_itemSizePositions.size(); i++)
		{
			auto& item = m_itemSizePositions[i];
			if (!m_items->at(i)->isSpearator && Rectangle { item.m_position, item.m_size }.IsInside(args.Position))
			{
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	bool MenuBoxReactor::MouseMoveInternal(const ArgMouse& args)
	{
		int selectedIndex = FindItem(args);
		if (selectedIndex == m_selectedIndex)
		{
			return false;
		}

		if (selectedIndex != -1 && !m_items->at(selectedIndex)->isEnabled)
		{
			if (m_openedSubMenuIndex != -1)
			{
				m_selectedSubMenuIndex = -1;
				m_openedSubMenuIndex = -1;
				m_subMenuTimer.Stop();

				GUI::DisposeMenu(m_next);
				m_next = nullptr;
			}
			selectedIndex = -1;
		}
		else
		{
			if (selectedIndex != -1 && m_items->at(selectedIndex)->m_subMenu)
			{
				auto& subMenu = m_items->at(selectedIndex)->m_subMenu;
				if (!subMenu->m_menuBox)
				{
					if (m_openedSubMenuIndex == -1)
					{
						m_selectedSubMenuIndex = selectedIndex;
						if (m_selectedSubMenuIndex >= 0)
						{
							m_subMenuTimer.Stop();
						}
						m_subMenuTimer.Start();
					}
					else
					{
						GUI::DisposeMenu(m_next);
						m_next = nullptr;

						m_openedSubMenuIndex = -1;
						m_selectedSubMenuIndex = selectedIndex;
						m_subMenuTimer.Start();
					}
				}
			}
			else if (selectedIndex != -1 && m_openedSubMenuIndex != -1 && !m_items->at(selectedIndex)->m_subMenu)
			{
				m_selectedSubMenuIndex = -1;
				m_openedSubMenuIndex = -1;
				m_subMenuTimer.Stop();
				GUI::DisposeMenu(m_next);
				m_next = nullptr;
			}
			else if (m_selectedSubMenuIndex != -1 && m_selectedSubMenuIndex != m_openedSubMenuIndex)
			{
				m_subMenuTimer.Stop();
				m_selectedSubMenuIndex = -1;
			}
			else if (selectedIndex == -1 && m_openedSubMenuIndex != -1)
			{
				auto& openedSubItem = m_itemSizePositions.at(m_openedSubMenuIndex);
				auto tolerance = (int)(openedSubItem.m_size.Height >> 2);
				if (args.Position.Y >= (openedSubItem.m_position.Y - tolerance) && args.Position.Y <= (openedSubItem.m_position.Y + tolerance) + (int)openedSubItem.m_size.Height)
				{
					BT_CORE_TRACE << "      - ...." << std::endl;
					selectedIndex = m_openedSubMenuIndex;
				}
				else
				{
					selectedIndex = m_openedSubMenuIndex;
				}
			}
		}
		
		bool hasChanged = selectedIndex != m_selectedIndex;
		m_selectedIndex = selectedIndex;
		return hasChanged;
	}

	MenuBox::MenuBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle, { false, false, false, false, true, false });
		GUI::MakeWindowActive(m_handle, false);

#if BT_DEBUG
		std::ostringstream builder;
		builder << "Menu box" << g_globalId;
		SetDebugName(builder.str());
		++g_globalId;
#endif
	}

	MenuBox::~MenuBox()
	{
#if BT_DEBUG
		--g_globalId;
#endif
	}

	void MenuBox::Init(Menu* menuOwner, std::vector<Menu::Item*>& items, const Rectangle& rect)
	{
		m_reactor.SetItems(items);
		m_reactor.SetMenuBarItemRect(rect);
		m_reactor.SetMenuOwner(menuOwner);
	}

	void MenuBox::SetIgnoreFirstMouseUp(bool value)
	{
		m_reactor.SetIgnoreFirstMouseUp(value);
	}

	void MenuBox::Popup()
	{
		GUI::Capture(m_handle);
		Show();
	}

	Size MenuBox::GetMenuBoxSize()
	{
		return m_reactor.GetMenuBoxSize();
	}

	bool MenuItem::GetEnabled() const
	{
		return m_target->isEnabled;
	}

	void MenuItem::SetEnabled(bool isEnabled)
	{
		m_target->isEnabled = isEnabled;
	}

	void MenuItem::SetText(const std::wstring& text)
	{
		m_target->text = text;
	}
}