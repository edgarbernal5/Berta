/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "MenuBar.h"

#include "Berta/Core/Foundation.h"
#include "Berta/GUI/Interface.h"
#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	namespace ReactorCore::MenuBar
	{
		void Reactor::Init(ControlBase& control, Graphics* graphics)
		{
			m_module.m_control = &control;
			m_module.m_owner = control.Handle();
		}

		void Reactor::Update(Graphics& graphics)
		{
			auto window = m_module.m_owner;
			auto appearance = window->Appearance.get();
			bool enabled = m_module.m_control->GetEnabled();

			// 1. Dibujar fondo de la barra
			graphics.DrawRectangle(window->ClientSize.ToRectangle(), 
				enabled ? appearance->ButtonBackground : appearance->ButtonDisabledBackground, true);

			// 2. Dibujar ítems desde la caché
			for (size_t i = 0; i < m_module.m_layoutCache.size(); ++i)
			{
				const auto& itemData = m_module.m_items[i];
				const auto& cache = m_module.m_layoutCache[i];
            
				bool isSelected = (m_module.m_interaction.m_selectedIndex == i);
				bool isOpen = m_module.m_interaction.m_isMenuOpen;

				// A) Fondos dinámicos
				if (isSelected && enabled)
				{
					Color bgColor = isOpen ? appearance->BoxPressedBackground : appearance->ButtonHighlightBackground;
					graphics.DrawRectangle(cache.bounds, bgColor, true);
				}

				// B) Texto principal
				Color textColor = enabled ? appearance->Foreground : appearance->ButtonDisabledBackground;
				graphics.DrawString(cache.textPosition, itemData.text, textColor);

				// C) Subrayado de AccessKey (Ej: la 'A' de Archivo)
				if (enabled && itemData.accessKey != 0)
				{
					GUI::DrawAccessKeyUnderline(graphics, itemData.text, itemData.accessKey, 
						itemData.accessKeyPosition, cache.textPosition, textColor);
				}
			}
			
			/*graphics.FillRectangle(window->ClientSize.ToRectangle(), enabled ? window->Appearance->ButtonBackground : window->Appearance->ButtonDisabledBackground);

			auto& items = m_module.m_items;
			auto itemMargin = window->ToScale(4u);

			for (size_t i = 0; i < items.size(); i++)
			{
				auto& itemData = *(items[i]);

				auto textPosition = Point{ itemData.position.X + static_cast<int>(itemData.center.Width), itemData.position.Y + static_cast<int>(itemData.center.Height) };
				if (m_module.m_interactionData.m_selectedItemIndex == static_cast<int>(i))
				{
					graphics.FillRectangle({ itemData.position.X, itemData.position.Y, itemData.size.Width, itemData.size.Height }, m_module.IsMenuOpen() ? window->Appearance->MenuBackground : window->Appearance->HighlightColor);

					graphics.DrawString(textPosition, itemData.text, window->Appearance->Foreground);
					graphics.DrawRectangle({ itemData.position.X, itemData.position.Y, itemData.size.Width, itemData.size.Height }, window->Appearance->BoxBorderColor);
				}
				else
				{
					graphics.DrawString(textPosition, itemData.text, enabled ? window->Appearance->Foreground : window->Appearance->BoxBorderDisabledColor);
				}
				GUI::DrawAccessKeyUnderline(graphics, itemData.text, itemData.accessKey, itemData.accessKeyPosition, { itemData.position.X + (int)itemData.center.Width, itemData.position.Y + (int)itemData.center.Height }, window->Appearance->Foreground);
			}*/
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			if (!m_module.m_interaction.m_isMenuOpen)
			{
				m_module.m_interaction.m_selectedIndex = std::nullopt;
			}
			
			/*if (m_module.IsMenuOpen())
			{
				return;
			}
			auto savedIndex = m_module.m_interactionData.m_selectedItemIndex;
			if (savedIndex != -1)
			{
				GUI::MarkAsNeedUpdate(m_module.m_owner);
			}*/
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			if (m_module.m_interaction.m_selectedIndex.has_value())
			{
				// Toggle: Si está abierto lo cerramos, si está cerrado lo abrimos
				m_module.m_interaction.m_isMenuOpen = !m_module.m_interaction.m_isMenuOpen;

				if (m_module.m_interaction.m_isMenuOpen)
				{
					m_module.OpenMenu(false); // Abrimos con el ratón, sin auto-foco de teclado
				}
				else
				{
					//Foundation::GetInstance().GetMenuManager().CloseAll();
				}
			}
			
			/*if (!args.ButtonState.LeftButton)
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
			}*/
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			std::optional<std::size_t> newHoveredIndex = std::nullopt;

			// Hit-Testing directo (O(N) sobre N muy pequeño)
			for (size_t i = 0; i < m_module.m_layoutCache.size(); ++i)
			{
				if (m_module.m_layoutCache[i].bounds.Contains(args.Position))
				{
					newHoveredIndex = i;
					break;
				}
			}

			if (m_module.m_interaction.m_selectedIndex != newHoveredIndex)
			{
				m_module.m_interaction.m_selectedIndex = newHoveredIndex;
            
				// Si el usuario ya tenía un menú abierto y pasa el mouse sobre otro ítem de la barra,
				// cambiamos de menú automáticamente (comportamiento clásico de Win32/macOS).
				if (m_module.m_interaction.m_isMenuOpen && newHoveredIndex.has_value())
				{
					m_module.OpenMenu(false);
				}
			}
			
			/*int selectedItem = m_module.FindItem(args.Position);

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
			m_module.m_lastMousePosition = args.Position;*/
		}

		void Reactor::Resize(Graphics& graphics, const ArgResize& args)
		{
			m_module.CalculateLayout();
		}

		void Reactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
		{
			auto& interaction = m_module.m_interaction;
			auto& items = m_module.m_items;

			// 1. Manejo de "Alt + Letra" (Access Keys) interceptado desde Foundation
			//if (args.IsSystem && args.Character != 0)
			if (false)
			{
				for (size_t i = 0; i < items.size(); ++i)
				{
					//if (std::toupper(items[i].accessKey) == args.Character)
					{
						interaction.m_selectedIndex = i;
						interaction.m_isMenuOpen = true;
						m_module.OpenMenu(true); // Foco al primer ítem del menú
						return; 
					}
				}
			}

			// 2. Manejo de Navegación Estándar (Flechas)
			if (interaction.m_selectedIndex.has_value())
			{
				switch (args.Key)
				{
				case VK_RIGHT:
					m_module.MoveSelection(1);
					break;

				case VK_LEFT:
					m_module.MoveSelection(-1);
					break;
                    
				case VK_DOWN:
				case VK_RETURN:
					interaction.m_isMenuOpen = true;
					m_module.OpenMenu(true); // Foco al primer ítem
					break;

				case VK_ESCAPE:
					interaction.m_selectedIndex = std::nullopt;
					interaction.m_isMenuOpen = false;
					//Foundation::GetInstance().GetMenuManager().CloseAll();
					break;
				}
			}
			
			/*if (m_module.IsMenuOpen())
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
			}*/
		}

		Menu& Reactor::Module::At(size_t index)
		{
			return m_items.at(index).menu;
		}

		Menu& Reactor::Module::PushBack(const std::wstring& text)
		{
			wchar_t accessKey;
			std::size_t accessKeyPosition;
        
			// Lógica de atajos (ej. "&Archivo" -> extrae 'A')
			auto cleanText = GUI::GetAccessKeyText(text, accessKey, &accessKeyPosition);

			m_items.push_back(MenuBarItemData{ cleanText, accessKey, accessKeyPosition, true, Menu{} });
			CalculateLayout(); // Recalculamos al agregar un nuevo ítem

			return m_items.back().menu;
		}

		void Reactor::Module::CalculateLayout()
		{
			if (m_items.empty() || !m_owner)
			{
				return;
			}

			auto& graphics = m_owner->Renderer.GetGraphics();
			m_layoutCache.clear();
			m_layoutCache.resize(m_items.size());

			auto appearance = reinterpret_cast<Appearance*>(m_owner->Appearance.get());
			int paddingX = m_owner->ToScale(appearance->ItemPaddingInner);
			int barHeight = m_owner->ClientSize.Height;
			int currentX = m_owner->ToScale(2u); // Margen inicial izquierdo

			for (size_t i = 0; i < m_items.size(); ++i)
			{
				auto& itemData = m_items[i];
				auto& cache = m_layoutCache[i];

				auto textSize = graphics.GetTextExtent(itemData.text);
				int itemWidth = textSize.Width + (paddingX * 2);

				cache.bounds = { currentX, 0, (uint32_t)itemWidth, (uint32_t)barHeight };
            
				// Centrado vertical
				int textY = (barHeight - textSize.Height) / 2;
				cache.textPosition = { currentX + paddingX, textY };

				currentX += itemWidth;
			}
		}

		void Reactor::Module::OpenMenu(bool focusFirstItem)
		{
			if (!m_interaction.m_selectedIndex.has_value()) return;
        
			size_t index = m_interaction.m_selectedIndex.value();
			auto& itemData = m_items[index];
			auto& cache = m_layoutCache[index];

			auto& menuManager = Foundation::GetInstance().GetMenuManager();
        
			// 1. Cerramos cualquier popup abierto previamente (Ej. Si pasamos de Archivo a Edición)
			menuManager.CloseAll();

			// 2. Le indicamos a Berta que este menú nace de la barra superior (para el enrutamiento de teclado)
			// (Nota: Si tu MenuManager tiene un setter para m_fromMenuBar, podrías usarlo aquí, 
			// o pasar un flag en ShowContextMenu)
        
			// 3. Calculamos la posición: Inicia en X de su hitbox, Y justo debajo de la barra
			Point popupPos = { cache.bounds.X, cache.bounds.Y + (int)cache.bounds.Height };
        
			// Convertimos a coordenadas globales de pantalla
			popupPos = GUI::GetPointClientToScreen(m_owner, popupPos);

			// 4. ¡Instanciamos la Vista del menú!
			menuManager.ShowContextMenu(itemData.menu, m_owner, popupPos);

			// 5. Si fue abierto con teclado, pasamos el foco al primer elemento del menú recién creado
			if (focusFirstItem)
			{
				Window* activePopup = menuManager.GetActiveMenu(false);
				if (activePopup)
				{
					ArgKeyboard downArgs;
					downArgs.Key = VK_DOWN;
					//Foundation::GetInstance().ProcessEvents(activePopup, nullptr, &ControlEvents::KeyPressed, downArgs);
				}
			}
		}

		void Reactor::Module::MoveSelection(int step)
		{
			if (m_items.empty()) return;

			int count = static_cast<int>(m_items.size());
			int currentIndex = m_interaction.m_selectedIndex.value_or(step > 0 ? -1 : count); 

			// Movimiento cíclico (Da la vuelta)
			currentIndex = (currentIndex + step + count) % count;
        
			m_interaction.m_selectedIndex = currentIndex;

			// Si el usuario estaba navegando con teclado mientras un menú estaba abierto,
			// al cambiar de columna debemos abrir el menú contiguo inmediatamente.
			if (m_interaction.m_isMenuOpen)
			{
				OpenMenu(true); // true = Pasar foco automático al nuevo menú
			}
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