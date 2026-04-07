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
			auto appearance = reinterpret_cast<Appearance*>(window->Appearance.get());
			bool enabled = m_module.m_control->GetEnabled();

			graphics.FillRectangle(window->ClientSize.ToRectangle(), enabled ? appearance->ButtonBackground : appearance->ButtonDisabledBackground);

			for (size_t i = 0; i < m_module.m_layoutCache.size(); ++i)
			{
				const auto& itemData = m_module.m_items[i];
				const auto& cache = m_module.m_layoutCache[i];
            
				bool isSelected = (m_module.m_interaction.m_selectedIndex == i);
				bool isOpen = m_module.m_interaction.m_isMenuOpen;

				if (isSelected && enabled)
				{
					Color bgColor = isOpen ? appearance->BoxPressedBackground : appearance->ButtonHighlightBackground;
					graphics.FillRectangle(cache.bounds, bgColor);
				}

				Color textColor = enabled ? appearance->Foreground : appearance->ButtonDisabledBackground;
				graphics.DrawString(cache.textPosition, itemData.text, textColor);

				if (enabled && itemData.accessKey != 0)
				{
					GUI::DrawAccessKeyUnderline(graphics, itemData.text, itemData.accessKey, 
						itemData.accessKeyPosition, cache.textPosition, textColor);
				}
			}
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			if (!m_module.m_interaction.m_isMenuOpen)
			{
				if (m_module.m_interaction.m_selectedIndex.has_value())
				{
					m_module.m_interaction.m_selectedIndex = std::nullopt;
					GUI::MarkAsNeedUpdate(m_module.m_owner);
				}
			}
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			if (m_module.m_interaction.m_selectedIndex.has_value())
			{
				m_module.m_interaction.m_isMenuOpen = !m_module.m_interaction.m_isMenuOpen;
				GUI::MarkAsNeedUpdate(m_module.m_owner);
				
				if (m_module.m_interaction.m_isMenuOpen)
				{
					m_module.OpenMenu(false);
				}
				else
				{
					Foundation::GetInstance().GetMenuManager().CloseAll();
				}
			}
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			std::optional<std::size_t> hitIndex = std::nullopt;

			for (size_t i = 0; i < m_module.m_layoutCache.size(); ++i)
			{
				if (m_module.m_layoutCache[i].bounds.Contains(args.Position))
				{
					hitIndex = i;
					break;
				}
			}

			if (hitIndex.has_value() && m_module.m_interaction.m_selectedIndex != hitIndex)
			{
				m_module.m_interaction.m_selectedIndex = hitIndex;
				GUI::MarkAsNeedUpdate(m_module.m_owner);

				if (m_module.m_interaction.m_isMenuOpen)
				{
					m_module.OpenMenu(false);
				}
			}
			else if (!m_module.m_interaction.m_isMenuOpen && !hitIndex.has_value())
			{
				if (m_module.m_interaction.m_selectedIndex.has_value())
				{
					m_module.m_interaction.m_selectedIndex = std::nullopt;
					GUI::MarkAsNeedUpdate(m_module.m_owner);
				}
			}
		}

		void Reactor::Resize(Graphics& graphics, const ArgResize& args)
		{
			m_module.CalculateLayout();
		}

		void Reactor::DpiChanged(Graphics& graphics)
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
				case KeyboardKey::ArrowRight:
					m_module.MoveSelection(1);
					break;

				case KeyboardKey::ArrowLeft:
					m_module.MoveSelection(-1);
					break;
                    
				case KeyboardKey::ArrowDown:
				case KeyboardKey::Enter:
					interaction.m_isMenuOpen = true;
					m_module.OpenMenu(true); // Foco al primer ítem
					break;

				case KeyboardKey::Escape:
					interaction.m_selectedIndex = std::nullopt;
					interaction.m_isMenuOpen = false;
					Foundation::GetInstance().GetMenuManager().CloseAll();
					break;
				}
			}
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
			auto appearance = reinterpret_cast<Appearance*>(m_owner->Appearance.get());
			
			m_layoutCache.clear();
			m_layoutCache.resize(m_items.size());

			int paddingX = (int)m_owner->ToScale(appearance->ItemPaddingInner);
			int barHeight = (int)m_owner->ClientSize.Height;
			int currentX = m_owner->ToScale(2); // Margen inicial izquierdo

			for (size_t i = 0; i < m_items.size(); ++i)
			{
				auto& itemData = m_items[i];
				auto& cache = m_layoutCache[i];

				auto textSize = graphics.GetTextExtent(itemData.text);
				int itemWidth = (int)textSize.Width + (paddingX * 2);

				cache.bounds = { currentX, 0, (uint32_t)itemWidth, (uint32_t)barHeight };
            
				// Centrado vertical
				int textY = (barHeight - textSize.Height) / 2;
				cache.textPosition = { currentX + paddingX, textY };

				currentX += itemWidth;
			}
		}

		void Reactor::Module::OpenMenu(bool focusFirstItem)
		{
			if (!m_interaction.m_selectedIndex.has_value())
			{
				return;
			}
        
			size_t index = m_interaction.m_selectedIndex.value();
			auto& itemData = m_items[index];
			auto& cache = m_layoutCache[index];

			auto& menuManager = Foundation::GetInstance().GetMenuManager();
			menuManager.CloseAll();
			
			m_interaction.m_selectedIndex = index;
			m_interaction.m_isMenuOpen = true;
			
			Point popupPos = { cache.bounds.X, cache.bounds.Y + (int)cache.bounds.Height };
			menuManager.ShowMenuBarPopup(itemData.menu, m_owner, popupPos);

			Window* activePopup = menuManager.GetActiveMenu(false);
			if (activePopup)
			{
				activePopup->Events->Destroy.Connect([this](const ArgDestroy& args)
				{
					auto& manager = Foundation::GetInstance().GetMenuManager();
					if (manager.GetPopupCount() == 0)
					{
						m_interaction.m_isMenuOpen = false;
			    
						Point localPos = Berta::GUI::GetMousePositionToWindow(m_owner);
			    
						if (!m_owner->ClientSize.ToRectangle().Contains(localPos))
						{
							m_interaction.m_selectedIndex = std::nullopt;
						}
			    
						GUI::MarkAsNeedUpdate(m_owner);
					}
				});
			}
			
			if (focusFirstItem)
			{
			}
		}

		void Reactor::Module::MoveSelection(int step)
		{
			if (m_items.empty())
			{
				return;
			}

			int count = static_cast<int>(m_items.size());
			int currentIndex = m_interaction.m_selectedIndex.value_or(step > 0 ? -1 : count); 

			// Movimiento cíclico (Da la vuelta)
			currentIndex = (currentIndex + step + count) % count;
        
			m_interaction.m_selectedIndex = currentIndex;

			GUI::MarkAsNeedUpdate(m_owner);
			
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