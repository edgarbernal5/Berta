/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "TabBar.h"

#include <algorithm>
#include <utility>

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void WindowDeleter::operator()(Window* window) const
	{
		if (window)
		{
			GUI::DisposeWindow(window);
		}
	}
	
	void TabBarReactor::Init(ControlBase& control, Graphics* graphics)
	{
		m_control = &control;
		m_module.m_owner = control.Handle();
		m_module.m_events = reinterpret_cast<TabBarEvents*>(m_module.m_owner->Events.get());
		m_module.m_appearance = reinterpret_cast<TabBarAppearance*>(m_module.m_owner->Appearance.get());
	}

	void TabBarReactor::Update(Graphics& graphics)
	{
		auto enabled = m_control->GetEnabled();
		auto appearance = reinterpret_cast<TabBarAppearance*>(m_module.m_owner->Appearance.get());
	    
	    graphics.DrawRectangle(appearance->Background, true);

	    if (m_module.m_panels.empty() || !m_module.m_selectedTabIndex.has_value())
	    {
	        graphics.DrawRectangle(appearance->BoxBorderColor, false);
	        return;
	    }

	    // Guardar dimensiones en variables locales (KISS)
	    int tabBarItemHeight = m_module.m_owner->ToScale(appearance->TabBarItemHeight);
		int cornerRadius = m_module.m_owner->ToScale(5);
		
	    int width = static_cast<int>(m_module.m_owner->ClientSize.Width);
	    int height = static_cast<int>(m_module.m_owner->ClientSize.Height);
		
		size_t selectedIndex = m_module.m_selectedTabIndex.value();

	    // 1. DIBUJAR PESTAÑAS
	    for (size_t i = 0; i < m_module.m_panels.size(); ++i)
	    {
	        const auto& tabItem = m_module.m_panels[i];
	        bool isSelected = (m_module.m_selectedTabIndex == i);

	    	int xLeft = tabItem.Position.X;
	    	int tabWidth = static_cast<int>(tabItem.Size.Width);
	    	auto textColor = isSelected ? appearance->Foreground : appearance->BoxBorderDisabledColor;

	    	if (isSelected)
	    	{
	    		if (m_module.m_tabPosition == TabBarPosition::Top)
	    		{
	    			Rectangle activeBgRect{ Point{xLeft, 0}, Size{static_cast<uint32_t>(tabWidth), static_cast<uint32_t>(tabBarItemHeight + cornerRadius)} };
	    			//graphics.DrawRoundRectBox(activeBgRect, cornerRadius, appearance->SelectedBackgroundColor, appearance->BoxBorderColor, true, true);
	    			graphics.DrawRoundRectBox(activeBgRect, cornerRadius, appearance->SelectedBackgroundColor, appearance->BoxBorderColor, true);
	    		}
	    		else
	    		{
	    			Rectangle activeBgRect{ Point{xLeft, height - tabBarItemHeight - cornerRadius}, Size{static_cast<uint32_t>(tabWidth), static_cast<uint32_t>(tabBarItemHeight + cornerRadius)} };
	    			//graphics.DrawRoundRectBox(activeBgRect, cornerRadius, appearance->SelectedBackgroundColor, appearance->BoxBorderColor, true, true);
	    			graphics.DrawRoundRectBox(activeBgRect, cornerRadius, appearance->SelectedBackgroundColor, appearance->BoxBorderColor, true);
	    		}
	    	}

	    	// Texto
	    	graphics.DrawString({ tabItem.Center.X + xLeft, tabItem.Center.Y + tabItem.Position.Y }, tabItem.Id, textColor);

	    	// Botón de Cerrar 'X'
	    	/*int btnX = xLeft + tabItem.CloseButtonArea.Position.X;
	    	int btnY = tabItem.Position.Y + tabItem.CloseButtonArea.Position.Y;
	    	int btnS = tabItem.CloseButtonArea.Size.Width;

	    	graphics.DrawLine({ btnX, btnY }, { btnX + btnS, btnY + btnS }, textColor);
	    	graphics.DrawLine({ btnX, btnY + btnS }, { btnX + btnS, btnY }, textColor);*/
	    }

	    // 2. DIBUJAR BASE DE LAS PESTAÑAS Y BORDES DEL CONTENEDOR
		const auto& selectedTab = m_module.m_panels[selectedIndex];
		int activeXStart = selectedTab.Position.X;
		int activeWidth = static_cast<int>(selectedTab.Size.Width);

		if (m_module.m_tabPosition == TabBarPosition::Top)
		{
			int accentLineY = tabBarItemHeight - 2;
			graphics.DrawLine({ activeXStart, accentLineY }, { activeXStart + activeWidth, accentLineY }, appearance->AccentColor);

			graphics.DrawLine({ 0, tabBarItemHeight }, { activeXStart, tabBarItemHeight }, appearance->BoxBorderColor);
			graphics.DrawLine({ activeXStart + activeWidth, tabBarItemHeight }, { width, tabBarItemHeight }, appearance->BoxBorderColor);
		}
		else
		{
			int accentLineY = height - tabBarItemHeight + 1;
			graphics.DrawLine({ activeXStart, accentLineY }, { activeXStart + activeWidth, accentLineY }, appearance->AccentColor);

			int yBase = height - 1 - tabBarItemHeight;
			graphics.DrawLine({ 0, yBase }, { activeXStart, yBase }, appearance->BoxBorderColor);
			graphics.DrawLine({ activeXStart + activeWidth, yBase }, { width, yBase }, appearance->BoxBorderColor);
		}
		
		/*auto enabled = m_control->GetEnabled();
		graphics.DrawRectangle(m_module.m_owner->Appearance->Background, true);

		if (m_module.m_panels.empty() || m_module.m_selectedTabIndex == -1)
		{
			graphics.DrawRectangle(m_module.m_owner->Appearance->BoxBorderColor, false);
			return;
		}

		auto tabBarItemHeight = m_module.m_owner->ToScale((int)m_module.m_appearance->TabBarItemHeight);
		auto tabPadding = m_module.m_owner->ToScale(10);
		auto tabMarginUnselected = m_module.m_owner->ToScale(4);
		auto one = m_module.m_owner->ToScale(1);

		int lastPositionX = 0;
		int selectedPositionX = 0;
		int i = 0;
		
		for (auto tabItem = m_module.m_panels.cbegin(); tabItem != m_module.m_panels.cend(); ++i, ++tabItem)
		{
			if (m_module.m_selectedTabIndex == i)
			{
				if (m_module.m_tabPosition == TabBarPosition::Top)
				{
					graphics.DrawLine({ lastPositionX, 1 }, { lastPositionX, tabBarItemHeight }, m_module.m_owner->Appearance->BoxBorderColor);
					graphics.DrawLine({ lastPositionX + 1, 0 }, { lastPositionX + (int)tabItem->Size.Width - 1, 0 }, m_module.m_owner->Appearance->BoxBorderColor);
					graphics.DrawLine({ lastPositionX + (int)tabItem->Size.Width - 1, 1 }, { lastPositionX + (int)tabItem->Size.Width - 1, tabBarItemHeight }, m_module.m_owner->Appearance->BoxBorderColor);
				}
				else
				{
					graphics.DrawLine({ lastPositionX, (int)m_module.m_owner->ClientSize.Height - 2 }, { lastPositionX, (int)m_module.m_owner->ClientSize.Height - tabBarItemHeight }, m_module.m_owner->Appearance->BoxBorderColor);
					graphics.DrawLine({ lastPositionX + 1, (int)m_module.m_owner->ClientSize.Height - 1 }, { lastPositionX + (int)tabItem->Size.Width - 1, (int)m_module.m_owner->ClientSize.Height - 1 }, m_module.m_owner->Appearance->BoxBorderColor);
					graphics.DrawLine({ lastPositionX + (int)tabItem->Size.Width - 1, (int)m_module.m_owner->ClientSize.Height - 2 }, { lastPositionX + (int)tabItem->Size.Width - 1, (int)m_module.m_owner->ClientSize.Height - tabBarItemHeight - 1 }, m_module.m_owner->Appearance->BoxBorderColor);
				}
				graphics.DrawString({ tabItem->Center.X + lastPositionX, tabItem->Center.Y + tabItem->Position.Y }, tabItem->Id, enabled ? m_module.m_owner->Appearance->Foreground : m_module.m_owner->Appearance->BoxBorderDisabledColor);
				selectedPositionX = lastPositionX;
			}
			else
			{
				if (m_module.m_tabPosition == TabBarPosition::Top)
				{
					graphics.DrawLine({ lastPositionX, 1 + tabMarginUnselected }, { lastPositionX, tabBarItemHeight }, m_module.m_owner->Appearance->BoxBorderColor);

					graphics.DrawLine({ lastPositionX + 1, tabMarginUnselected }, { lastPositionX + (int)tabItem->Size.Width - 1, tabMarginUnselected }, m_module.m_owner->Appearance->BoxBorderColor);
					graphics.DrawLine({ lastPositionX + (int)tabItem->Size.Width - 1, tabMarginUnselected + 1 }, { lastPositionX + (int)tabItem->Size.Width - 1, tabBarItemHeight }, m_module.m_owner->Appearance->BoxBorderColor);

					graphics.DrawString({ tabItem->Center.X + lastPositionX, tabItem->Center.Y + one + tabItem->Position.Y }, tabItem->Id, enabled ? m_module.m_owner->Appearance->Foreground : m_module.m_owner->Appearance->BoxBorderDisabledColor);
				}
				else
				{
					graphics.DrawLine({ lastPositionX, (int)m_module.m_owner->ClientSize.Height - 2 - tabMarginUnselected }, { lastPositionX, (int)m_module.m_owner->ClientSize.Height - 2 - tabBarItemHeight }, m_module.m_owner->Appearance->BoxBorderColor);
					
					graphics.DrawLine({ lastPositionX + 1, (int)m_module.m_owner->ClientSize.Height - 1 - tabMarginUnselected }, { lastPositionX + (int)tabItem->Size.Width - 1, (int)m_module.m_owner->ClientSize.Height - 1 - tabMarginUnselected }, m_module.m_owner->Appearance->BoxBorderColor);
					graphics.DrawLine({ lastPositionX + (int)tabItem->Size.Width - 1, (int)m_module.m_owner->ClientSize.Height - 2 - tabMarginUnselected }, { lastPositionX + (int)tabItem->Size.Width - 1, (int)m_module.m_owner->ClientSize.Height -2 - tabBarItemHeight }, m_module.m_owner->Appearance->BoxBorderColor);
					
					graphics.DrawString({ tabItem->Center.X + lastPositionX, tabItem->Center.Y - one + tabItem->Position.Y }, tabItem->Id, enabled ? m_module.m_owner->Appearance->Foreground : m_module.m_owner->Appearance->BoxBorderDisabledColor);
				}
				
			}
			lastPositionX += (int)tabItem->Size.Width;
		}
		auto selectedTabItem = m_module.At(m_module.m_selectedTabIndex);
		if (selectedPositionX > 0)
		{
			if (m_module.m_tabPosition == TabBarPosition::Top)
			{
				graphics.DrawLine({ 0, tabBarItemHeight }, { selectedPositionX, tabBarItemHeight }, m_module.m_owner->Appearance->BoxBorderColor);
			}
			else
			{
				graphics.DrawLine({ 0, (int)m_module.m_owner->ClientSize.Height - 1 - tabBarItemHeight }, { selectedPositionX, (int)m_module.m_owner->ClientSize.Height - 1 - tabBarItemHeight }, m_module.m_owner->Appearance->BoxBorderColor);
			}
		}

		if (m_module.m_tabPosition == TabBarPosition::Top)
		{
			graphics.DrawLine({ 0, tabBarItemHeight }, { 0, (int)m_module.m_owner->ClientSize.Height }, m_module.m_owner->Appearance->BoxBorderColor);
			graphics.DrawLine({ 0, (int)m_module.m_owner->ClientSize.Height - 1 }, { (int)m_module.m_owner->ClientSize.Width, (int)m_module.m_owner->ClientSize.Height - 1 }, m_module.m_owner->Appearance->BoxBorderColor);
			graphics.DrawLine({ (int)m_module.m_owner->ClientSize.Width - 1, tabBarItemHeight + 1 }, { (int)m_module.m_owner->ClientSize.Width - 1, (int)m_module.m_owner->ClientSize.Height }, m_module.m_owner->Appearance->BoxBorderColor);

			if (selectedPositionX + (int)selectedTabItem->Size.Width < (int)m_module.m_owner->ClientSize.Width)
			{
				graphics.DrawLine({ selectedPositionX + (int)selectedTabItem->Size.Width,tabBarItemHeight }, { (int)m_module.m_owner->ClientSize.Width, tabBarItemHeight }, m_module.m_owner->Appearance->BoxBorderColor);
			}
		}
		else
		{
			graphics.DrawLine({ 0, (int)m_module.m_owner->ClientSize.Height - tabBarItemHeight }, { 0, 0 }, m_module.m_owner->Appearance->BoxBorderColor);
			graphics.DrawLine({ 0, 0 }, { (int)m_module.m_owner->ClientSize.Width, 0 }, m_module.m_owner->Appearance->BoxBorderColor);
			graphics.DrawLine({ (int)m_module.m_owner->ClientSize.Width - 1, 0 }, { (int)m_module.m_owner->ClientSize.Width - 1, (int)m_module.m_owner->ClientSize.Height - tabBarItemHeight - 1 }, m_module.m_owner->Appearance->BoxBorderColor);

			if (selectedPositionX + (int)selectedTabItem->Size.Width < (int)m_module.m_owner->ClientSize.Width)
			{
				graphics.DrawLine({ selectedPositionX + (int)selectedTabItem->Size.Width, (int)m_module.m_owner->ClientSize.Height - 1 - tabBarItemHeight }, { (int)m_module.m_owner->ClientSize.Width, (int)m_module.m_owner->ClientSize.Height - 1 - tabBarItemHeight }, m_module.m_owner->Appearance->BoxBorderColor);
			}
		}*/
	}

	void TabBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		/*for (size_t i = 0; i < m_module.m_panels.size(); ++i)
		{
			auto& tab = m_module.m_panels[i];
			Rectangle absCloseBtn = tab.CloseButtonArea;
			absCloseBtn.Position.X += tab.Position.X;
			absCloseBtn.Position.Y += tab.Position.Y;

			if (absCloseBtn.IsInside(args.Position))
			{
				m_module.EraseTab(i);
				return;
			}
		}*/
		
		auto newSelectedIndex = m_module.FindItem(args.Position);
		if (!newSelectedIndex.has_value())
		{
			return;
		}
		
		if (m_module.m_selectedTabIndex != newSelectedIndex) 
		{
			auto& selectedTabItem = m_module.m_panels[*m_module.m_selectedTabIndex];
			GUI::ShowWindow(selectedTabItem.PanelPtr.get(), false);
			
			m_module.m_selectedTabIndex = newSelectedIndex;

			auto& newSelectedTabItem = m_module.m_panels[*newSelectedIndex];
			GUI::ShowWindow(newSelectedTabItem.PanelPtr.get(), true);

			ArgTabBar argsTabBar{ newSelectedIndex.value(), newSelectedTabItem.Id };
			m_module.m_events->TabChanged.Emit(argsTabBar);

			GUI::MarkAsNeedUpdate(m_module.m_owner);
			
			/*auto selectedTabItem = m_module.At(m_module.m_selectedTabIndex);

			GUI::ShowWindow(selectedTabItem->PanelPtr, false);
			m_module.SelectIndex(newSelectedIndex);

			auto newSelectedTabItem = m_module.At(newSelectedIndex);
			GUI::ShowWindow(newSelectedTabItem->PanelPtr, true);

			ArgTabBar argsTabBar{ newSelectedTabItem->Id };
			m_module.m_events->TabChanged.Emit(argsTabBar);

			GUI::MarkAsNeedUpdate(m_module.m_owner);*/
		}
	}

	void TabBarReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.BuildItems();

		for (auto& tabItem : m_module.m_panels)
		{
			GUI::MoveWindow(tabItem.PanelPtr.get(), tabItem.PanelArea);
		}
	}

	bool TabBarReactor::Module::Clear()
	{
		bool needUpdate = !m_panels.empty();
		m_selectedTabIndex.reset();
		m_panels.clear();

		return needUpdate;
	}

	bool TabBarReactor::Module::InsertTab(size_t index, std::string tabId, Window* window)
	{
		index = std::min<size_t>(index, m_panels.size());
		int startIndex = static_cast<int>(index);

		GUI::SetParentWindow(window, m_owner);
		
		PanelItem newItem;
		newItem.Id = std::move(tabId);
		newItem.PanelPtr.reset(window);

		auto newIt = m_panels.insert(m_panels.begin() + index, std::move(newItem));
		UpdatePanelMoveRect(window);

		if (!m_selectedTabIndex)
		{
			m_selectedTabIndex = 0;
		}
		else if (m_selectedTabIndex.value() == index)
		{
			GUI::ShowWindow(m_panels[index + 1].PanelPtr.get(), false);
		}
		else
		{
			GUI::ShowWindow(window, false);
		}

		BuildItems(startIndex);
		return true;
	}

	void TabBarReactor::Module::BuildItems(size_t startIndex)
	{
		if (startIndex >= m_panels.size()) return;

		int tabBarItemHeight = m_owner->ToScale(static_cast<int>(m_appearance->TabBarItemHeight));
		int tabPadding = m_owner->ToScale(10);
		int closeBtnSize = m_owner->ToScale(8);
		int spacing = m_owner->ToScale(6);

		auto& graphics = m_owner->Renderer.GetGraphics();

		int currentX = 0;
		if (startIndex > 0)
		{
			const auto& prevTab = m_panels[startIndex - 1];
			currentX = prevTab.Position.X + static_cast<int>(prevTab.Size.Width);
		}

		int clientWidth = static_cast<int>(m_owner->ClientSize.Width);
		int clientHeight = static_cast<int>(m_owner->ClientSize.Height);
		Rectangle contentArea;

		if (m_tabPosition == TabBarPosition::Top)
			contentArea = { 0, tabBarItemHeight, static_cast<uint32_t>(clientWidth), static_cast<uint32_t>(clientHeight - tabBarItemHeight) };
		else
			contentArea = { 0, 0, static_cast<uint32_t>(clientWidth), static_cast<uint32_t>(clientHeight - tabBarItemHeight) };

		for (size_t i = startIndex; i < m_panels.size(); ++i)
		{
			auto& tab = m_panels[i];
			auto textSize = graphics.GetTextExtent(tab.Id);

			tab.Size.Width = textSize.Width + (tabPadding * 2) + closeBtnSize + spacing;
			tab.Size.Height = tabBarItemHeight;

			tab.Position.X = currentX;
			tab.Position.Y = 0;

			tab.Center.X = tabPadding;
			tab.Center.Y = (tabBarItemHeight - textSize.Height) / 2;

			/*tab.CloseButtonArea = {
				static_cast<uint32_t>(tabPadding + textSize.Width + spacing),
				static_cast<uint32_t>((tabBarItemHeight - closeBtnSize) / 2),
				static_cast<uint32_t>(closeBtnSize),
				static_cast<uint32_t>(closeBtnSize)
			};*/

			tab.PanelArea = contentArea;
			currentX += static_cast<int>(tab.Size.Width);
		}
		
		/*if (startIndex >= m_panels.size())
		{
			return;
		}

		auto tabBarItemHeight = m_owner->ToScale(m_appearance->TabBarItemHeight);
		auto tabPadding = m_owner->ToScale(10u);

		Point offset{ 0, 0 };
		Point tabPositionOffset{};
		if (m_tabPosition == TabBarPosition::Bottom)
		{
			tabPositionOffset.Y = m_owner->ClientSize.Height > tabBarItemHeight ? static_cast<int>(m_owner->ClientSize.Height - tabBarItemHeight) : 0 ;
		}

		int newWidth = std::max<int>(0, static_cast<int>(m_owner->ClientSize.Width) - 4);
		int newHeight =  std::max<int>(0, static_cast<int>(m_owner->ClientSize.Height) - static_cast<int>(tabBarItemHeight) - 4);
		Rectangle panelTabArea;
		if (m_tabPosition == TabBarPosition::Top)
		{
			panelTabArea = { 2, static_cast<int>(tabBarItemHeight) + 2, static_cast<uint32_t>(newWidth), static_cast<uint32_t>(newHeight) };
		}
		else
		{
			panelTabArea = { 2, 2, static_cast<uint32_t>(newWidth), static_cast<uint32_t>(newHeight) };
		}

		if (startIndex > 0)
		{
			auto element = At(startIndex - 1);
			offset.X = element->Position.X + static_cast<int>(element->Size.Width);
		}

		auto current = At(startIndex);
		for (size_t i = startIndex; i < m_panels.size(); ++i, ++current)
		{
			auto textSize = m_owner->Renderer.GetGraphics().GetTextExtent(current->Id);
			Size itemSize{ textSize.Width + tabPadding, tabBarItemHeight };

			Point center{ static_cast<int>(itemSize.Width) - static_cast<int>(textSize.Width), static_cast<int>(itemSize.Height) - static_cast<int>(textSize.Height) };
			center >>= 1;

			Point itemPos = offset + tabPositionOffset;
			current->Position = itemPos;
			current->Size = itemSize;
			current->Center = center;

			current->PanelArea = panelTabArea;

			offset.X += static_cast<int>(itemSize.Width);
		}*/
	}

	bool TabBarReactor::Module::EraseTab(size_t index)
	{
		if (index >= m_panels.size())
		{
			return false;
		}
		ArgTabClosing closingArgs{ index, m_panels[index].Id };
		m_events->TabClosing.Emit(closingArgs);
		if (closingArgs.Cancel) return false;
		
		std::string idCopia{ m_panels[index].Id };
		bool removeSelectedIndex = (m_selectedTabIndex && m_selectedTabIndex.value() == index);
		
		m_panels.erase(m_panels.begin() + index);
		
		if (m_selectedTabIndex && m_selectedTabIndex.value() >= m_panels.size())
		{
			if (m_panels.empty()) m_selectedTabIndex.reset();
			else m_selectedTabIndex = m_panels.size() - 1;
		}

		if (removeSelectedIndex && m_selectedTabIndex)
		{
			size_t newIdx = m_selectedTabIndex.value();
			ArgTabBar argsTabBar{ newIdx, m_panels[newIdx].Id };
			m_events->TabChanged.Emit(argsTabBar);
			GUI::ShowWindow(m_panels[newIdx].PanelPtr.get(), true);
		}
		ArgTabBar closedArgs{ index, idCopia };
		m_events->TabClosed.Emit(closedArgs);
		
		BuildItems(index);
		return true;
	}

	std::optional<size_t> TabBarReactor::Module::FindItem(const Point& position) const
	{
		for (size_t i = 0; i < m_panels.size(); ++i)
		{
			if (Rectangle{ m_panels[i].Position, m_panels[i].Size }.IsInside(position))
			{
				return i;
			}
		}
		return std::nullopt;
	}

	std::optional<size_t> TabBarReactor::Module::GetSelectedIndex() const
	{
		if (m_panels.empty())
		{
			return std::nullopt;
		}

		return m_selectedTabIndex;
	}

	void TabBarReactor::Module::UpdatePanelMoveRect(Window* window) const
	{
		auto tabBarItemHeight = m_owner->ToScale(m_appearance->TabBarItemHeight);
		int newWidth = (std::max)(0, static_cast<int>(m_owner->ClientSize.Width) - 4);
		int newHeight = (std::max)(0, static_cast<int>(m_owner->ClientSize.Height) - static_cast<int>(tabBarItemHeight) - 4);
		
		Rectangle rect;
		if (m_tabPosition == TabBarPosition::Top)
		{
			rect = { 2, (int)tabBarItemHeight + 2, static_cast<uint32_t>(newWidth), static_cast<uint32_t>(newHeight) };
		}
		else
		{
			rect = { 2, 2, static_cast<uint32_t>(newWidth), static_cast<uint32_t>(newHeight) };
		}
		GUI::MoveWindow(window, rect);
	}

	TabBar::TabBar(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "TabBar";
#endif
	}

	void TabBar::Clear()
	{
		auto& module = GetReactor().GetModule();
		module.Clear();
	}
	
	size_t TabBar::Count() const
	{
		return GetReactor().GetModule().m_panels.size();
	}

	void TabBar::Erase(size_t index)
	{
		auto& module = GetReactor().GetModule();
		module.EraseTab(index);
	}
	
	void TabBar::Insert(size_t position, std::string tabId, Window* window)
	{
		auto& module = GetReactor().GetModule();
		module.InsertTab(position, std::move(tabId), window);
	}

	void TabBar::PushBack(const std::string& tabId, Window* window)
	{
		auto& module = GetReactor().GetModule();
		module.AddTab(tabId, window);
	}

	void TabBar::SetTabBarPosition(TabBarPosition position)
	{
		auto& module = GetReactor().GetModule();
		
		if (module.m_tabPosition == position)
			return;
		
		module.m_tabPosition = position;
		module.BuildItems();
	}

	bool TabBarReactor::Module::AddTab(std::string tabId, Window* window)
	{
		auto startIndex = m_panels.size();
		auto& newItem = m_panels.emplace_back();
		newItem.Id = std::move(tabId);
		newItem.PanelPtr.reset(window);

		GUI::SetParentWindow(window, m_owner);
		
		UpdatePanelMoveRect(window);

		if (!m_selectedTabIndex)
		{
			m_selectedTabIndex = 0;
		}
		else
		{
			GUI::ShowWindow(window, false);
		}

		BuildItems(startIndex);

		return true;
	}

	std::optional<size_t> TabBar::GetSelectedIndex() const
	{
		return GetReactor().GetModule().GetSelectedIndex();
	}
}
