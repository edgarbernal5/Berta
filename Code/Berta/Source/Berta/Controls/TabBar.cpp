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
	void TabWindowDeleter::operator()(Window* window) const
	{
		if (window)
		{
			GUI::DisposeWindow(window);
		}
	}
	
	void TabBarReactor::DoOnInit()
	{
		m_module.m_owner = m_control->Handle();
		m_module.m_events = reinterpret_cast<TabBarEvents*>(m_module.m_owner->Events.get());
	}

	void TabBarReactor::Update(Graphics& graphics)
	{
		auto enabled = m_control->GetEnabled();
		auto appearance = reinterpret_cast<TabBarAppearance*>(m_module.m_owner->Appearance.get());
	    auto clientRect = m_module.m_owner->ClientSize.ToRectangle();
		
	    graphics.FillRectangle(clientRect, appearance->Background);

	    if (m_module.m_panels.empty() || !m_module.m_selectedTabIndex)
	    {
	        graphics.DrawRectangle(clientRect, appearance->BoxBorderColor);
	    	clientRect.X = clientRect.Y = 1;
	    	clientRect.Width -= 2;
	    	clientRect.Height -= 2;
	        graphics.DrawRectangle(clientRect, appearance->InnerHighlightColor);
	        return;
	    }
		
	    int tabBarItemHeight = static_cast<int>(m_module.m_owner->ToScale(appearance->TabBarItemHeight));
		auto cornerRadius = m_module.m_owner->ToScale(4.0f);
		auto iconSize = m_module.m_owner->ToScale(appearance->SmallIconSize);
		auto iconMargin = m_module.m_owner->ToScale(6);
		
	    int clientWidth = static_cast<int>(m_module.m_owner->ClientSize.Width);
	    int clientHeight = static_cast<int>(m_module.m_owner->ClientSize.Height);
		
		size_t selectedIndex = m_module.m_selectedTabIndex.value();
		
		Rectangle contentArea = m_module.GetTabPageArea(false);
	    graphics.FillRectangle(contentArea, appearance->TabBackgroundColor);
		
	    for (size_t i = 0; i < m_module.m_panels.size(); ++i)
	    {
	        const auto& tabItem = m_module.m_panels[i];
	        bool isSelected = (m_module.m_selectedTabIndex == i);

	    	int xLeft = tabItem.Position.X;
	    	int tabWidth = static_cast<int>(tabItem.Size.Width);
	    	auto textColor = isSelected ? appearance->Foreground : appearance->Foreground2nd;

	    	if (isSelected)
	    	{
	    		Rectangle activeBgRect{ Point{xLeft, tabItem.Position.Y}, Size{static_cast<uint32_t>(tabWidth), static_cast<uint32_t>(tabBarItemHeight + 1)} };
	    		if (m_module.m_tabRowPosition == TabRowPosition::Top)
	    		{
	    			graphics.FillTopRoundedRectangle(activeBgRect, cornerRadius, appearance->TabBackgroundColor);
	    			
	    			Rectangle highlightRect{ 
	    				Point{xLeft + 1, tabItem.Position.Y + 1}, 
						Size{static_cast<uint32_t>(tabWidth - 2), static_cast<uint32_t>(tabBarItemHeight - 2)} 
	    			};
	    			
	    			graphics.DrawTopRoundedRectangle(highlightRect, cornerRadius - 1.0f, appearance->InnerHighlightColor, false);
	    			graphics.DrawTopRoundedRectangle(activeBgRect, cornerRadius, appearance->BoxBorderColor, false);
	    		}
	    		else
	    		{
	    			graphics.FillBottomRoundedRectangle(activeBgRect, cornerRadius, appearance->TabBackgroundColor);
	    			
	    			Rectangle highlightRect{ 
	    				Point{xLeft + 1, tabItem.Position.Y + 2}, 
						Size{static_cast<uint32_t>(tabWidth - 2), static_cast<uint32_t>(tabBarItemHeight - 2)} 
	    			};
	    			
	    			graphics.DrawBottomRoundedRectangle(highlightRect, cornerRadius - 1.0f, appearance->InnerHighlightColor, false);
	    			graphics.DrawBottomRoundedRectangle(activeBgRect, cornerRadius, appearance->BoxBorderColor, false);
	    		}
	    	}
		    else
		    {
		    	if (m_module.m_tabRowPosition == TabRowPosition::Top)
			    {
				    int yOffset = 3;
		    		Rectangle inactiveBgRect{ 
		    			Point{xLeft, tabItem.Position.Y + yOffset}, 
						Size{static_cast<uint32_t>(tabWidth), static_cast<uint32_t>(tabBarItemHeight - yOffset + 1)} 
		    		};

		    		// Relleno y borde cerrado (la línea horizontal que dibujaremos después pasará por encima)
		    		graphics.FillTopRoundedRectangle(inactiveBgRect, cornerRadius, appearance->Background);
		    		graphics.DrawTopRoundedRectangle(inactiveBgRect, cornerRadius, appearance->BoxBorderColor, false);
			    }
			    else
			    {
			    	int yOffset = 3;
			    	Rectangle inactiveBgRect{ 
			    		Point{xLeft, tabItem.Position.Y}, 
						Size{static_cast<uint32_t>(tabWidth), static_cast<uint32_t>(tabBarItemHeight - yOffset + 1)} 
			    	};

			    	// Relleno y borde cerrado (la línea horizontal que dibujaremos después pasará por encima)
			    	graphics.FillBottomRoundedRectangle(inactiveBgRect, cornerRadius, appearance->Background);
			    	graphics.DrawBottomRoundedRectangle(inactiveBgRect, cornerRadius, appearance->BoxBorderColor, false);
			    }
		    }
	    	
	    	//Icono
	    	if (tabItem.Icon)
	    	{
	    		Rectangle iconRect{ xLeft + iconMargin, tabItem.Position.Y + (tabBarItemHeight- (int)iconSize)/2, iconSize, iconSize };
	    		tabItem.Icon.Paste(graphics, iconRect);
	    	}
	    	
	    	// Texto
	    	graphics.DrawString({ tabItem.Center.X + xLeft, tabItem.Center.Y + tabItem.Position.Y }, tabItem.Id, textColor);

	    	// Botón de Cerrar 'X'
	    	if (m_module.m_showCloseButton)
	    	{
	    		int btnX = xLeft + tabItem.CloseButtonArea.X;
	    		int btnY = tabItem.Position.Y + tabItem.CloseButtonArea.Y;
	    		int btnS = static_cast<int>(tabItem.CloseButtonArea.Width);

	    		bool isHovered = (m_module.m_hoveredCloseBtnIndex.has_value() && m_module.m_hoveredCloseBtnIndex.value() == i);
	    		if (isHovered)
	    		{
	    			Rectangle btnRect{ Point{btnX, btnY}, Size{static_cast<uint32_t>(btnS), static_cast<uint32_t>(btnS)} };
	    			graphics.FillRectangle(btnRect, appearance->MenuBackground);
	    		}
	    		graphics.DrawLine({ btnX + 2, btnY + 2 }, { btnX - 2 + btnS, btnY - 2 + btnS }, textColor);
	    		graphics.DrawLine({ btnX + 2, btnY - 2 + btnS }, { btnX - 2 + btnS, btnY + 2 }, textColor);
	    	}
	    }

	    // 2. DIBUJAR BASE DE LAS PESTAÑAS Y BORDES DEL CONTENEDOR
		const auto& selectedTab = m_module.m_panels[selectedIndex];
		int activeXStart = selectedTab.Position.X + 1;
		int activeWidth = static_cast<int>(selectedTab.Size.Width) - 2;

		int accentThickness = m_module.m_owner->ToScale(3); 
		if (m_module.m_tabRowPosition == TabRowPosition::Top)
		{
			int accentLineY = tabBarItemHeight - accentThickness / 2;
			Rectangle accentRect{ Point{activeXStart, accentLineY}, Size{static_cast<uint32_t>(activeWidth), static_cast<uint32_t>(accentThickness)} };
			graphics.FillRectangle(accentRect, appearance->AccentColor);
			
			int yBase = tabBarItemHeight + 1;
			graphics.DrawLine({ 0, yBase }, { activeXStart - 1, yBase }, appearance->BoxBorderColor);
			graphics.DrawLine({ activeXStart + activeWidth, yBase }, { clientWidth, yBase }, appearance->BoxBorderColor);
			
			++yBase;
			graphics.DrawLine({ 0, yBase }, { activeXStart - 1, yBase }, appearance->InnerHighlightColor);
			graphics.DrawLine({ activeXStart + activeWidth, yBase }, { clientWidth, yBase }, appearance->InnerHighlightColor);
			
			Rectangle contentAreaRect{ Point{0, tabBarItemHeight + 1}, Size{static_cast<uint32_t>(clientWidth), static_cast<uint32_t>(clientHeight - 1 - tabBarItemHeight)}  };
			graphics.DrawBottomRoundedRectangle(contentAreaRect, cornerRadius, appearance->BoxBorderColor, false);
			
			contentAreaRect.X = 1;
			contentAreaRect.Y++;
			contentAreaRect.Width -= 2;
			contentAreaRect.Height -= 2;
			graphics.DrawBottomRoundedRectangle(contentAreaRect, cornerRadius, appearance->InnerHighlightColor, false);
		}
		else
		{
			int accentLineY = clientHeight - tabBarItemHeight - 1 - accentThickness / 2;
			Rectangle accentRect{ Point{activeXStart, accentLineY}, Size{static_cast<uint32_t>(activeWidth), static_cast<uint32_t>(accentThickness)} };
			graphics.FillRectangle(accentRect, appearance->AccentColor);
			
			int yBase = clientHeight - tabBarItemHeight - 2;
			graphics.DrawLine({ 0, yBase }, { activeXStart - 1, yBase }, appearance->BoxBorderColor);
			graphics.DrawLine({ activeXStart + activeWidth, yBase }, { clientWidth, yBase }, appearance->BoxBorderColor);
			
			--yBase;
			graphics.DrawLine({ 0, yBase }, { activeXStart - 1, yBase }, appearance->InnerHighlightColor);
			graphics.DrawLine({ activeXStart + activeWidth, yBase }, { clientWidth, yBase }, appearance->InnerHighlightColor);
			
			Rectangle contentAreaRect{ Point{0, 0}, Size{static_cast<uint32_t>(clientWidth), static_cast<uint32_t>(clientHeight - 1 - tabBarItemHeight)}  };
			graphics.DrawTopRoundedRectangle(contentAreaRect, cornerRadius, appearance->BoxBorderColor, false);
			
			contentAreaRect.X = contentAreaRect.Y = 1;
			contentAreaRect.Width -= 2;
			contentAreaRect.Height -= 2;
			graphics.DrawTopRoundedRectangle(contentAreaRect, cornerRadius, appearance->InnerHighlightColor, false);
		}
	}

	void TabBarReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_hoveredTabIndex.has_value() || m_module.m_hoveredCloseBtnIndex.has_value())
		{
			m_module.m_hoveredTabIndex.reset();
			m_module.m_hoveredCloseBtnIndex.reset();
			
			m_module.m_mouseDownCloseBtnIndex.reset(); 

			GUI::MarkAsNeedUpdate(m_module.m_owner);
		}
	}

	void TabBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		m_module.m_mouseDownCloseBtnIndex.reset();
		if (m_module.m_hoveredCloseBtnIndex.has_value())
		{
			m_module.m_mouseDownCloseBtnIndex = m_module.m_hoveredCloseBtnIndex;
			return;
		}
		
		if (m_module.m_hoveredTabIndex.has_value())
		{
			size_t selectedIndex = m_module.m_hoveredTabIndex.value();
			
			if (!m_module.m_selectedTabIndex || m_module.m_selectedTabIndex.value() != selectedIndex)
			{
				auto& selectedTabItem = m_module.m_panels[*m_module.m_selectedTabIndex];
				GUI::ShowWindow(selectedTabItem.PanelPtr.get(), false);
		
				m_module.m_selectedTabIndex = selectedIndex;
		
				GUI::ShowWindow(m_module.m_panels[selectedIndex].PanelPtr.get(), true);

				ArgTabBar argsTabBar{ selectedIndex, m_module.m_panels[selectedIndex].Id };
				m_module.m_events->TabChanged.Emit(argsTabBar);

				GUI::MarkAsNeedUpdate(m_module.m_owner);
			}
		}
	}

	void TabBarReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto hoveredTabIndex = m_module.FindItem(args.Position);
		std::optional<size_t> newHoveredTab;
		std::optional<size_t> newHoveredCloseBtn;
		
		if (m_module.m_showCloseButton && hoveredTabIndex.has_value())
		{
			newHoveredTab = hoveredTabIndex;
			
			auto& tabItem = m_module.m_panels[*hoveredTabIndex];
			Rectangle absCloseBtn = tabItem.CloseButtonArea;
			absCloseBtn.X += tabItem.Position.X;
			absCloseBtn.Y += tabItem.Position.Y;
			if (absCloseBtn.Contains(args.Position))
			{
				newHoveredCloseBtn = newHoveredTab;
			}
		}
		
		if (m_module.m_hoveredTabIndex != newHoveredTab || m_module.m_hoveredCloseBtnIndex != newHoveredCloseBtn)
		{
			m_module.m_hoveredTabIndex = newHoveredTab;
			m_module.m_hoveredCloseBtnIndex = newHoveredCloseBtn;
			
			GUI::MarkAsNeedUpdate(m_module.m_owner);
		}
		
		if (hoveredTabIndex.has_value())
		{
			auto& newSelectedTabItem = m_module.m_panels[*hoveredTabIndex];
			ArgTabMouse argsTabMouse{{hoveredTabIndex.value(), newSelectedTabItem.Id}, args };
			m_module.m_events->TabMouseMove.Emit(argsTabMouse);
		}
	}

	void TabBarReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_hoveredCloseBtnIndex.has_value() && 
			m_module.m_mouseDownCloseBtnIndex.has_value() && 
			m_module.m_hoveredCloseBtnIndex.value() == m_module.m_mouseDownCloseBtnIndex.value())
		{
			size_t tabToClose = m_module.m_hoveredCloseBtnIndex.value();
			
			m_module.EraseTab(tabToClose);
			
			m_module.m_hoveredTabIndex.reset();
			m_module.m_hoveredCloseBtnIndex.reset();
			m_module.m_mouseDownCloseBtnIndex.reset();
			
			GUI::MarkAsNeedUpdate(m_module.m_owner);
			return;
		}

		m_module.m_mouseDownCloseBtnIndex.reset();

		if (m_module.m_hoveredTabIndex.has_value())
		{
			ArgTabMouse tabMouseArgs{{m_module.m_hoveredTabIndex.value(), m_module.m_panels[m_module.m_hoveredTabIndex.value()].Id},{args.Position}};
			m_module.m_events->TabMouseUp.Emit(tabMouseArgs);
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
	
	bool TabBarReactor::Module::AddTab(std::string tabId, Window* window)
	{
		auto startIndex = m_panels.size();
		auto& newItem = m_panels.emplace_back();
		newItem.Id = std::move(tabId);
		newItem.PanelPtr.reset(window);

		GUI::SetParentWindow(window, m_owner);
		MoveTabPage(window);

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

		GUI::SetParentWindow(window, m_owner);
		
		PanelItem newItem;
		newItem.Id = std::move(tabId);
		newItem.PanelPtr.reset(window);

		m_panels.insert(m_panels.begin() + index, std::move(newItem));
		MoveTabPage(window);

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

		BuildItems(index);
		return true;
	}

	void TabBarReactor::Module::BuildItems(size_t startIndex)
	{
		if (startIndex >= m_panels.size())
		{
			return;
		}
		auto appearance = reinterpret_cast<TabBarAppearance*>(m_owner->Appearance.get());
	    
		int tabBarItemHeight = m_owner->ToScale(static_cast<int>(appearance->TabBarItemHeight));
		int tabPadding = m_owner->ToScale(10);
		int closeBtnSize = m_owner->ToScale(12);
		int spacing = m_owner->ToScale(6);
		int iconSize = static_cast<int>(m_owner->ToScale(appearance->SmallIconSize));

		auto& graphics = m_owner->Renderer.GetGraphics();

		int currentX = 1;
		if (startIndex > 0)
		{
			const auto& prevTab = m_panels[startIndex - 1];
			currentX = prevTab.Position.X + static_cast<int>(prevTab.Size.Width);
		}

		int clientWidth = static_cast<int>(m_owner->ClientSize.Width);
		int clientHeight = static_cast<int>(m_owner->ClientSize.Height);
		
		int tabY = (m_tabRowPosition == TabRowPosition::Top) ? 0 : (clientHeight - tabBarItemHeight - 1);
		
		Rectangle contentArea = GetTabPageArea(true);
		for (size_t i = startIndex; i < m_panels.size(); ++i)
		{
			auto& tab = m_panels[i];
			auto textSize = graphics.GetTextExtent(tab.Id);

			tab.Size.Width = textSize.Width + (tabPadding * 2) + spacing;
			if (tab.Icon)
			{
				tab.Size.Width += iconSize;
			}
			
			if (m_showCloseButton)
			{
				tab.Size.Width += closeBtnSize;
				
				tab.CloseButtonArea = {
					static_cast<int>(tab.Size.Width) - tabPadding - closeBtnSize,
					(tabBarItemHeight - closeBtnSize) / 2 + 1,
					static_cast<uint32_t>(closeBtnSize),
					static_cast<uint32_t>(closeBtnSize)
				};
			}
			tab.Size.Height = tabBarItemHeight;

			tab.Position.X = currentX;
			tab.Position.Y = tabY;

			tab.Center.X = tabPadding;
			if (tab.Icon)
			{
				tab.Center.X += iconSize;
			}
			tab.Center.Y = static_cast<int>(tabBarItemHeight - textSize.Height) / 2;

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
		
		std::string idCopy{ m_panels[index].Id };
		bool removeSelectedIndex = (m_selectedTabIndex && m_selectedTabIndex.value() == index);
		
		m_panels.erase(m_panels.begin() + index);
		if (m_selectedTabIndex.has_value())
		{
			size_t currentSelected = m_selectedTabIndex.value();

			if (removeSelectedIndex)
			{
				if (m_panels.empty())
				{
					m_selectedTabIndex.reset();
				}
				else if (currentSelected >= m_panels.size())
				{
					m_selectedTabIndex = m_panels.size() - 1;
				}
			}
			else if (index < currentSelected)
			{
				m_selectedTabIndex = currentSelected - 1;
			}
		}

		if (removeSelectedIndex && m_selectedTabIndex.has_value())
		{
			size_t newIdx = m_selectedTabIndex.value();
			ArgTabBar argsTabBar{ newIdx, m_panels[newIdx].Id };
			m_events->TabChanged.Emit(argsTabBar);
			
			GUI::ShowWindow(m_panels[newIdx].PanelPtr.get(), true);
		}
		
		ArgTabBar closedArgs{ index, idCopy };
		m_events->TabClosed.Emit(closedArgs);
		
		BuildItems(index);
		return true;
	}

	Window* TabBarReactor::Module::DetachTab(size_t index)
	{
		if (index >= m_panels.size())
		{
			return nullptr;
		}
		
		Window* detachedWindow = m_panels[index].PanelPtr.release();
		EraseTab(index);

		return detachedWindow;
	}

	void TabBarReactor::Module::Draw()
	{
		GUI::UpdateWindow(m_owner);
	}

	Rectangle TabBarReactor::Module::GetTabPageArea(bool includePadding) const
	{
		auto appearance = reinterpret_cast<TabBarAppearance*>(m_owner->Appearance.get());
		int tabBarItemHeight = static_cast<int>(m_owner->ToScale(appearance->TabBarItemHeight));
		
		int clientWidth = static_cast<int>(m_owner->ClientSize.Width);
		int clientHeight = static_cast<int>(m_owner->ClientSize.Height);
		
		Rectangle contentArea;
		if (m_tabRowPosition == TabRowPosition::Top)
		{
			contentArea = { 2, tabBarItemHeight + 3, static_cast<uint32_t>(clientWidth - 4), static_cast<uint32_t>(clientHeight - tabBarItemHeight - 4 - 2) };
		}
		else
		{
			contentArea = { 2, 3, static_cast<uint32_t>(clientWidth - 4), static_cast<uint32_t>(clientHeight - tabBarItemHeight - 4 - 2) };
		}
		
		if (includePadding)
		{
			contentArea.X += m_tabPagePadding.Left;
			contentArea.Y += m_tabPagePadding.Top;
		
			contentArea.Width -= m_tabPagePadding.Right;
			contentArea.Height -= m_tabPagePadding.Bottom;
		}
		
		return contentArea;
	}

	void TabBarReactor::Module::MoveTabPage(Window* window) const
	{
		Rectangle contentArea = GetTabPageArea(true);
		GUI::MoveWindow(window, contentArea);
	}
	
	std::optional<size_t> TabBarReactor::Module::FindItem(const Point& position) const
	{
		for (size_t i = 0; i < m_panels.size(); ++i)
		{
			if (Rectangle{ m_panels[i].Position, m_panels[i].Size }.Contains(position))
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

	void TabBarItem::SetTitle(const std::string& title)
	{
		m_module->m_panels[m_logicalIndex].Id = title;
		
		m_module->BuildItems();
		m_module->Draw();
	}

	void TabBarItem::SetIcon(const Image& image)
	{
		m_module->m_panels[m_logicalIndex].Icon = image;
		
		m_module->BuildItems();
		m_module->Draw();
	}

	TabBar::TabBar(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "TabBar";
#endif
	}

	TabBarItem TabBar::At(size_t index)
	{
		auto& module = GetReactor().GetModule();
		if (index >= module.m_panels.size())
		{
			return {};
		}
		return { index, &module };
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

	std::optional<size_t> TabBar::GetSelectedIndex() const
	{
		return GetReactor().GetModule().GetSelectedIndex();
	}
	
	void TabBar::Insert(size_t position, std::string tabId, Window* window)
	{
		auto& module = GetReactor().GetModule();
		module.InsertTab(position, std::move(tabId), window);
		module.Draw();
	}

	void TabBar::PushBack(std::string tabId, Window* window)
	{
		auto& module = GetReactor().GetModule();
		module.AddTab(std::move(tabId), window);
		module.Draw();
	}

	Window* TabBar::Detach(size_t index)
	{
		auto& module = GetReactor().GetModule();
		auto detachedWindow = module.DetachTab(index);
		module.Draw();
		
		return detachedWindow;
	}

	TabRowPosition TabBar::GetTabRowPosition() const
	{
		return GetReactor().GetModule().m_tabRowPosition;
	}

	void TabBar::SetTabRowPosition(TabRowPosition position)
	{
		auto& module = GetReactor().GetModule();
		if (module.m_tabRowPosition == position)
		{
			return;
		}
		
		module.m_tabRowPosition = position;
		module.BuildItems();
		
		for (auto& tabItem : module.m_panels)
		{
			GUI::MoveWindow(tabItem.PanelPtr.get(), tabItem.ContentArea);
		}
		module.Draw();
	}

	void TabBar::SetTabPagePadding(Padding padding)
	{
		auto& module = GetReactor().GetModule();
		module.m_tabPagePadding = padding;
		
		module.BuildItems();
		module.Draw();
	}

	void TabBar::ShowCloseButton(bool show)
	{
		auto& module = GetReactor().GetModule();
		if (module.m_showCloseButton == show)
			return;
		
		module.m_showCloseButton = show;
		module.BuildItems();
		module.Draw();
	}
}
