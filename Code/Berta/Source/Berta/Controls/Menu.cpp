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
	
	MenuItem Menu::Append(const std::string& text, ClickCallback onClick)
	{
		std::wstring wstr = StringUtils::UTF8ToWide(text);
		m_items.emplace_back(MenuAction{ wstr, L"", Image{}, std::move(onClick) });
		return {this, m_items.size() - 1};
	}

	MenuItem Menu::Append(const std::wstring& text, ClickCallback onClick)
	{
		m_items.emplace_back(MenuAction{ text, L"", Image{}, std::move(onClick) });
		return {this, m_items.size() - 1};
	}

	MenuItem Menu::AppendSeparator()
	{
		m_items.emplace_back(MenuSeparator{});
		return {this, m_items.size() - 1};
	}

	MenuItem Menu::AppendSubMenu(const std::wstring& text, std::unique_ptr<Menu> subMenu)
	{
		m_items.emplace_back(MenuSubMenu{ text, Image{}, std::move(subMenu) });
		return {this, m_items.size() - 1};
	}

	MenuItem Menu::AppendCheckbox(const std::wstring& text, bool initialState, ToggleCallback onToggle)
	{
		m_items.emplace_back(MenuCheckbox{ text, initialState, std::move(onToggle) });
		return {this, m_items.size() - 1};
	}

	/*void Menu::ShowPopup(Window* owner, const Point& position, bool fromMenuBar, bool ignoreFirstMouseUp)
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
	}*/

	/*void Menu::ShowPopup(Window* owner, const ArgMouse& args)
	{
		if (!args.ButtonState.RightButton)
		{
			return;
		}
		auto screenPosition = args.Position;
		ShowPopup(owner, screenPosition, false);
	}*/

	/*Menu* Menu::CreateSubMenu(std::size_t index)
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
	}*/

	void Menu::SetText(size_t index, const std::wstring& text)
	{
		if (index >= m_items.size())
		{
			return;
		}
		
		std::visit([&text](auto& item)
		{
			using T = std::decay_t<decltype(item)>;
			if constexpr (!std::is_same_v<T, MenuSeparator>)
			{
				item.text = text;
			}
		}, m_items[index]);
	}

	std::wstring Menu::GetText(size_t index) const
	{
		if (index >= m_items.size())
		{
			return L"";
		}
		
		std::wstring result;
		std::visit([&result](const auto& item)
		{
			using T = std::decay_t<decltype(item)>;
			if constexpr (!std::is_same_v<T, MenuSeparator>)
			{
				result = item.text;
			}
		}, m_items[index]);
		
		return result;
	}

	void Menu::SetImage(size_t index, const Image& image)
	{
		if (index >= m_items.size())
		{
			return;
		}
		
		// Usamos std::visit para modificar solo si es un tipo que soporta imagen
		std::visit([&image](auto& item)
		{
			using T = std::decay_t<decltype(item)>;
			if constexpr (std::is_same_v<T, MenuAction> || std::is_same_v<T, MenuSubMenu>)
			{
				item.image = image;
			}
		}, m_items[index]);
	}

	bool Menu::GetEnabled(size_t index) const
	{
		if (index >= m_items.size())
		{
			return false;
		}
		
		bool result = false;
		std::visit([&result](const auto& item)
		{
			using T = std::decay_t<decltype(item)>;
			if constexpr (!std::is_same_v<T, MenuSeparator>)
			{
				result = item.isEnabled;
			}
		}, m_items[index]);
		
		return result;
	}

	void Menu::SetEnabled(size_t index, bool enabled)
	{
		if (index >= m_items.size())
		{
			return;
		}
		
		std::visit([enabled](auto& item)
		{
			using T = std::decay_t<decltype(item)>;
			if constexpr (std::is_same_v<T, MenuAction> || std::is_same_v<T, MenuSubMenu>)
			{
				item.isEnabled = enabled;
			}
		}, m_items[index]);
	}

	void Menu::SetChecked(size_t index, bool checked)
	{
		if (index >= m_items.size())
		{
			return;
		}
		
		if (auto* checkbox = std::get_if<MenuCheckbox>(&m_items[index]))
		{
			checkbox->isChecked = checked;
		}
	}

	bool Menu::IsChecked(size_t index) const
	{
		if (index >= m_items.size())
		{
			return false;
		}

		if (const auto* checkbox = std::get_if<MenuCheckbox>(&m_items[index]))
		{
			return checkbox->isChecked;
		}
		return false;
	}

	void Menu::ToggleCheckbox(size_t index)
	{
		if (index >= m_items.size()) return;

		// Solo cambiamos el estado si realmente es un MenuCheckbox
		if (auto* checkbox = std::get_if<MenuCheckbox>(&m_items[index]))
		{
			checkbox->isChecked = !checkbox->isChecked;
		}
	}

	/*void Menu::CloseMenuBox()
	{
		if (!m_menuBox)
			return;

		//GUI::ReleaseCapture(m_menuBox->Handle());
		m_menuBox->Dispose();
		m_menuBox = nullptr;
	}*/

	/*Size Menu::GetMenuBoxSize(Window* parent) const
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
	}*/
	
	namespace ReactorCore::MenuBox
	{
		void Reactor::Init(ControlBase& control, Graphics* graphics)
		{
			ControlReactor::Init(control, graphics);
			//m_menuBox = reinterpret_cast<Berta::MenuBox*>(&control);
			
			m_module.m_control = &control;
			m_module.InitTimer();
		}

		void Reactor::Update(Graphics& graphics)
		{
			if (!m_module.m_menuData)
			{
				return;
			}
			
			auto window = m_module.m_owner;
			auto appearance = window->Appearance.get(); // Solo leemos colores

			int paddingX = window->ToScale(8);
			
			graphics.DrawRectangle(window->ClientSize.ToRectangle(), appearance->MenuBackground, true);

			for (size_t i = 0; i < m_module.m_layoutCache.size(); ++i)
			{
				const auto& cache = m_module.m_layoutCache[i];
				bool isHovered = (m_module.m_hoveredIndex.has_value() && m_module.m_hoveredIndex.value() == i);
				
				// C++17 std::visit: Resolvemos el tipo de ítem en tiempo de compilación
				std::visit([&](const auto& itemData) {
					using T = std::decay_t<decltype(itemData)>;
					if constexpr (std::is_same_v<T, Menu::MenuSeparator>) {
						int midY = cache.bounds.Y + (int)cache.bounds.Height / 2;
						graphics.DrawLine({ cache.bounds.X + 4, midY }, { (int)cache.bounds.Width - 4, midY }, appearance->BoxBorderColor);
					}
					else
					{
						if (isHovered && itemData.isEnabled)
							graphics.DrawRectangle(cache.bounds, appearance->HighlightColor, true);

						Color textColor = itemData.isEnabled ? 
							(isHovered ? appearance->HighlightTextColor : appearance->Foreground) : 
							appearance->ButtonDisabledBackground;

						graphics.DrawString(cache.textPosition, itemData.text, textColor);

						if constexpr (std::is_same_v<T, Menu::MenuSubMenu>)
						{
							graphics.DrawString(cache.arrowPosition, L"►", textColor);
						}
						else if constexpr (std::is_same_v<T, Menu::MenuCheckbox>)
						{
							// Dibujar la palomita vectorial solo si el estado es 'true'
							if (itemData.isChecked)
							{
								// Tomamos el tamaño del checkmark configurado en Appearance
								int checkSize = window->ToScale(appearance->CheckboxHeight);
	                            
								// Lo centramos verticalmente respecto a su hitbox
								int checkY = cache.bounds.Y + (cache.bounds.Height - checkSize) / 2;
								int checkX = cache.bounds.X + window->ToScale(6u); // Margen izquierdo
	                            
								// Llamada a nuestro método de dibujo de vectores
								m_module.DrawCheckmark(graphics, { checkX, checkY }, checkSize, textColor);
							}
						}
						else if constexpr (std::is_same_v<T, Menu::MenuAction>)
						{
						if (!itemData.shortcutText.empty()) {
							auto sSize = graphics.GetTextExtent(itemData.shortcutText); // O precalcularlo en caché
							Point shortcutPos = { (int)cache.bounds.Width - paddingX - (int)sSize.Width, cache.textPosition.Y };
							graphics.DrawString(shortcutPos, itemData.shortcutText, appearance->Foreground2nd);
						}
						}
					}
				}, m_module.m_menuData->GetItems()[i]);
			}
			
			//old
			/*
			auto window = m_control->Handle();
			auto clientSize = window->ClientSize.ToRectangle();
			
			graphics.FillRectangle(clientSize, window->Appearance->MenuBackground);

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
							graphics.FillRectangle({ 1 + (int)(itemTextPadding), offsetY, window->ClientSize.Width - 2u - itemTextPadding * 2u, menuBoxItemHeight }, window->Appearance->HighlightColor);
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

			graphics.DrawRectangle(clientSize, window->Appearance->BoxBorderColor);*/
		}

		void Reactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
		{
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			// 1. Si el mouse sale de la ventana, cancelamos la intención de abrir un submenú
			m_module.m_pendingSubMenuIndex = std::nullopt;
			m_module.m_hoverTimer.Stop();

			// 2. UX Estándar de SO: Si hay un submenú abierto, el ítem padre 
			// DEBE quedarse resaltado (hovered) aunque el mouse se haya ido.
			if (m_module.m_openedSubMenuIndex.has_value())
			{
				m_module.m_hoveredIndex = m_module.m_openedSubMenuIndex;
			}
			else
			{
				// Si no hay submenús abiertos, limpiamos la selección
				m_module.m_hoveredIndex = std::nullopt;
			}
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			/*if (!args.ButtonState.LeftButton)
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
			}*/
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			std::optional<std::size_t> newHoveredIndex = std::nullopt;
			
			// 1. Hit-Testing rápido contra la caché
			for (size_t i = 0; i < m_module.m_layoutCache.size(); ++i)
			{
				if (m_module.m_layoutCache[i].bounds.Contains(args.Position))
				{
					// Solo se puede hacer hover sobre ítems habilitados y que no sean separadores
					if (!std::holds_alternative<Menu::MenuSeparator>(m_module.m_menuData->GetItems()[i]))
					{
						newHoveredIndex = i;
					}
					break;
				}
			}

			// 2. Si el ratón se movió a un ítem DIFERENTE
			if (m_module.m_hoveredIndex != newHoveredIndex)
			{
				m_module.m_hoveredIndex = newHoveredIndex;
        
				// El usuario cambió de opinión o se movió de ítem.
				// ¡Cancelamos el timer de apertura inmediatamente!
				m_module.m_pendingSubMenuIndex = std::nullopt;
				m_module.m_hoverTimer.Stop();

				// 3. Evaluamos el nuevo ítem
				if (m_module.m_hoveredIndex.has_value())
				{
					size_t index = m_module.m_hoveredIndex.value();
					const auto& itemData = m_module.m_menuData->GetItems()[index];

					// Si el nuevo ítem es un SubMenú y NO es el que ya está abierto
					if (std::holds_alternative<Menu::MenuSubMenu>(itemData) && m_module.m_openedSubMenuIndex != index)
					{
						// Registramos la "intención" e iniciamos el timer
						m_module.m_pendingSubMenuIndex = index;
						m_module.m_hoverTimer.SetInterval(Module::SubMenuDelayMs);
						m_module.m_hoverTimer.Start();
					}
					else if (std::holds_alternative<Menu::MenuAction>(itemData))
					{
						// Opcional: Si el ratón se posa sobre una acción normal, 
						// podrías iniciar un timer para CERRAR el submenú abierto actualmente.
						// (Para simplificar, Win32 suele cerrarlo si abres otro submenú o haces clic).
					}
				}
			}
			
			/*bool changes = MouseMoveInternal(args);
			if (changes)
			{
				auto window = m_control->Handle();
				GUI::MarkAsNeedUpdate(window);
			}*/
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			if (m_module.m_ignoreFirstMouseUp)
			{
				m_module.m_ignoreFirstMouseUp = false;
				return;
			}

			// 2. Si soltamos el clic sobre un ítem válido, disparamos la lógica central
			if (m_module.m_hoveredIndex.has_value())
			{
				m_module.ExecuteHoveredItem();
			}
			
			/*
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
			*/
		}

		void Reactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
		{
			switch (args.Key)
			{
			case KeyboardKey::ArrowDown:
				m_module.MoveSelection(1); 
				break;
			case KeyboardKey::ArrowUp:
				m_module.MoveSelection(-1); 
				break;
			case KeyboardKey::ArrowRight:
				if (m_module.m_hoveredIndex.has_value()) {
					m_module.OpenHoveredSubMenu(true);
				} else {
					Foundation::GetInstance().GetMenuManager().NavigateTopLevel(1);
				}
				break;
			case KeyboardKey::ArrowLeft:
				if (Foundation::GetInstance().GetMenuManager().GetPopupCount() > 1) {
					Foundation::GetInstance().GetMenuManager().Close(m_module.m_owner);
				} else {
					Foundation::GetInstance().GetMenuManager().NavigateTopLevel(-1);
				}
				break;
			case KeyboardKey::Enter:
				m_module.ExecuteHoveredItem();
				break;
			case KeyboardKey::Escape:
				Foundation::GetInstance().GetMenuManager().CloseAll();
				break;
			}
		}

		/*void Reactor::BuildItems()
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
		}*/

		/*Size Reactor::GetMenuBoxSize()
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
		}*/

		/*void Reactor::OpenSubMenu(Menu* subMenu, Menu* parentMenu, int selectedIndex, bool ignoreFirstMouseUp)
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
			if (!Rectangle{ m_control->Handle()->ClientSize }.Contains(args.Position))
			{
				return -1;
			}

			for (size_t i = 0; i < m_itemSizePositions.size(); i++)
			{
				auto& item = m_itemSizePositions[i];
				if (!m_items->at(i)->m_isSeparator && Rectangle { item.m_position, item.m_size }.Contains(args.Position))
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
		}*/
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

	/*void MenuBox::Init(Menu* menuOwner, std::vector<std::unique_ptr<Menu::MenuItemData>>& items)
	{
		menuOwner->m_menuBox = this;

		GetReactor().SetItems(items);
		GetReactor().SetMenuOwner(menuOwner);

		auto boxSize = GetMenuBoxSize();
		SetSize(boxSize);

		GetReactor().BuildItems();
	}*/

	void MenuBox::InitFromData(Menu& menuData)
	{
		auto& module = GetReactor().GetModule();
		// Pasamos los datos al reactor para que calcule tamaños
		module.InitFromData(menuData);
    
		// Calcula el tamaño de la ventana en base a los textos e íconos
		SetSize(module.m_calculatedBoxSize);
	}

	/*void MenuBox::SetIgnoreFirstMouseUp(bool value)
	{
		GetReactor().SetIgnoreFirstMouseUp(value);
	}*/

	/*void MenuBox::Popup(bool fromMenuBar)
	{
		auto& menuManager = Foundation::GetInstance().GetMenuManager();
		menuManager.ShowPopup(m_handle, GetOwner(), fromMenuBar);
		Show();
	}*/

	bool MenuBox::MenuItem::GetEnabled() const
	{
		return IsValid() ? m_owner->GetEnabled(m_index) : false;
	}

	MenuItem& MenuBox::MenuItem::SetEnabled(bool enabled)
	{
		if (IsValid())
		{
			m_owner->SetEnabled(m_index, enabled);
		}
		return *this;
	}

	std::wstring MenuItem::GetText() const
	{
		return IsValid() ? m_owner->GetText(m_index) : L"";
	}

	MenuItem& MenuBox::MenuItem::SetText(const std::wstring& text)
	{
		if (IsValid())
		{
			m_owner->SetText(m_index, text);
		}
		
		return *this;
	}

	MenuItem& MenuItem::SetImage(const Image& image)
	{
		if (IsValid())
		{
			m_owner->SetImage(m_index, image);
		}
		return *this;
	}

	MenuItem& MenuItem::SetChecked(bool checked)
	{
		if (IsValid())
		{
			m_owner->SetChecked(m_index, checked);
		}
		
		return *this;
	}

	bool MenuItem::IsChecked() const
	{
		return IsValid() ? m_owner->IsChecked(m_index) : false;
	}

	MenuItem& MenuItem::Toggle()
	{
		if (IsValid())
		{
			m_owner->ToggleCheckbox(m_index);
		}
		return *this;
	}

	void ReactorCore::MenuBox::Module::CalculateLayout(const Menu& menuData)
	{
		auto& window = m_owner;
		auto& graphics = window->Renderer.GetGraphics();
		const auto& items = menuData.GetItems();
        
		m_layoutCache.resize(items.size());
        
		int iconWidth = window->ToScale(16);
		int paddingX = window->ToScale(8u);
		int itemHeight = window->ToScale(24u);
		int maxTextWidth = 0;
		int maxShortcutWidth = 0;
		
		// PASADA 1: Medir
		for (const auto& itemData : items)
		{
			std::visit([&](const auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (!std::is_same_v<T, Menu::MenuSeparator>)
				{
					auto textSize = graphics.GetTextExtent(arg.text);
					if (textSize.Width > maxTextWidth) maxTextWidth = textSize.Width;
				}
				else if constexpr (std::is_same_v<T, Menu::MenuAction>)
				{
					auto tSize = graphics.GetTextExtent(arg.text);
					auto sSize = graphics.GetTextExtent(arg.shortcutText); // Medimos el atajo
					if (tSize.Width > maxTextWidth) maxTextWidth = tSize.Width;
					if (sSize.Width > maxShortcutWidth) maxShortcutWidth = sSize.Width;
				}
			}, itemData);
		}

		// El ancho total del menú ahora incluye la columna extra
		int finalWidth = paddingX + iconWidth + maxTextWidth + paddingX + maxShortcutWidth + paddingX;
		//int finalWidth = std::max<int>(maxTextWidth + (paddingX * 4), window->ToScale(120));
		int currentY = window->ToScale(2);

		// PASADA 2: Asignar coordenadas (Caché)
		for (size_t i = 0; i < items.size(); ++i)
		{
			auto& cache = m_layoutCache[i];
			std::visit([&](const auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, Menu::MenuSeparator>)
				{
					cache.bounds = { 0, currentY, (uint32_t)finalWidth, window->ToScale(3u) };
					currentY += cache.bounds.Height;
				}
				else
				{
					if constexpr (std::is_same_v<T, Menu::MenuAction> || std::is_same_v<T, Menu::MenuSubMenu> || std::is_same_v<T, Menu::MenuCheckbox>) 
					{
						auto textSize = graphics.GetTextExtent(arg.text);
						if (textSize.Width > maxTextWidth)
						{
							maxTextWidth = textSize.Width;
						}
					}
					cache.bounds = { 0, currentY, (uint32_t)finalWidth, (uint32_t)itemHeight };
					cache.textPosition = { paddingX * 2, currentY + window->ToScale(4) };
					if constexpr (std::is_same_v<T, Menu::MenuSubMenu>)
					{
						cache.arrowPosition = { finalWidth - paddingX * 2, cache.textPosition.Y };
					}
					currentY += itemHeight;
				}
			}, items[i]);
		}
		m_calculatedBoxSize = { (uint32_t)finalWidth, currentY + window->ToScale(2u) };
	}

	void ReactorCore::MenuBox::Module::InitFromData(Menu& menuData)
	{
		m_menuData = &menuData;
		CalculateLayout(menuData);
    
		// Asignamos el tamaño de la ventana (MenuBox) basándonos en el cálculo
		//m_owner->SetSize(m_calculatedBoxSize); //TODO
	}

	void ReactorCore::MenuBox::Module::InitTimer()
	{
		m_hoverTimer.SetOwner(m_owner);
		m_hoverTimer.SetInterval(400);
		m_hoverTimer.Connect([this](const ArgTimer& args)
		{
			// 1. Detenemos el timer
			m_hoverTimer.Stop();
			
			// 2. Extraemos el índice seguro
			size_t indexToOpen = m_pendingSubMenuIndex.value();
			m_pendingSubMenuIndex = std::nullopt; // Ya no está pendiente

			// 3. Obtenemos los datos del submenú
			if (auto* subMenuData = std::get_if<Menu::MenuSubMenu>(&m_menuData->GetItem(indexToOpen)))
			{
				if (subMenuData->isEnabled && subMenuData->subMenu) // Solo si está habilitado y tiene datos
				{
					auto& menuManager = Foundation::GetInstance().GetMenuManager();
		            
					// 4. Si había otro submenú de este mismo nivel abierto, lo cerramos
					// (Tu MenuManager podría necesitar saber qué popups son hijos de quién, 
					// pero cerrar el submenú actual es la idea básica).
		            
					// 5. Calculamos la posición geométrica basándonos en la caché
					const auto& cache = m_layoutCache[indexToOpen];
		            
					// Aparece a la derecha del ítem actual. 
					// TODO: Podrías necesitar convertir 'cache.bounds' a coordenadas de pantalla
					// dependiendo de cómo maneja las coordenadas tu framework.
					Point popupPos = { cache.bounds.X + (int)cache.bounds.Width, cache.bounds.Y };

					// 6. ¡Abrimos el submenú!
					menuManager.ShowContextMenu(*(subMenuData->subMenu), m_owner, popupPos);
		            
					m_openedSubMenuIndex = indexToOpen;
				}
			}
		});
	}

	void ReactorCore::MenuBox::Module::ExecuteHoveredItem()
	{
		if (!m_hoveredIndex.has_value() || !m_menuData)
		{
			return;
		}

		size_t index = m_hoveredIndex.value();
    
		std::visit([&](auto& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, Menu::MenuAction>)
			{
				if (arg.isEnabled && arg.onClick)
				{
					// CRÍTICO: Copiamos el callback a una variable local.
					auto callback = arg.onClick;

					// 1. Destruimos los popups y liberamos la memoria ANTES de ejecutar.
					// Esto devuelve el control a la ventana principal de Berta.
					Foundation::GetInstance().GetMenuManager().CloseAll();

					// 2. ¡DISPARAMOS EL CALLBACK DEL USUARIO!
					callback(MenuItem(m_menuData, index));
				}
			}
			else if constexpr (std::is_same_v<T,  Menu::MenuCheckbox>)
			{
				if (arg.isEnabled)
				{
					// 1. Cambiamos el estado (Toggle)
					m_menuData->ToggleCheckbox(index);
					bool newState = arg.isChecked;
					auto callback = arg.onToggle;

					// 2. Cerramos el menú
					Foundation::GetInstance().GetMenuManager().CloseAll();

					// 3. Disparamos el callback
					if (callback)
					{
						callback(MenuItem(m_menuData, index), newState);
					}
				}
			}
			else if constexpr (std::is_same_v<T,  Menu::MenuSubMenu>)
			{
				// Si el usuario hace clic (o presiona Enter) explícitamente en un submenú,
				// no esperamos el temporizador de hover, lo abrimos inmediatamente.
				OpenHoveredSubMenu(true); 
			}
		}, m_menuData->GetItem(index));
	}

	void ReactorCore::MenuBox::Module::OpenHoveredSubMenu(bool selectFirstItem)
	{
		// 1. Validamos que tengamos un ítem seleccionado y datos
        if (!m_hoveredIndex.has_value() || !m_menuData)
        {
	        return;
        }
		
        size_t indexToOpen = m_hoveredIndex.value();
        
        // 2. Cancelamos cualquier timer pendiente, ya que lo abrimos instantáneamente
        m_pendingSubMenuIndex = std::nullopt;
        m_hoverTimer.Stop();

        // 3. Obtenemos los datos del ítem
        const auto& itemData = m_menuData->GetItems()[indexToOpen];

        // 4. Verificamos que realmente sea un SubMenú y esté habilitado
        if (auto* subMenuData = std::get_if<Menu::MenuSubMenu>(&itemData))
        {
            if (subMenuData->isEnabled && subMenuData->subMenu)
            {
                auto& menuManager = Foundation::GetInstance().GetMenuManager();
                
                // --- Opcional (Depende de tu implementación en MenuManager) ---
                // Aquí podrías decirle a MenuManager que cierre otros submenús hermanos
                // que estén abiertos en este mismo nivel, antes de abrir el nuevo.
                // menuManager.CloseSiblings(m_module.m_owner); 

                // 5. Calculamos la posición geométrica usando nuestra Caché de Layout
                const auto& cache = m_layoutCache[indexToOpen];
                
                // Aparece a la derecha del menú actual, alineado con la altura del ítem.
                // (Sumamos el ancho total para que aparezca "afuera").
                Point popupPos = { cache.bounds.X + (int)cache.bounds.Width, cache.bounds.Y };

                // Si tu ventana actual necesita convertir esto a coordenadas de pantalla completas:
                // popupPos = m_module.m_owner->PointToScreen(popupPos);

                // 6. ¡Abrimos el submenú usando nuestra Fábrica (Manager)!
                menuManager.ShowContextMenu(*(subMenuData->subMenu), m_owner, popupPos);
                
                // 7. Registramos que este es el submenú actualmente abierto
                m_openedSubMenuIndex = indexToOpen;

                // 8. Navegación por teclado: Si lo abrimos con la flecha Derecha, 
                // el foco debe pasar automáticamente al primer ítem del nuevo submenú.
                if (selectFirstItem)
                {
                    // Como ShowContextMenu acaba de agregar la ventana a la pila de popups,
                    // obtenemos la ventana más reciente (la que acabamos de crear).
                    Window* newlyOpenedMenu = menuManager.GetActiveMenu(false);
                    
                    if (newlyOpenedMenu)
                    {
                        // Simulamos presionar "Abajo" en el nuevo menú para seleccionar su primer ítem
                        //ArgKeyboard args;
                        //args.Key = VK_DOWN;
                        //Foundation::GetInstance().ProcessEvents(
                        //    newlyOpenedMenu, nullptr, &ControlEvents::KeyPressed, args);
                    }
                }
            }
        }
	}

	void ReactorCore::MenuBox::Module::MoveSelection(int step)
	{
		const auto& items = m_menuData->GetItems();
		if (items.empty())
		{
			return;
		}
		
		int count = static_cast<int>(items.size());
        
		// Si no hay nada seleccionado, empezamos desde arriba (o abajo si step es negativo)
		int currentIndex = m_hoveredIndex.value_or(step > 0 ? -1 : count); 

		// Iteramos para buscar el siguiente ítem válido (saltando separadores)
		for (int i = 0; i < count; ++i)
		{
			// Aritmética modular para que el menú sea cíclico (da la vuelta)
			currentIndex = (currentIndex + step + count) % count;

			// Verificamos si el ítem es válido para ser seleccionado usando std::visit
			bool isValid = std::visit([](const auto& item)
			{
				using T = std::decay_t<decltype(item)>;
				if constexpr (std::is_same_v<T, Menu::MenuSeparator>)
				{
					return false; // Nunca seleccionamos separadores
				}
				else
				{
					return item.isEnabled; // Solo seleccionamos si está habilitado
				}
			}, items[currentIndex]);

			if (isValid)
			{
				m_hoveredIndex = currentIndex;
                
				// Opcional: si el ítem está fuera de la pantalla (en caso de scroll),
				// aquí podrías ajustar el offset de la ventana.
				return;
			}
		}
	}

	void ReactorCore::MenuBox::Module::DrawCheckmark(Graphics& graphics, const Point& position, int size, Color color)
	{
		// Definimos los tres puntos de la "palomita" basados en el tamaño de su caja (size x size)
		// P1: Empieza en el 20% de X y 50% de Y (lado izquierdo, a la mitad)
		Point p1 = { 
			position.X + static_cast<int>(size * 0.2f), 
			position.Y + static_cast<int>(size * 0.5f) 
		};
        
		// P2: Baja hasta el 45% de X y 75% de Y (el vértice inferior)
		Point p2 = { 
			position.X + static_cast<int>(size * 0.45f), 
			position.Y + static_cast<int>(size * 0.75f) 
		};
        
		// P3: Sube hasta el 80% de X y 25% de Y (la punta derecha alta)
		Point p3 = { 
			position.X + static_cast<int>(size * 0.8f), 
			position.Y + static_cast<int>(size * 0.25f) 
		};

		// Dibujamos las dos líneas que forman el checkmark
		graphics.DrawLine(p1, p2, color);
		graphics.DrawLine(p2, p3, color);

		// Opcional: Si tu API Graphics no soporta grosor (thickness) en DrawLine, 
		// puedes hacer la línea "más gorda" desplazando todo 1 píxel hacia abajo o a la derecha:
		graphics.DrawLine({ p1.X, p1.Y + 1 }, { p2.X, p2.Y + 1 }, color);
		graphics.DrawLine({ p2.X, p2.Y + 1 }, { p3.X, p3.Y + 1 }, color);
	}
}
