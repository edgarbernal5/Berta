/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Menu.h"

#include <utility>

#include "Berta/GUI/Interface.h"
#include "Berta/Controls/Menu.h"
#include "Berta/GUI/EnumTypes.h"

#include "Berta/Core/Foundation.h"

namespace Berta
{
#if BT_DEBUG
	int MenuBox::g_globalId = 0;
#endif
	
	void Menu::Append(const std::string& text, ClickCallback onClick)
	{
		std::wstring wstr(text.begin(), text.end());
		auto& newItem = m_items.emplace_back(new Menu::Item{ wstr , std::move(onClick)});
	}

	void Menu::Append(const std::wstring& text, ClickCallback onClick)
	{
		auto& newItem = m_items.emplace_back(new Menu::Item{ text , std::move(onClick)});
	}

	void Menu::AppendSeparator()
	{
		m_items.emplace_back(new Menu::Item());
	}

	void Menu::ShowPopup(Window* owner, const Point& position, bool fromMenuBar, bool ignoreFirstMouseUp)
	{
		m_parentWindow = owner;

		m_menuBox = new Berta::MenuBox(owner, position);
		m_menuBox->Init(this, m_items);
		m_menuBox->SetIgnoreFirstMouseUp(ignoreFirstMouseUp);

		m_menuBox->GetEvents().Destroy.Connect([this](const ArgDestroy& argDestroy)
		{
			delete m_menuBox;
			m_menuBox = nullptr;

			if (m_destroyCallback)
			{
				m_destroyCallback();
			}
		});
	
		m_menuBox->Popup(fromMenuBar);
	}

	void Menu::ShowPopup(Window* owner, const ArgMouse& args)
	{
		if (!args.ButtonState.RightButton)
		{
			return;
		}
		auto screenPosition = args.Position;
		ShowPopup(owner, screenPosition, false);
	}

	Menu* Menu::CreateSubMenu(std::size_t index)
	{
		if (index < m_items.size())
		{
			auto& menuItem = m_items.at(index);
			if (!menuItem->m_subMenu)
			{
				menuItem->m_subMenu = std::make_unique<Menu>();
			}

			return menuItem->m_subMenu.get();
		}
		return nullptr;
	}

	void Menu::SetImage(size_t index, const Image& image)
	{
		m_items.at(index)->m_image = image;
	}

	void Menu::SetEnabled(size_t index, bool enabled)
	{
		m_items.at(index)->m_isEnabled = enabled;
	}

	void Menu::CloseMenuBox()
	{
		if (!m_menuBox)
			return;

		//GUI::ReleaseCapture(m_menuBox->Handle());
		m_menuBox->Dispose();
		m_menuBox = nullptr;
	}

	Size Menu::GetMenuBoxSize(Window* parent) const
	{
		uint32_t separators = 0;
		uint32_t maxWidth = 0;
		bool hasSubmenu = false;
		for (size_t i = 0; i < m_items.size(); i++)
		{
			if (m_items[i]->m_isSeparator)
			{
				++separators;
			}
			else
			{
				auto textSize = parent->Renderer.GetGraphics().GetTextExtent((m_items[i]->m_text));
				maxWidth = (std::max)(maxWidth, textSize.Width);
				hasSubmenu |= m_items[i]->m_subMenu != nullptr;
			}
		}
	
		auto menuBoxLeftPaneWidth = parent->ToScale(m_menuBox->GetAppearance().MenuBoxLeftPaneWidth);
		auto itemTextPadding = parent->ToScale(ItemTextPadding);
		auto menuBoxSubMenuArrowWidth = hasSubmenu ? parent->ToScale(m_menuBox->GetAppearance().MenuBoxSubMenuArrowWidth) : 0;
		auto separatorHeight = parent->ToScale(SeparatorHeight);
		auto menuBoxItemHeight = parent->ToScale(m_menuBox->GetAppearance().MenuBoxItemHeight);
		auto menuBoxShortcutWidth = parent->ToScale(m_menuBox->GetAppearance().MenuBoxShortcutWidth);

		return { 
			2 + menuBoxLeftPaneWidth + maxWidth + itemTextPadding * 2u + menuBoxSubMenuArrowWidth + menuBoxShortcutWidth,
			2 + itemTextPadding * 2u + static_cast<uint32_t>(m_items.size() - separators) * (menuBoxItemHeight) + separators * separatorHeight
		};
	}
	
	namespace ReactorCore::MenuBox
	{
		void Reactor::Init(ControlBase& control, Graphics* graphics)
		{
			ControlReactor::Init(control, graphics);
			m_menuBox = reinterpret_cast<Berta::MenuBox*>(&control);
			m_appearance = reinterpret_cast<Appearance*>(m_menuBox->Handle()->Appearance.get());

			m_subMenuTimer.SetOwner(m_control->Handle());
			m_subMenuTimer.SetInterval(400);
			m_subMenuTimer.Connect([this](const ArgTimer& args)
			{
				//opens submenu
				if (m_selectedSubMenuIndex != -1 && m_items->at(m_selectedSubMenuIndex)->m_subMenu)
				{
					if (m_openedSubMenuIndex != m_selectedIndex && m_openedSubMenuIndex >= 0)
					{
						auto subMenu = m_items->at(m_openedSubMenuIndex)->m_subMenu.get();

						GUI::DisposeMenu(m_next);
						m_next = nullptr;
					}
					auto subMenu = m_items->at(m_selectedSubMenuIndex)->m_subMenu.get();
					if (!subMenu->m_menuBox)
					{
						m_openedSubMenuIndex = m_selectedSubMenuIndex;
						OpenSubMenu(subMenu, m_menuOwner, m_openedSubMenuIndex, false);
						m_subMenuTimer.Stop();
					}
				}
				else if (m_selectedSubMenuIndex == -1 && m_openedSubMenuIndex >= 0)
				{
					auto subMenu = m_items->at(m_openedSubMenuIndex)->m_subMenu.get();
					GUI::DisposeMenu(m_next);
					m_next = nullptr;
					m_openedSubMenuIndex = -1;
				}
				m_subMenuTimer.Stop();
			});
		}

		void Reactor::Update(Graphics& graphics)
		{
			auto window = m_control->Handle();

			graphics.DrawRectangle(window->Appearance->MenuBackground, true);

			auto menuBoxLeftPaneWidth = window->ToScale(m_appearance->MenuBoxLeftPaneWidth);
			auto itemTextPadding = window->ToScale(ItemTextPadding);
			auto menuBoxItemHeight = window->ToScale(m_appearance->MenuBoxItemHeight);
			auto menuBoxSubMenuArrowWidth = window->ToScale(m_appearance->MenuBoxSubMenuArrowWidth);
			auto separatorHeight = window->ToScale(SeparatorHeight);
			auto smallIconSize = window->ToScale(window->Appearance->SmallIconSize);

			if (m_items)
			{
				int offsetY = 1 + static_cast<int>(itemTextPadding);
				for (size_t i = 0; i < m_items->size(); i++)
				{
					auto& item = *(m_items->at(i));
					if (item.m_isSeparator)
					{
						int separatorCenterOffset = (separatorHeight >> 1) - 1;
						graphics.DrawLine({ 1 + static_cast<int>(menuBoxLeftPaneWidth) - 4, offsetY + separatorCenterOffset + 1 }, { static_cast<int>(window->ClientSize.Width) - 2, offsetY + separatorCenterOffset + 1 }, window->Appearance->BoxBorderColor);
						offsetY += separatorHeight;
					}
					else
					{
						auto& textSize = graphics.GetTextExtent();
						int center = (int)menuBoxItemHeight - (int)textSize.Height;
						center >>= 1;

						bool isItemSelected = m_selectedIndex == (int)i;
						if (isItemSelected)
						{
							graphics.DrawRectangle({ 1 + (int)(itemTextPadding), offsetY, window->ClientSize.Width - 2u - itemTextPadding * 2u, menuBoxItemHeight }, window->Appearance->HighlightColor, true);
						}
						if (item.m_image)
						{
							Point paneSize{ (int)menuBoxLeftPaneWidth, (int)menuBoxItemHeight };
							Size scaleImageSize{ smallIconSize , smallIconSize };
							Point imageSize{ (int)scaleImageSize.Width, (int)scaleImageSize.Height };
							Point centerImage = paneSize - imageSize;
							centerImage /= 2;
							Rectangle destRect{ { 1 + centerImage.X + (int)itemTextPadding, offsetY + centerImage.Y }, scaleImageSize };
							item.m_image.Paste(item.m_image.GetSize().ToRectangle(), graphics, destRect);
						}
						auto textPosition = Point{ 1 + (int)(menuBoxLeftPaneWidth + itemTextPadding), offsetY + center };
						graphics.DrawString(textPosition, item.m_text, item.m_isEnabled ? (window->Appearance->Foreground) : window->Appearance->BoxBorderDisabledColor);
						if (item.m_isEnabled && item.m_accessKey)
						{
							GUI::DrawAccessKeyUnderline(graphics, item.m_text, item.m_accessKey, item.m_accessKeyPosition, textPosition, window->Appearance->Foreground);
						}

						if (item.m_subMenu)
						{
							int arrowWidth = window->ToScale(4);
							int arrowLength = window->ToScale(2);
							graphics.DrawArrow({ static_cast<int>(window->ClientSize.Width - menuBoxSubMenuArrowWidth) , offsetY, menuBoxSubMenuArrowWidth, menuBoxItemHeight },
								arrowLength,
								arrowWidth,
								Graphics::ArrowDirection::Right,
								item.m_isEnabled ? (window->Appearance->Foreground) : window->Appearance->BoxBorderDisabledColor,
								true,
								item.m_isEnabled ? (window->Appearance->Foreground) : window->Appearance->BoxBorderDisabledColor);
						}

						offsetY += menuBoxItemHeight;
					}
				}
			}

			graphics.DrawRectangle(window->Appearance->BoxBorderColor, false);
		}

		void Reactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
		{
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			bool changes = MouseMoveInternal(args);
			if (changes)
			{
				auto window = m_control->Handle();
				GUI::MarkAsNeedUpdate(window);
			}
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			if (!args.ButtonState.LeftButton)
			{
				return;
			}

			int selectedIndex = FindItem(args);
			if (!m_control->Handle()->ClientSize.IsInside(args.Position) && selectedIndex == -1)
			{
				GUI::DisposeMenu();
			}
			else if (m_selectedIndex != -1 && m_items->at(m_selectedIndex)->m_subMenu)
			{
				auto subMenu = m_items->at(m_selectedIndex)->m_subMenu.get();
				if (!subMenu->m_menuBox)
				{
					m_subMenuTimer.Stop();
					m_selectedSubMenuIndex = m_selectedIndex;
					m_openedSubMenuIndex = m_selectedIndex;
					OpenSubMenu(subMenu, m_menuOwner, m_selectedIndex, m_ignoreFirstMouseUp);
				}
			}
		 
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			bool changes = MouseMoveInternal(args);
			if (changes)
			{
				auto window = m_control->Handle();
				GUI::MarkAsNeedUpdate(window);
			}
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			//BT_CORE_DEBUG << " MenuBoxReactor MouseUp(). " << m_ignoreFirstMouseUp << std::endl;
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

			GUI::DisposeMenu();

			if (!item->m_isSeparator && item->m_onClick)
			{
				MenuItem menuItem(*item);
				item->m_onClick(menuItem);
			}
		}

		void Reactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
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

		MenuItemReactor* Reactor::GetLastMenuItem() const
		{
			auto activeMenuItemReactor = (MenuItemReactor*)this;
			while (activeMenuItemReactor->Next() != nullptr)
			{
				activeMenuItemReactor = activeMenuItemReactor->Next();
			}
			return activeMenuItemReactor;
		}

		bool Reactor::OnClickSubMenu(const ArgMouse& args)
		{
			int selectedIndex = -1;
			for (size_t i = 0; i < m_itemSizePositions.size(); i++)
			{
				auto& item = m_itemSizePositions[i];
				if (!m_items->at(i)->m_isSeparator && Rectangle { item.m_position, item.m_size }.IsInside(args.Position))
				{
					selectedIndex = static_cast<int>(i);
					break;
				}
			}

			if (selectedIndex == -1)
			{
				return false;
			}

			return m_items->at(selectedIndex)->m_subMenu != nullptr;
		}

		Window* Reactor::Owner() const
		{
			return m_control->Handle();
		}

		void Reactor::MoveToNextItem(bool upwards)
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
			auto item = m_items->at(selectedIndex).get();
			while (selectedIndex >= 0 && (!item->m_isEnabled || item->m_isSeparator))
			{
				selectedIndex = ((selectedIndex + direction + totalItems) % totalItems);
				if (selectedIndex == savedIndex)
				{
					break;
				}

				item = m_items->at(selectedIndex).get();
			}

			if (m_selectedIndex != selectedIndex)
			{
				m_selectedIndex = selectedIndex;

				GUI::UpdateWindow(*m_control);
			}
		}

		bool Reactor::ExitSubMenu()
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

		bool Reactor::EnterSubMenu()
		{
			if (m_items->empty() || m_selectedIndex == -1)
			{
				return false;
			}
			auto item = m_items->at(m_selectedIndex).get();
			if (!item->m_subMenu)
			{
				return false;
			}
			m_openedSubMenuIndex = m_selectedIndex;
			m_selectedSubMenuIndex = m_selectedIndex;
			m_subMenuTimer.Stop();
			OpenSubMenu(item->m_subMenu.get(), m_menuOwner, m_openedSubMenuIndex, false);

			return true;
		}

		void Reactor::Select()
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
					OpenSubMenu(item->m_subMenu.get(), m_menuOwner, m_selectedIndex);
					m_subMenuTimer.Stop();
				}
				return;
			}

			GUI::DisposeMenu();

			if (!item->m_isSeparator && item->m_onClick)
			{
				MenuItem menuItem(*item);
				item->m_onClick(menuItem);
			}
		}

		void Reactor::Quit()
		{
			GUI::DisposeMenu(this);
		}

		void Reactor::BuildItems()
		{
			m_itemSizePositions.clear();

			auto window = m_control->Handle();
			auto menuBoxLeftPaneWidth = window->ToScale(m_appearance->MenuBoxLeftPaneWidth);
			auto itemTextPadding = window->ToScale(ItemTextPadding);
			auto menuBoxItemHeight = window->ToScale(m_appearance->MenuBoxItemHeight);
			auto separatorHeight = window->ToScale(SeparatorHeight);

			Point position{ 1 + static_cast<int>(itemTextPadding), 1 + static_cast<int>(itemTextPadding) };
			uint32_t separators = 0;
			uint32_t maxWidth = 0;
			uint32_t sizeOfNormalItem = window->ClientSize.Width - 2u - itemTextPadding * 2u;

			for (size_t i = 0; i < m_items->size(); i++)
			{
				auto& item = m_items->at(i);
				auto& itemSizePosition = m_itemSizePositions.emplace_back();
				if (item->m_isSeparator)
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

		void Reactor::SetItems(std::vector<std::unique_ptr<Menu::Item>>& items)
		{
			m_items = &items;
		}

		void Reactor::SetMenuOwner(Menu* menuOwner)
		{
			m_menuOwner = menuOwner;
		}

		Size Reactor::GetMenuBoxSize()
		{
			auto parent = m_control->Handle();

			Size size;
			uint32_t separators = 0;
			uint32_t maxWidth = 0;
			bool hasSubmenu = false;
			for (size_t i = 0; i < m_items->size(); i++)
			{
				if (m_items->at(i)->m_isSeparator)
				{
					++separators;
				}
				else
				{
					auto textSize = parent->Renderer.GetGraphics().GetTextExtent((m_items->at(i)->m_text));
					maxWidth = (std::max)(maxWidth, textSize.Width);
					hasSubmenu |= m_items->at(i)->m_subMenu != nullptr;
				}
			}

			auto menuBoxLeftPaneWidth = parent->ToScale(m_menuBox->GetAppearance().MenuBoxLeftPaneWidth);
			auto itemTextPadding = parent->ToScale(ItemTextPadding);
			auto menuBoxSubMenuArrowWidth = hasSubmenu ? parent->ToScale(m_menuBox->GetAppearance().MenuBoxSubMenuArrowWidth) : 0;
			auto separatorHeight = parent->ToScale(SeparatorHeight);
			auto menuBoxItemHeight = parent->ToScale(m_menuBox->GetAppearance().MenuBoxItemHeight);
			auto menuBoxShortcutWidth = parent->ToScale(m_menuBox->GetAppearance().MenuBoxShortcutWidth);

			return {
				2 + menuBoxLeftPaneWidth + maxWidth + itemTextPadding * 2u + menuBoxSubMenuArrowWidth + menuBoxShortcutWidth,
				2 + itemTextPadding * 2u + (uint32_t)(m_items->size() - separators) * (menuBoxItemHeight) + separators * separatorHeight
			};
		}

		void Reactor::OpenSubMenu(Menu* subMenu, Menu* parentMenu, int selectedIndex, bool ignoreFirstMouseUp)
		{
			auto window = m_control->Handle();
			int two = window->ToScale(2);
			int four = window->ToScale(4);

			subMenu->m_parentMenu = parentMenu;
			Point position
			{
				(int)m_control->GetSize().Width - four,
				m_itemSizePositions[selectedIndex].m_position.Y
			};
			subMenu->ShowPopup(window, position, !m_menuOwner, ignoreFirstMouseUp);

			m_next = subMenu->m_menuBox->GetItemReactor();
			m_next->m_prev = this;

			subMenu->m_menuBox->GetEvents().Destroy.Connect([this](const ArgDestroy& args)
			{
				m_openedSubMenuIndex = -1;
				m_next = nullptr;
			});
		}

		int Reactor::FindItem(const ArgMouse& args)
		{
			if (!Rectangle{ m_control->Handle()->ClientSize }.IsInside(args.Position))
			{
				return -1;
			}

			for (size_t i = 0; i < m_itemSizePositions.size(); i++)
			{
				auto& item = m_itemSizePositions[i];
				if (!m_items->at(i)->m_isSeparator && Rectangle { item.m_position, item.m_size }.IsInside(args.Position))
				{
					return static_cast<int>(i);
				}
			}
			return -1;
		}

		bool Reactor::MouseMoveInternal(const ArgMouse& args)
		{
			int selectedIndex = FindItem(args);
			if (selectedIndex == m_selectedIndex)
			{
				return false;
			}

			if (selectedIndex != -1 && !m_items->at(selectedIndex)->m_isEnabled)
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
	}
	
	MenuBox::MenuBox(Window* parent, const Point& position)
	{
		Create(parent, false, { position.X, position.Y, 1, 1 }, FormStyle::Float(false), false);
		GUI::MakeWindowActive(m_handle, false, nullptr);

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

	void MenuBox::Init(Menu* menuOwner, std::vector<std::unique_ptr<Menu::Item>>& items)
	{
		menuOwner->m_menuBox = this;

		GetReactor().SetItems(items);
		GetReactor().SetMenuOwner(menuOwner);

		auto boxSize = GetMenuBoxSize();
		SetSize(boxSize);

		GetReactor().BuildItems();
	}

	void MenuBox::SetIgnoreFirstMouseUp(bool value)
	{
		GetReactor().SetIgnoreFirstMouseUp(value);
	}

	void MenuBox::Popup(bool fromMenuBar)
	{
		auto& menuManager = Foundation::GetInstance().GetMenuManager();
		menuManager.ShowPopup(m_handle, GetOwner(), fromMenuBar);
		Show();
	}

	Size MenuBox::GetMenuBoxSize()
	{
		return GetReactor().GetMenuBoxSize();
	}

	bool MenuBox::MenuItem::GetEnabled() const
	{
		return m_target.m_isEnabled;
	}

	void MenuBox::MenuItem::SetEnabled(bool isEnabled)
	{
		m_target.m_isEnabled = isEnabled;
	}

	void MenuBox::MenuItem::SetText(const std::wstring& text)
	{
		m_target.m_text = text;
	}
}