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

	void Menu::ShowPopup(Window* parent, const Point& position, bool ignoreFirstMouseUp)
	{
		m_popupFromMenuBar = true;
		m_parent = parent;

		auto boxSize = GetMenuBoxSize(parent);
		m_menuBox = new MenuBox(parent, { position.X, position.Y, boxSize.Width, boxSize.Height });
		m_menuBox->Init(m_items);
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
			}
		}

		auto menuBoxLeftPaneWidth = static_cast<uint32_t>(parent->Appereance->MenuBoxLeftPaneWidth * parent->DPIScaleFactor);
		auto itemTextPadding = static_cast<uint32_t>(ItemTextPadding * parent->DPIScaleFactor);
		auto menuBoxSubMenuArrowWidth = static_cast<uint32_t>(parent->Appereance->MenuBoxSubMenuArrowWidth * parent->DPIScaleFactor);
		auto separatorHeight = static_cast<uint32_t>(SeparatorHeight * parent->DPIScaleFactor);
		auto menuBoxItemHeight = static_cast<uint32_t>(parent->Appereance->MenuBoxItemHeight * parent->DPIScaleFactor);

		return { 
			2 + menuBoxLeftPaneWidth + maxWidth + itemTextPadding * 2u + menuBoxSubMenuArrowWidth,
			2 + itemTextPadding * 2u + (uint32_t)(m_items.size() - separators) * (menuBoxItemHeight) + separators * separatorHeight
		};
	}

	MenuBoxReactor::~MenuBoxReactor()
	{
	}

	void MenuBoxReactor::Init(ControlBase& control)
	{
		m_control = reinterpret_cast<MenuBox*>(&control);
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

	void MenuBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto window = m_control->Handle();

		int selectedIndex = -1;
		for (size_t i = 0; i < m_itemSizePositions.size(); i++)
		{
			auto& item = m_itemSizePositions[i];
			if (!m_items->at(i)->isSpearator && Rectangle{ item.m_position.X, item.m_position.Y, item.m_size.Width, item.m_size.Height }.IsInside(args.Position))
			{
				selectedIndex = (int)i;
				break;
			}
		}

		if (selectedIndex != m_selectedIndex)
		{
			m_selectedIndex = selectedIndex;

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

		m_control->Dispose();
	}

	void MenuBoxReactor::SetItems(std::vector<Menu::Item*>& items)
	{
		m_items = &items;
		m_itemSizePositions.clear();

		auto window = m_control->Handle();
		auto menuBoxLeftPaneWidth = static_cast<uint32_t>(window->Appereance->MenuBoxLeftPaneWidth * window->DPIScaleFactor);
		auto itemTextPadding = static_cast<uint32_t>(ItemTextPadding * window->DPIScaleFactor);
		auto menuBoxSubMenuArrowWidth = static_cast<uint32_t>(window->Appereance->MenuBoxSubMenuArrowWidth * window->DPIScaleFactor);
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