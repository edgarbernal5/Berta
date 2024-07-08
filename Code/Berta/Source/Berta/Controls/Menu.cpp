/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Menu.h"

#include "Berta/GUI/Interface.h"
#include "Berta/Controls/Menu.h"

namespace Berta
{
	void Menu::Append(const std::wstring& text, ClickCallback onClick)
	{
		auto& newItem = m_items.emplace_back(new Menu::Item{ text , onClick });
	}

	void Menu::AppendSeparator()
	{
		m_items.emplace_back(new Menu::Item());
	}

	void Menu::ShowPopup(Window* parentWindow, const Point& position, bool ignoreFirstMouseUp)
	{
		m_popupFromMenuBar = true;
		m_parentWindow = parentWindow;

		auto boxSize = GetMenuBoxSize(parentWindow);
		m_menuBox = new MenuBox(parentWindow, { position.X, position.Y, boxSize.Width, boxSize.Height });
		m_menuBox->Init(m_items);
		m_menuBox->SetIgnoreFirstMouseUp(ignoreFirstMouseUp);

		m_menuBox->GetEvents().Destroy.Connect([this](const ArgDestroy& argDestroy)
		{
			BT_CORE_TRACE << "   - menu box destroy callback..." << std::endl;
			delete m_menuBox;
			m_menuBox = nullptr;

			if (m_destroyCallback)
			{
				m_destroyCallback();
			}
		});

		m_menuBox->Show();
		GUI::Capture(m_menuBox->Handle());
	}

	Menu* Menu::CreateSubMenu(std::size_t index)
	{
		if (index >= 0 && index < m_items.size())
		{
			auto& menuItem = m_items.at(index);
			menuItem->m_subMenu = new Menu();

			return menuItem->m_subMenu;
		}
		return nullptr;
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

		auto menuBoxLeftPaneWidth = static_cast<uint32_t>(parent->Appereance->MenuBoxLeftPaneWidth * parent->DPIScaleFactor);
		auto itemTextPadding = static_cast<uint32_t>(ItemTextPadding * parent->DPIScaleFactor);
		auto menuBoxSubMenuArrowWidth = hasSubmenu ? static_cast<uint32_t>(parent->Appereance->MenuBoxSubMenuArrowWidth * parent->DPIScaleFactor) : 0;
		auto separatorHeight = static_cast<uint32_t>(SeparatorHeight * parent->DPIScaleFactor);
		auto menuBoxItemHeight = static_cast<uint32_t>(parent->Appereance->MenuBoxItemHeight * parent->DPIScaleFactor);
		auto menuBoxShorcutWidth = static_cast<uint32_t>(parent->Appereance->MenuBoxShortcutWidth * parent->DPIScaleFactor);

		return { 
			2 + menuBoxLeftPaneWidth + maxWidth + itemTextPadding * 2u + menuBoxSubMenuArrowWidth + menuBoxShorcutWidth,
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
					GUI::DisposeMenu(subMenu->m_menuBox->GetItemReactor());
					m_next = nullptr;
				}
				auto subMenu = m_items->at(m_selectedSubMenuIndex)->m_subMenu;
				if (!subMenu->m_menuBox)
				{
					m_openedSubMenuIndex = m_selectedSubMenuIndex;
					OpenSubMenu(subMenu);
					m_subMenuTimer.Stop();
				}
			}
			else if (m_selectedSubMenuIndex == -1 && m_openedSubMenuIndex >= 0)
			{
				auto subMenu = m_items->at(m_openedSubMenuIndex)->m_subMenu;
				GUI::DisposeMenu(subMenu->m_menuBox->GetItemReactor());
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

		auto menuBoxLeftPaneWidth = static_cast<uint32_t>(window->Appereance->MenuBoxLeftPaneWidth * window->DPIScaleFactor);
		auto itemTextPadding = static_cast<uint32_t>(ItemTextPadding * window->DPIScaleFactor);
		auto menuBoxItemHeight = static_cast<uint32_t>(window->Appereance->MenuBoxItemHeight * window->DPIScaleFactor);
		auto menuBoxSubMenuArrowWidth = static_cast<uint32_t>(window->Appereance->MenuBoxSubMenuArrowWidth * window->DPIScaleFactor);
		auto separatorHeight = static_cast<uint32_t>(SeparatorHeight * window->DPIScaleFactor);

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
					graphics.DrawString({ 1 + (int)(menuBoxLeftPaneWidth + itemTextPadding), offsetY + center}, item.text, isItemSelected ? window->Appereance->HighlightTextColor : window->Appereance->Foreground);
					
					if (item.m_subMenu)
					{
						int arrowWidth = static_cast<int>(4 * window->DPIScaleFactor);
						int arrowLength = static_cast<int>(2 * window->DPIScaleFactor);
						graphics.DrawArrow({ static_cast<int>(window->Size.Width - menuBoxSubMenuArrowWidth) , offsetY, menuBoxSubMenuArrowWidth, menuBoxItemHeight },
							arrowLength,
							arrowWidth,
							isItemSelected ? window->Appereance->HighlightTextColor : window->Appereance->Foreground,
							Graphics::ArrowDirection::Right,
							true);
					}

					offsetY += menuBoxItemHeight;
				}
			}
		}

		graphics.DrawRectangle(window->Appereance->BoxBorderColor, false);
	}

	void MenuBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (m_selectedIndex != -1 && m_items->at(m_selectedIndex)->m_subMenu)
		{
			auto subMenu = m_items->at(m_selectedIndex)->m_subMenu;
			if (!subMenu->m_menuBox)
			{
				m_selectedSubMenuIndex = m_selectedIndex;
				m_openedSubMenuIndex = m_selectedIndex;
				OpenSubMenu(subMenu);
				m_subMenuTimer.Stop();
			}
		}
	}

	void MenuBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto window = m_control->Handle();
		BT_CORE_TRACE << " MouseMove " << std::endl;
		bool changes = MouseMoveInternal(args);
		if (changes)
		{
			Update(graphics);
			GUI::UpdateDeferred(window);
		}
	}

	void MenuBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (m_ignoreFirstMouseUp)
		{
			m_ignoreFirstMouseUp = false;
			return;
		}

		if (m_selectedIndex != -1 && m_items->at(m_selectedIndex)->m_subMenu)
		{
			return;
		}

		GUI::DisposeMenu(true);
	}

	bool MenuBoxReactor::OnCheckMenuItemMouseMove(const ArgMouse& args)
	{
		auto window = m_control->Handle();
		if (!Rectangle{ 0,0, window->Size.Width, window->Size.Height }.IsInside(args.Position))
			return false;

		return true;
	}

	void MenuBoxReactor::OnMenuItemMouseMove(const ArgMouse& args)
	{
		auto window = m_control->Handle();

		BT_CORE_TRACE << " OnMenuItemMouseMove " << std::endl;
		bool changes = MouseMoveInternal(args);
		if (changes)
		{
			Update(window->Renderer.GetGraphics());
			GUI::RefreshWindow(window);
		}
	}

	Window* MenuBoxReactor::Owner() const
	{
		return m_control->Handle();
	}

	void MenuBoxReactor::SetItems(std::vector<Menu::Item*>& items)
	{
		m_items = &items;
		m_itemSizePositions.clear();

		auto window = m_control->Handle();
		auto menuBoxLeftPaneWidth = static_cast<uint32_t>(window->Appereance->MenuBoxLeftPaneWidth * window->DPIScaleFactor);
		auto itemTextPadding = static_cast<uint32_t>(ItemTextPadding * window->DPIScaleFactor);
		auto menuBoxItemHeight = static_cast<uint32_t>(window->Appereance->MenuBoxItemHeight * window->DPIScaleFactor);
		auto separatorHeight = static_cast<uint32_t>(3u * window->DPIScaleFactor);

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

	void MenuBoxReactor::OpenSubMenu(Menu* subMenu)
	{
		auto window = m_control->Handle();
		int two = static_cast<int>(2 * window->DPIScaleFactor);
		int four = static_cast<int>(4 * window->DPIScaleFactor);
		auto pointInScreen = GUI::GetPointClientToScreen(window, window->Position);

		Point position{ pointInScreen.X + (int)m_control->GetSize().Width - four, pointInScreen.Y + two };
		subMenu->ShowPopup(window, position);

		m_next = subMenu->m_menuBox->GetItemReactor();
		subMenu->m_menuBox->GetItemReactor()->Prev(this);
		GUI::SetSubMenu(window, subMenu->m_menuBox->GetItemReactor());
	}

	bool MenuBoxReactor::MouseMoveInternal(const ArgMouse& args)
	{
		int selectedIndex = -1;
		for (size_t i = 0; i < m_itemSizePositions.size(); i++)
		{
			auto& item = m_itemSizePositions[i];
			if (!m_items->at(i)->isSpearator && Rectangle { item.m_position.X, item.m_position.Y, item.m_size.Width, item.m_size.Height }.IsInside(args.Position))
			{
				selectedIndex = (int)i;
				break;
			}
		}
		bool hasChanged = selectedIndex != m_selectedIndex;
		if (hasChanged)
		{
			BT_CORE_TRACE << "  cambio de indice. selected " << selectedIndex << " != m_selected " << m_selectedIndex << ". m_openedSubMenuIndex = " << m_openedSubMenuIndex << ". m_selectedSubMenuIndex = " << m_selectedSubMenuIndex << std::endl;
			
			m_selectedIndex = selectedIndex;
			
			if (selectedIndex == -1)
				BT_CORE_TRACE << "      - seleccion nula..." << std::endl;

			if (m_selectedIndex != -1 && m_items->at(m_selectedIndex)->m_subMenu)
			{
				auto& subMenu = m_items->at(m_selectedIndex)->m_subMenu;
				if (!subMenu->m_menuBox)
				{
					if (m_selectedSubMenuIndex >= 0)
					{
						m_selectedSubMenuIndex = m_selectedIndex;
						m_subMenuTimer.Stop();
					}
					else
					{
						m_selectedSubMenuIndex = m_selectedIndex;
						m_subMenuTimer.Start();
					}
				}
			}
			else if (m_selectedIndex == -1 || m_selectedSubMenuIndex != m_selectedIndex)
			{
				if (m_subMenuTimer.IsRunning())
				{
					m_subMenuTimer.Stop();
					m_selectedSubMenuIndex = -1;
				}
				else
				{
					m_selectedSubMenuIndex = -1;
					m_subMenuTimer.Start();
				}
			}
			else if (m_selectedIndex == -1 || m_openedSubMenuIndex != m_selectedIndex)
			{
				if (m_subMenuTimer.IsRunning())
				{
					m_subMenuTimer.Stop();

					GUI::DisposeMenu(m_next);
					m_next = nullptr;
					m_openedSubMenuIndex = -1;
					m_selectedSubMenuIndex = -1;
				}
			}
		}

		return hasChanged;
	}

	MenuBox::MenuBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle, { false, false, false, false, true, false });
		GUI::MakeWindowActive(m_handle, false);
#if BT_DEBUG
		SetDebugName("Menu box");
#endif
	}

	MenuBox::~MenuBox()
	{
	}

	void MenuBox::Init(std::vector<Menu::Item*>& items)
	{
		m_reactor.SetItems(items);
	}

	void MenuBox::SetIgnoreFirstMouseUp(bool value)
	{
		m_reactor.SetIgnoreFirstMouseUp(value);
	}
}