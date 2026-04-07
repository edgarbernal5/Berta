/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Menu.h"

#include <utility>

#include "Berta/GUI/Interface.h"
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

	void Menu::ShowPopup(Window* owner, const ArgMouse& args)
	{
		Foundation::GetInstance().GetMenuManager().ShowContextMenu(*this, owner, args.Position);
	}

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
	
	namespace ReactorCore::MenuBox
	{
		void Reactor::Init(ControlBase& control, Graphics* graphics)
		{
			ControlReactor::Init(control, graphics);
			
			m_module.m_control = &control;
			m_module.m_owner = control.Handle();
			m_module.InitTimer();
		}

		void Reactor::Update(Graphics& graphics)
		{
			if (!m_module.m_menuData)
			{
				return;
			}
			
			auto window = m_module.m_owner;
			auto appearance = reinterpret_cast<Appearance*>(window->Appearance.get());
			int leftPaneWidth = window->ToScale(appearance->MenuBoxLeftPaneWidth);
			auto menuArrowWidth = window->ToScale(appearance->MenuBoxSubMenuArrowWidth);
			auto smallIconSize = window->ToScale(window->Appearance->SmallIconSize);
			
			graphics.FillRectangle(window->ClientSize.ToRectangle(), appearance->MenuBackground);
			graphics.DrawRectangle(window->ClientSize.ToRectangle(), appearance->BoxBorderColor);
			
			for (size_t i = 0; i < m_module.m_layoutCache.size(); ++i)
			{
				const auto& cache = m_module.m_layoutCache[i];
				bool isHovered = (m_module.m_hoveredIndex.has_value() && m_module.m_hoveredIndex.value() == i);
				
				// C++17 std::visit: Resolvemos el tipo de ítem en tiempo de compilación
				std::visit([&](const auto& itemData)
				{
					using T = std::decay_t<decltype(itemData)>;
					if constexpr (std::is_same_v<T, Menu::MenuSeparator>)
					{
						int midY = cache.bounds.Y + (int)cache.bounds.Height / 2;
						graphics.DrawLine(
							{ leftPaneWidth, midY }, 
							{ (int)cache.bounds.Width - window->ToScale(4), midY }, 
							appearance->BoxBorderColor
						);
					}
					else
					{
						if (isHovered && itemData.isEnabled)
						{
							Rectangle highlightRect = { 
							window->ToScale(2), 
							cache.bounds.Y, 
							cache.bounds.Width - window->ToScale(4u), 
							cache.bounds.Height 
						};
							graphics.FillRectangle(highlightRect, appearance->HighlightColor);
						}
							
						Color mainColor = itemData.isEnabled ? 
							(isHovered ? appearance->HighlightTextColor : appearance->Foreground) : 
							appearance->ButtonDisabledBackground;

						graphics.DrawString(cache.textPosition, itemData.text, mainColor);
						if constexpr (std::is_same_v<T, Menu::MenuAction> || std::is_same_v<T, Menu::MenuSubMenu>)
						{
							if (itemData.image)
							{
								int iconSize = window->ToScale(appearance->SmallIconSize);
								int iconX = cache.bounds.X + (leftPaneWidth - iconSize) / 2;
								int iconY = cache.bounds.Y + (cache.bounds.Height - iconSize) / 2;
								
								Rectangle destRect { iconX, iconY, (uint32_t)iconSize, (uint32_t)iconSize };
								Rectangle srcRect = itemData.image.GetSize().ToRectangle();
								
								itemData.image.Paste(srcRect, graphics, destRect);
							}
						}
						if constexpr (std::is_same_v<T, Menu::MenuSubMenu>)
						{
							int arrowWidth = window->ToScale(4);
							int arrowLength = window->ToScale(2);
							graphics.DrawArrow({ cache.arrowPosition.X , cache.arrowPosition.Y, menuArrowWidth, menuArrowWidth },
								arrowLength,
								arrowWidth,
								Graphics::ArrowDirection::Right,
								itemData.isEnabled ? (window->Appearance->Foreground) : window->Appearance->BoxBorderDisabledColor,
								true,
								itemData.isEnabled ? (window->Appearance->Foreground) : window->Appearance->BoxBorderDisabledColor);
						}
						else if constexpr (std::is_same_v<T, Menu::MenuCheckbox>)
						{
							if (itemData.isChecked)
							{
								int checkSize = window->ToScale(appearance->CheckboxSize);
								int checkY = cache.bounds.Y + (cache.bounds.Height - checkSize) / 2;
								
								int checkX = (leftPaneWidth - checkSize) / 2; 
                            
								m_module.DrawCheckmark(graphics, { checkX, checkY }, checkSize, mainColor);
							}
						}
						else if constexpr (std::is_same_v<T, Menu::MenuAction>)
						{
							if (!itemData.shortcutText.empty())
							{
								Color shortcutColor = isHovered ? appearance->HighlightTextColor : appearance->Foreground2nd;
								graphics.DrawString(cache.shortcutPosition, itemData.shortcutText, shortcutColor);
							}
						}
					}
				}, m_module.m_menuData->GetItems()[i]);
			}
		}

		void Reactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
		{
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			m_module.m_pendingSubMenuIndex = std::nullopt;
			m_module.m_hoverTimer.Stop();

			if (m_module.m_openedSubMenuIndex.has_value())
			{
				m_module.m_hoveredIndex = m_module.m_openedSubMenuIndex;
			}
			else
			{
				m_module.m_hoveredIndex = std::nullopt;
			}
			GUI::MarkAsNeedUpdate(m_module.m_owner);
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			if (m_module.m_owner->ClientSize.ToRectangle().Contains(args.Position))
			{
				return;
			}
			
			auto& manager = Foundation::GetInstance().GetMenuManager();
			Point screenPos = GUI::GetWindowPosition(m_module.m_owner);

			if (!manager.FindMenu(screenPos))
			{
				Window* owner = manager.GetOwner();
				bool clickedOwner = false;
				if (owner)
				{
					Point localOwnerPos = GUI::GetMousePositionToWindow(owner);
					clickedOwner = owner->ClientSize.ToRectangle().Contains(localOwnerPos);
				}

				if (!clickedOwner)
				{
					manager.CloseAll();
				}
			}
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			std::optional<std::size_t> newHoveredIndex = std::nullopt;
			
			for (size_t i = 0; i < m_module.m_layoutCache.size(); ++i)
			{
				if (m_module.m_layoutCache[i].bounds.Contains(args.Position))
				{
					if (!std::holds_alternative<Menu::MenuSeparator>(m_module.m_menuData->GetItem(i)))
					{
						newHoveredIndex = i;
					}
					break;
				}
			}

			if (m_module.m_hoveredIndex != newHoveredIndex)
			{
				m_module.m_hoveredIndex = newHoveredIndex;
				GUI::MarkAsNeedUpdate(m_module.m_owner);

				m_module.m_pendingSubMenuIndex = std::nullopt;
				m_module.m_hoverTimer.Stop();

				if (m_module.m_openedSubMenuIndex.has_value() && m_module.m_openedSubMenuIndex != newHoveredIndex)
				{
					Foundation::GetInstance().GetMenuManager().CloseChildrenOf(m_module.m_owner);
					
					m_module.m_openedSubMenuIndex = std::nullopt; 
				}
				
				if (m_module.m_hoveredIndex.has_value())
				{
					size_t index = m_module.m_hoveredIndex.value();
					const auto& itemData = m_module.m_menuData->GetItems()[index];

					if (std::holds_alternative<Menu::MenuSubMenu>(itemData) && m_module.m_openedSubMenuIndex != index)
					{
						m_module.m_pendingSubMenuIndex = index;
						m_module.m_hoverTimer.SetInterval(Module::SubMenuDelayMs);
						m_module.m_hoverTimer.Start();
					}
				}
			}
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			if (m_module.m_ignoreFirstMouseUp)
			{
				m_module.m_ignoreFirstMouseUp = false;
				return;
			}

			if (m_module.m_hoveredIndex.has_value())
			{
				m_module.ExecuteHoveredItem();
			}
		}

		void Reactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
		{
			auto& menuManager = Foundation::GetInstance().GetMenuManager();
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
					menuManager.NavigateTopLevel(1);
				}
				break;
			case KeyboardKey::ArrowLeft:
				if (menuManager.GetPopupCount() > 1) {
					menuManager.Close(m_module.m_owner);
				} else {
					menuManager.NavigateTopLevel(-1);
				}
				break;
			case KeyboardKey::Enter:
				m_module.ExecuteHoveredItem();
				break;
			case KeyboardKey::Escape:
				menuManager.CloseAll();
				break;
			}
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

	void MenuBox::InitFromData(Menu& menuData)
	{
		auto& module = GetReactor().GetModule();
		module.InitFromData(menuData);
		
		SetSize(module.m_calculatedBoxSize);
	}
	
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
		auto appearance = reinterpret_cast<Appearance*>(window->Appearance.get());
		const auto& items = menuData.GetItems();
        
		m_layoutCache.resize(items.size());
        
		int leftPaneWidth = m_owner->ToScale(appearance->MenuBoxLeftPaneWidth);
		int itemHeight = m_owner->ToScale(appearance->MenuBoxItemHeight);
		int padding = m_owner->ToScale(appearance->ItemTextPadding);
		int arrowWidth = m_owner->ToScale(appearance->MenuBoxSubMenuArrowWidth);
		int shortcutWidth = m_owner->ToScale(appearance->MenuBoxShortcutWidth);
		int separatorHeight = m_owner->ToScale(appearance->SeparatorHeight);
		
		int maxTextWidth = 0;
		int maxShortcutWidth = 0;
		
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
					auto sSize = graphics.GetTextExtent(arg.shortcutText);
					if (tSize.Width > maxTextWidth) maxTextWidth = tSize.Width;
					if (sSize.Width > maxShortcutWidth) maxShortcutWidth = sSize.Width;
				}
			}, itemData);
		}

		int finalWidth = leftPaneWidth + padding + maxTextWidth + padding + shortcutWidth + arrowWidth;
		int currentY = window->ToScale(2);

		for (size_t i = 0; i < items.size(); ++i)
		{
			auto& cache = m_layoutCache[i];
			std::visit([&](const auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, Menu::MenuSeparator>)
				{
					cache.bounds = { 0, currentY, (uint32_t)finalWidth, window->ToScale(3u) };
					currentY += (int)cache.bounds.Height;
				}
				else
				{
					cache.bounds = { 0, currentY, (uint32_t)finalWidth, (uint32_t)itemHeight };
					
					auto textSize = graphics.GetTextExtent(arg.text);
					if (textSize.Width > maxTextWidth)
					{
						maxTextWidth = textSize.Width;
					}
					
					int textY = currentY + (itemHeight - textSize.Height) / 2;
					cache.textPosition = { leftPaneWidth + padding, textY };
					
					if constexpr (std::is_same_v<T, Menu::MenuSubMenu>)
					{
						int arrowY = currentY + (itemHeight - arrowWidth) / 2;
						cache.arrowPosition = { finalWidth - arrowWidth, arrowY };
					}
					else if constexpr (std::is_same_v<T, Menu::MenuAction>)
					{
						if (!arg.shortcutText.empty())
						{
							auto sSize = graphics.GetTextExtent(arg.shortcutText);
							cache.shortcutPosition = { finalWidth - shortcutWidth - padding, textY };
						}
					}
					
					currentY += itemHeight;
				}
			}, items[i]);
		}
		
		m_calculatedBoxSize = { (uint32_t)finalWidth, (uint32_t)(currentY + m_owner->ToScale(2u)) };
	}

	void ReactorCore::MenuBox::Module::InitFromData(Menu& menuData)
	{
		m_menuData = &menuData;
		CalculateLayout(menuData);
    
		m_control->SetSize(m_calculatedBoxSize); //TODO
	}

	void ReactorCore::MenuBox::Module::InitTimer()
	{
		m_hoverTimer.SetOwner(m_owner);
		m_hoverTimer.SetInterval(400);
		m_hoverTimer.Connect([this](const ArgTimer& args)
		{
			m_hoverTimer.Stop();
			
			size_t indexToOpen = m_pendingSubMenuIndex.value();
			m_pendingSubMenuIndex = std::nullopt;

			if (auto* subMenuData = std::get_if<Menu::MenuSubMenu>(&m_menuData->GetItem(indexToOpen)))
			{
				if (subMenuData->isEnabled && subMenuData->subMenu)
				{
					auto& menuManager = Foundation::GetInstance().GetMenuManager();
		            
					// 4. Si había otro submenú de este mismo nivel abierto, lo cerramos
					// (Tu MenuManager podría necesitar saber qué popups son hijos de quién, 
					// pero cerrar el submenú actual es la idea básica).
		            
					const auto& cache = m_layoutCache[indexToOpen];
					
					Point popupPos = { cache.bounds.X + (int)cache.bounds.Width, cache.bounds.Y };
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
				if (arg.isEnabled)
				{
					auto callback = arg.onClick;
					Menu* safeMenuData = m_menuData;
					Foundation::GetInstance().GetMenuManager().CloseAll();

					if (callback)
					{
						callback(MenuItem(safeMenuData, index));
					}
				}
			}
			else if constexpr (std::is_same_v<T,  Menu::MenuCheckbox>)
			{
				if (arg.isEnabled)
				{
					m_menuData->ToggleCheckbox(index);
					bool newState = arg.isChecked;
					auto callback = arg.onToggle;
					Menu* safeMenuData = m_menuData;

					Foundation::GetInstance().GetMenuManager().CloseAll();

					if (callback)
					{
						callback(MenuItem(safeMenuData, index), newState);
					}
				}
			}
			else if constexpr (std::is_same_v<T,  Menu::MenuSubMenu>)
			{
				OpenHoveredSubMenu(true); 
			}
		}, m_menuData->GetItem(index));
	}

	void ReactorCore::MenuBox::Module::OpenHoveredSubMenu(bool selectFirstItem)
	{
        if (!m_hoveredIndex.has_value() || !m_menuData)
        {
	        return;
        }
		
        size_t indexToOpen = m_hoveredIndex.value();
        
        m_pendingSubMenuIndex = std::nullopt;
        m_hoverTimer.Stop();

        const auto& itemData = m_menuData->GetItems()[indexToOpen];

        if (auto* subMenuData = std::get_if<Menu::MenuSubMenu>(&itemData))
        {
            if (subMenuData->isEnabled && subMenuData->subMenu)
            {
                auto& menuManager = Foundation::GetInstance().GetMenuManager();
                
                // --- Opcional (Depende de tu implementación en MenuManager) ---
                // Aquí podrías decirle a MenuManager que cierre otros submenús hermanos
                // que estén abiertos en este mismo nivel, antes de abrir el nuevo.
                // menuManager.CloseSiblings(m_module.m_owner); 

                const auto& cache = m_layoutCache[indexToOpen];
                
                Point popupPos = { cache.bounds.X + (int)cache.bounds.Width, cache.bounds.Y };
                menuManager.ShowContextMenu(*(subMenuData->subMenu), m_owner, popupPos);
                
                m_openedSubMenuIndex = indexToOpen;

                if (selectFirstItem)
                {
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
        
		int currentIndex = m_hoveredIndex.value_or(step > 0 ? -1 : count); 

		for (int i = 0; i < count; ++i)
		{
			currentIndex = (currentIndex + step + count) % count;

			bool isValid = std::visit([](const auto& item)
			{
				using T = std::decay_t<decltype(item)>;
				if constexpr (std::is_same_v<T, Menu::MenuSeparator>)
				{
					return false;
				}
				else
				{
					return item.isEnabled;
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
