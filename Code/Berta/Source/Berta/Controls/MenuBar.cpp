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
	namespace ReactorCore::MenuBar
	{
		void Reactor::Init(ControlBase& control, Graphics* graphics)
		{
			m_module.m_control = reinterpret_cast<Berta::MenuBar*>(&control);

			m_module.m_owner = control.Handle();
		}

		void Reactor::Update(Graphics& graphics)
		{
			auto window = m_module.m_owner;
			bool enabled = m_module.m_control->GetEnabled();

			graphics.DrawRectangle(window->ClientSize.ToRectangle(), enabled ? window->Appearance->ButtonBackground : window->Appearance->ButtonDisabledBackground, true);

			auto& items = m_module.m_items;
			auto itemMargin = window->ToScale(4u);

			for (size_t i = 0; i < items.size(); i++)
			{
				auto& itemData = *(items[i]);

				auto textPosition = Point{ itemData.position.X + static_cast<int>(itemData.center.Width), itemData.position.Y + static_cast<int>(itemData.center.Height) };
				if (m_module.m_interactionData.m_selectedItemIndex == static_cast<int>(i))
				{
					graphics.DrawRectangle({ itemData.position.X, itemData.position.Y, itemData.size.Width, itemData.size.Height }, m_module.IsMenuOpen() ? window->Appearance->MenuBackground : window->Appearance->HighlightColor, true);

					graphics.DrawString(textPosition, itemData.text, window->Appearance->Foreground);
					graphics.DrawRectangle({ itemData.position.X, itemData.position.Y, itemData.size.Width, itemData.size.Height }, window->Appearance->BoxBorderColor, false);
				}
				else
				{
					graphics.DrawString(textPosition, itemData.text, enabled ? window->Appearance->Foreground : window->Appearance->BoxBorderDisabledColor);
				}
				GUI::DrawAccessKeyUnderline(graphics, itemData.text, itemData.accessKey, itemData.accessKeyPosition, { itemData.position.X + (int)itemData.center.Width, itemData.position.Y + (int)itemData.center.Height }, window->Appearance->Foreground);
			}
		}

		void Reactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
		{
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			if (m_module.IsMenuOpen())
			{
				return;
			}
			auto savedIndex = m_module.m_interactionData.m_selectedItemIndex;
			m_module.SelectIndex(-1);
			if (savedIndex != -1)
			{
				GUI::MarkAsNeedUpdate(m_module.m_owner);
			}
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			if (!args.ButtonState.LeftButton)
			{
				return;
			}

			int selectedItem = m_module.FindItem(args.Position);
			int prevSelectedItem = m_module.m_interactionData.m_selectedItemIndex;
			m_module.SelectIndex(selectedItem);
			if (selectedItem != -1)
			{
				if (m_module.IsMenuOpen() && prevSelectedItem == selectedItem)
				{
					GUI::DisposeMenu();
				}
				else
				{
					m_module.OpenMenu();
					if (m_module.IsMenuOpen())
					{
						m_next = m_module.m_interactionData.m_activeMenu->m_menuBox->GetItemReactor();
					}
				}

				GUI::MarkAsNeedUpdate(m_module.m_owner);
			}
			else
			{
				GUI::DisposeMenu();
				GUI::MarkAsNeedUpdate(m_module.m_owner);
			}
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
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
					m_module.OpenMenu(false);
					m_next = m_module.GetActiveMenuBox()->GetItemReactor();

					GUI::UpdateWindow(m_module.m_owner);
				}
			}
			else
			{
				if (selectedItem != m_module.m_interactionData.m_selectedItemIndex)
				{
					m_module.SelectIndex(selectedItem);

					GUI::MarkAsNeedUpdate(m_module.m_owner);
				}
			}
			m_module.m_lastMousePosition = args.Position;
		}

		void Reactor::Resize(Graphics& graphics, const ArgResize& args)
		{
			m_module.BuildItems();
		}

		void Reactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
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

		void Reactor::MoveToNextItem(bool upwards)
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
		
			GUI::UpdateWindow(m_module.m_owner);
		}

		void Reactor::Select()
		{
		}

		void Reactor::Quit()
		{
		}

		Window* Reactor::Owner() const
		{
			return m_module.m_owner;
		}

		ReactorCore::MenuBox::MenuItemReactor* Reactor::GetLastMenuItem() const
		{
			auto activeMenuItemReactor = (MenuItemReactor*)this;
			while (activeMenuItemReactor->Next() != nullptr)
			{
				activeMenuItemReactor = activeMenuItemReactor->Next();
			}
			return activeMenuItemReactor;
		}

		int Reactor::Module::FindItem(const Point& position) const
		{
			auto& items = m_items;

			for (size_t i = 0; i < items.size(); i++)
			{
				auto& itemData = *(items[i]);

				if (Rectangle{ itemData.position, itemData.size}.IsInside(position))
				{
					return static_cast<int>(i);
				}
			}
			return -1;
		}

		void Reactor::Module::OpenMenu(bool ignoreFirstMouseUp)
		{
			auto window = m_owner;
			auto itemData = m_items[m_interactionData.m_selectedItemIndex].get();
			auto& activeMenuPtr = m_interactionData.m_activeMenu;
			if (activeMenuPtr == &itemData->menu)
				return;

			activeMenuPtr = &itemData->menu;
		
			Point boxPosition{};
			boxPosition.X += itemData->position.X;
			boxPosition.Y += itemData->position.Y + (int)itemData->size.Height;

			activeMenuPtr->m_destroyCallback = [this]()
			{
				m_interactionData.m_activeMenu = nullptr;
				SelectIndex(-1);

				GUI::UpdateWindow(*m_control);
			};

			//TODO: focus window
			activeMenuPtr->ShowPopup(m_owner, boxPosition, true, ignoreFirstMouseUp);
		}

		void Reactor::Module::SelectIndex(int index)
		{
			m_interactionData.m_selectedItemIndex = index;
		}

		Berta::MenuBox* Reactor::Module::GetActiveMenuBox() const
		{
			return m_interactionData.m_activeMenu->m_menuBox;
		}

		Menu& Reactor::Module::At(size_t index)
		{
			return m_items[index]->menu;
		}

		void Reactor::Module::BuildItems(size_t startIndex)
		{
			if (startIndex >= m_items.size())
			{
				return;
			}

			auto itemMargin = m_owner->ToScale(6u);
			auto itemMarginInner = m_owner->ToScale(10u);
			auto menuBarItemHeight = m_owner->ClientSize.Height - itemMargin;
			Point offset{ 0, (int)(m_owner->ClientSize.Height - menuBarItemHeight) >> 1 };

			if (startIndex > 0)
			{
				offset.X = m_items[startIndex - 1]->position.X + (int)m_items[startIndex - 1]->size.Width;
			}

			for (size_t i = startIndex; i < m_items.size(); i++)
			{
				auto& itemData = *m_items[i];
				auto textSize = m_owner->Renderer.GetGraphics().GetTextExtent(itemData.text);
				Size itemSize{ textSize.Width + itemMarginInner * 2u, menuBarItemHeight };

				auto center = itemSize - textSize;
				center = center * 0.5f;

				Point itemPos = offset;
				itemData.position = itemPos;
				itemData.size = itemSize;
				itemData.center = center;

				offset.X += static_cast<int>(itemSize.Width);
			}
		}

		Menu& Reactor::Module::PushBack(const std::wstring& text)
		{
			wchar_t accessKey;
			std::size_t accessKeyPosition;
			auto transformedText = GUI::GetAccessKeyText(text, accessKey, &accessKeyPosition);

			auto startIndex = m_items.size();
			auto& newItem = m_items.emplace_back(new Reactor::MenuBarItemData{ transformedText, accessKey, accessKeyPosition });
			BuildItems(startIndex);

			return newItem->menu;
		}
	}

	MenuBar::MenuBar(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "MenuBar";
#endif
	}

	Menu& MenuBar::At(size_t index)
	{
		return GetReactor().GetModule().At(index);
	}

	size_t MenuBar::GetCount() const
	{
		return GetReactor().GetModule().m_items.size();
	}

	Menu& MenuBar::PushBack(const std::wstring& itemName)
	{
		return GetReactor().GetModule().PushBack(itemName);
	}

	Menu& MenuBar::PushBack(const std::string& itemName)
	{
		std::wstring wItemName = StringUtils::UTF8ToWide(itemName);

		return GetReactor().GetModule().PushBack(wItemName);
	}
}