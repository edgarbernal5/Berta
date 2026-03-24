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
		
	    int tabBarItemHeight = static_cast<int>(m_module.m_owner->ToScale(appearance->TabBarItemHeight));
		int cornerRadius = m_module.m_owner->ToScale(4);
		
	    int width = static_cast<int>(m_module.m_owner->ClientSize.Width);
	    int height = static_cast<int>(m_module.m_owner->ClientSize.Height);
		
		size_t selectedIndex = m_module.m_selectedTabIndex.value();
		
	    for (size_t i = 0; i < m_module.m_panels.size(); ++i)
	    {
	        const auto& tabItem = m_module.m_panels[i];
	        bool isSelected = (m_module.m_selectedTabIndex == i);

	    	int xLeft = tabItem.Position.X;
	    	int tabWidth = static_cast<int>(tabItem.Size.Width);
	    	auto textColor = isSelected ? appearance->Foreground : appearance->BoxBorderDisabledColor;

	    	if (isSelected)
	    	{
	    		Rectangle activeBgRect{ Point{xLeft, tabItem.Position.Y}, Size{static_cast<uint32_t>(tabWidth), static_cast<uint32_t>(tabBarItemHeight)} };
	    		if (m_module.m_tabPosition == TabBarPosition::Top)
	    		{
	    			graphics.DrawTopRoundedRectBox(activeBgRect, (float)cornerRadius, appearance->SelectedBackgroundColor, appearance->BoxBorderColor, true, true);
	    		}
	    		else
	    		{
	    			graphics.DrawBottomRoundedRectBox(activeBgRect, (float)cornerRadius, appearance->SelectedBackgroundColor, appearance->BoxBorderColor, true, true);
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
			int accentLineY = tabBarItemHeight;
			graphics.DrawLine({ activeXStart, accentLineY }, { activeXStart + activeWidth, accentLineY }, 2.0f, appearance->AccentColor);

			graphics.DrawLine({ 0, tabBarItemHeight + 1 }, { activeXStart, tabBarItemHeight + 1 }, appearance->BoxBorderColor);
			graphics.DrawLine({ activeXStart + activeWidth, tabBarItemHeight + 1 }, { width, tabBarItemHeight + 1 }, appearance->BoxBorderColor);
		}
		else
		{
			int accentLineY = height - tabBarItemHeight - 1;
			graphics.DrawLine({ activeXStart, accentLineY }, { activeXStart + activeWidth, accentLineY }, 2.0f, appearance->AccentColor);

			int yBase = height - 2 - tabBarItemHeight;
			graphics.DrawLine({ 0, yBase }, { activeXStart, yBase }, appearance->BoxBorderColor);
			graphics.DrawLine({ activeXStart + activeWidth, yBase }, { width, yBase }, appearance->BoxBorderColor);
		}
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
		}
	}

	void TabBarReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.BuildItems();

		for (auto& tabItem : m_module.m_panels)
		{
			GUI::MoveWindow(tabItem.PanelPtr.get(), tabItem.ContentArea);
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

		m_panels.insert(m_panels.begin() + index, std::move(newItem));
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
		if (startIndex >= m_panels.size())
		{
			return;
		}
		
		int tabBarItemHeight = m_owner->ToScale(static_cast<int>(m_appearance->TabBarItemHeight));
		int tabPadding = m_owner->ToScale(10);
		int closeBtnSize = m_owner->ToScale(8);
		int spacing = m_owner->ToScale(6);

		auto& graphics = m_owner->Renderer.GetGraphics();

		int currentX = 1;
		if (startIndex > 0)
		{
			const auto& prevTab = m_panels[startIndex - 1];
			currentX = prevTab.Position.X + static_cast<int>(prevTab.Size.Width);
		}

		int clientWidth = static_cast<int>(m_owner->ClientSize.Width);
		int clientHeight = static_cast<int>(m_owner->ClientSize.Height);
		
		int tabY = (m_tabPosition == TabBarPosition::Top) ? 1 : (clientHeight - tabBarItemHeight - 1);
		
		Rectangle contentArea;
		if (m_tabPosition == TabBarPosition::Top)
		{
			contentArea = { 2, tabBarItemHeight + 2, static_cast<uint32_t>(clientWidth - 4), static_cast<uint32_t>(clientHeight - tabBarItemHeight - 4) };
		}
		else
		{
			contentArea = { 2, 2, static_cast<uint32_t>(clientWidth - 4), static_cast<uint32_t>(clientHeight - tabBarItemHeight - 4) };
		}
		
		for (size_t i = startIndex; i < m_panels.size(); ++i)
		{
			auto& tab = m_panels[i];
			auto textSize = graphics.GetTextExtent(tab.Id);

			tab.Size.Width = textSize.Width + (tabPadding * 2) + closeBtnSize + spacing;
			tab.Size.Height = tabBarItemHeight;

			tab.Position.X = currentX;
			tab.Position.Y = tabY;

			tab.Center.X = tabPadding;
			tab.Center.Y = (tabBarItemHeight - textSize.Height) / 2;

			/*tab.CloseButtonArea = {
				static_cast<uint32_t>(tabPadding + textSize.Width + spacing),
				static_cast<uint32_t>((tabBarItemHeight - closeBtnSize) / 2),
				static_cast<uint32_t>(closeBtnSize),
				static_cast<uint32_t>(closeBtnSize)
			};*/

			tab.ContentArea = contentArea;
			currentX += static_cast<int>(tab.Size.Width);
		}
	}

	bool TabBarReactor::Module::EraseTab(size_t index)
	{
		if (index >= m_panels.size())
		{
			return false;
		}
		ArgTabClosing closingArgs{ index, m_panels[index].Id };
		m_events->TabClosing.Emit(closingArgs);
		if (closingArgs.Cancel)
		{
			return false;
		}
		
		std::string idCopia{ m_panels[index].Id };
		bool removeSelectedIndex = (m_selectedTabIndex && m_selectedTabIndex.value() == index);
		
		m_panels.erase(m_panels.begin() + index);
		
		if (m_selectedTabIndex && m_selectedTabIndex.value() >= m_panels.size())
		{
			if (m_panels.empty())
			{
				m_selectedTabIndex.reset();
			}
			else
			{
				m_selectedTabIndex = m_panels.size() - 1;
			}
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
		int newWidth = std::max<int>(0, static_cast<int>(m_owner->ClientSize.Width) - 4);
		int newHeight = std::max<int>(0, static_cast<int>(m_owner->ClientSize.Height) - static_cast<int>(tabBarItemHeight) - 4);
		
		Rectangle contentArea;
		if (m_tabPosition == TabBarPosition::Top)
		{
			contentArea = { 2, static_cast<int>(tabBarItemHeight) + 2, static_cast<uint32_t>(newWidth), static_cast<uint32_t>(newHeight) };
		}
		else
		{
			contentArea = { 2, 2, static_cast<uint32_t>(newWidth), static_cast<uint32_t>(newHeight) };
		}
		GUI::MoveWindow(window, contentArea);
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
		{
			return;
		}
		
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
