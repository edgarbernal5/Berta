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
	namespace Internal::MenuBar
	{
		void Reactor::DoOnInit()
		{
			m_module.m_control = m_control;
			m_module.m_owner = m_control->Handle();
			
			auto& menuManager = Foundation::GetInstance().GetMenuManager();
			m_module.m_closeListenerId = menuManager.SubscribeOnClose([this]() 
			{
				if (m_module.m_interaction.m_isMenuOpen)
				{
					m_module.m_interaction.m_isMenuOpen = false;
					
					Point localPos = GUI::GetMousePositionToWindow(m_module.m_owner);
					if (!m_module.m_owner->ClientSize.ToRectangle().Contains(localPos))
					{
						m_module.m_interaction.m_selectedIndex = std::nullopt;
					}

					GUI::MarkAsNeedUpdate(m_module.m_owner);
				}
			});
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
            
				bool isSelected = m_module.m_interaction.m_selectedIndex == i;
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
				if (m_module.m_interaction.m_isMenuOpen)
				{
					m_module.OpenMenu(false);
				}
				else
				{
					Foundation::GetInstance().GetMenuManager().CloseAll();
					
					std::optional<std::size_t> hitIndex = std::nullopt;

					for (size_t i = 0; i < m_module.m_layoutCache.size(); ++i)
					{
						if (m_module.m_layoutCache[i].bounds.Contains(args.Position))
						{
							hitIndex = i;
							break;
						}
					}

					m_module.m_interaction.m_selectedIndex = hitIndex;
				}
				
				GUI::MarkAsNeedUpdate(m_module.m_owner);
			}
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			if (!m_module.m_lastMousePos.has_value())
			{
				m_module.m_lastMousePos = args.Position;
				return; 
			}

			if (m_module.m_lastMousePos.value().X == args.Position.X && 
				m_module.m_lastMousePos.value().Y == args.Position.Y)
			{
				return; 
			}

			m_module.m_lastMousePos = args.Position;
			
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
			auto& menuManager = Foundation::GetInstance().GetMenuManager();
			/*if (args.Key == KeyboardKey::Alt)
			{
				if (menuManager.AnyPopupActive() || m_module.m_interaction.m_selectedIndex.has_value())
				{
					menuManager.CloseAll();
					m_module.m_interaction.m_selectedIndex = std::nullopt;
					m_module.m_interaction.m_isMenuOpen = false;
				}
				else if (!m_module.m_items.empty())
				{
					// Si la app estaba en reposo, activamos el primer ítem (solo resaltado, sin abrir popups)
					m_module.m_interaction.m_selectedIndex = 0;
					m_module.m_interaction.m_isMenuOpen = false;
				}
        
				GUI::MarkAsNeedUpdate(m_module.m_owner);
				return;
			}
			if (!m_module.m_interaction.m_selectedIndex.has_value())
				return;*/
			
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
					m_module.OpenMenu(true);
					break;

				case KeyboardKey::Escape:
					interaction.m_selectedIndex = std::nullopt;
					interaction.m_isMenuOpen = false;
					menuManager.CloseAll();
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
        
			auto cleanText = GUI::GetAccessKeyText(text, accessKey, &accessKeyPosition);

			m_items.emplace_back(MenuBarItemData{ cleanText, accessKey, accessKeyPosition, true, Menu{} });
			CalculateLayout();

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
			int currentX = 0;

			for (size_t i = 0; i < m_items.size(); ++i)
			{
				auto& itemData = m_items[i];
				auto& cache = m_layoutCache[i];

				auto textSize = graphics.GetTextExtent(itemData.text);
				int itemWidth = (int)textSize.Width + (paddingX * 2);

				cache.bounds = { currentX, 0, static_cast<uint32_t>(itemWidth), static_cast<uint32_t>(barHeight) };
				
				int textY = (barHeight - static_cast<int>(textSize.Height)) / 2;
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

			Window* activePopup = menuManager.GetTopPopup();
			if (activePopup && focusFirstItem)
			{
				ArgKeyboard downArgs;
				downArgs.Key = KeyboardKey::ArrowDown;
				Foundation::GetInstance().ProcessEvents<ArgKeyboard>(activePopup, &Renderer::KeyPressed, nullptr, downArgs);
			}
		}

		void Reactor::Module::MoveSelection(int step)
		{
			if (m_items.empty())
			{
				return;
			}

			int count = static_cast<int>(m_items.size());
			int currentIndex = static_cast<int>(m_interaction.m_selectedIndex.value_or(step > 0 ? -1 : count)); 

			currentIndex = (currentIndex + step + count) % count;
        
			m_interaction.m_selectedIndex = currentIndex;

			GUI::MarkAsNeedUpdate(m_owner);
			
			if (m_interaction.m_isMenuOpen)
			{
				OpenMenu(true);
			}
		}
	}

	MenuBar::MenuBar(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

		GUI::SetMenuBar(m_handle);
#if BT_DEBUG
		m_handle->Name = "MenuBar";
#endif
	}

	MenuBar::~MenuBar()
	{
		auto& module = GetReactor().GetModule();
		if (module.m_closeListenerId != 0)
		{
			auto& menuManager = Foundation::GetInstance().GetMenuManager();
			menuManager.UnsubscribeOnClose(module.m_closeListenerId);
			module.m_closeListenerId = 0;
		}
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