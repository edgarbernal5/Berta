/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "TabBar.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void TabBarReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_module.m_owner = control.Handle();
		m_module.m_events = reinterpret_cast<TabBarEvents*>(m_module.m_owner->Events.get());
		m_module.m_appearance = reinterpret_cast<TabBarAppearance*>(m_module.m_owner->Appearance.get());
	}

	void TabBarReactor::Update(Graphics& graphics)
	{
		auto enabled = m_control->GetEnabled();
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

		//TODO: drawlineto
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
		}
	}

	void TabBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		int newSelectedIndex = m_module.FindItem(args.Position);
		if (newSelectedIndex == -1)
		{
			return;
		}

		if (m_module.NewSelectedIndex(newSelectedIndex))
		{
			auto selectedTabItem = m_module.At(m_module.m_selectedTabIndex);

			selectedTabItem->PanelPtr->Hide();
			m_module.SelectIndex(newSelectedIndex);

			auto newSelectedTabItem = m_module.At(newSelectedIndex);
			newSelectedTabItem->PanelPtr->Show();

			ArgTabBar argsTabBar{ newSelectedTabItem->Id };
			m_module.m_events->TabChanged.Emit(argsTabBar);

			Update(graphics);
			GUI::MarkAsUpdated(m_module.m_owner);
		}
	}

	void TabBarReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.BuildItems();

		for (auto tabItem = m_module.m_panels.begin(); tabItem != m_module.m_panels.end(); ++tabItem)
		{
			tabItem->PanelPtr->SetArea(tabItem->PanelArea);
		}
	}

	void TabBarReactor::AddTab(const std::string& tabId, Panel* panel)
	{
		if (m_module.AddTab(tabId, panel))
		{
			GUI::UpdateWindow(*m_control);
		}
	}

	void TabBarReactor::Clear()
	{
		if (m_module.Clear())
		{
			GUI::UpdateWindow(*m_control);
		}
	}

	void TabBarReactor::InsertTab(size_t position, const std::string& tabId, Panel* panel)
	{
		if (m_module.InsertTab(position, tabId, panel))
		{
			GUI::UpdateWindow(*m_control);
		}
	}

	void TabBarReactor::EraseTab(size_t position)
	{
		if (m_module.EraseTab(position))
		{
			GUI::UpdateWindow(*m_control);
		}
	}

	int TabBarReactor::GetSelectedIndex() const
	{
		return m_module.GetSelectedIndex();
	}

	size_t TabBarReactor::Count() const
	{
		return m_module.m_panels.size();
	}

	void TabBarReactor::SetTabPosition(TabBarPosition position)
	{
		if (m_module.m_tabPosition == position)
			return;

		m_module.m_tabPosition = position;
		m_module.BuildItems();
		GUI::UpdateWindow(*m_control);
	}

	bool TabBarReactor::Module::Clear()
	{
		bool needUpdate = !m_panels.empty();
		m_selectedTabIndex = -1;
		for (auto it = m_panels.begin(); it != m_panels.end(); ++it)
		{
			GUI::DisposeWindow(*it->PanelPtr);
		}
		m_panels.clear();

		return needUpdate;
	}

	bool TabBarReactor::Module::InsertTab(size_t index, const std::string& tabId, Panel* panel)
	{
		if (index >= m_panels.size())
		{
			return AddTab(tabId, panel);
		}
		int startIndex = static_cast<int>(index);

		auto newIt = m_panels.emplace(At(index), std::move(tabId), std::move(panel));

		UpdatePanelMoveRect(panel);

		if (m_selectedTabIndex == -1)
		{
			m_selectedTabIndex = 0;
		}
		else if (m_selectedTabIndex == index)
		{
			++newIt;
			newIt->PanelPtr->Hide();
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

		auto tabBarItemHeight = m_owner->ToScale(m_appearance->TabBarItemHeight);
		auto tabPadding = m_owner->ToScale(10u);

		Point offset{ 0, 0 };
		Point tabPositionOffset{};
		if (m_tabPosition == TabBarPosition::Bottom)
		{
			tabPositionOffset.Y = m_owner->ClientSize.Height - tabBarItemHeight;
		}

		int newWidth = (std::max)(0, static_cast<int>(m_owner->ClientSize.Width) - 4);
		int newHeight = (std::max)(0, static_cast<int>(m_owner->ClientSize.Height) - static_cast<int>(tabBarItemHeight) - 4);
		Rectangle panelTabArea;
		if (m_tabPosition == TabBarPosition::Top)
		{
			panelTabArea = { 2, (int)tabBarItemHeight + 2, static_cast<uint32_t>(newWidth), static_cast<uint32_t>(newHeight) };
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

			Point center{ (int)itemSize.Width - (int)textSize.Width, (int)itemSize.Height - (int)textSize.Height };
			center >>= 1;

			Point itemPos = offset + tabPositionOffset;
			current->Position = itemPos;
			current->Size = itemSize;
			current->Center = center;

			current->PanelArea = panelTabArea;

			offset.X += static_cast<int>(itemSize.Width);
		}
	}

	bool TabBarReactor::Module::EraseTab(size_t index)
	{
		if (index >= m_panels.size())
		{
			return false;
		}

		auto current = At(index);

		auto panelPtr = current->PanelPtr;
		bool removeSelectedIndex = index == m_selectedTabIndex;
		current = m_panels.erase(current);
		if (m_selectedTabIndex >= static_cast<int>(m_panels.size()))
		{
			m_selectedTabIndex = static_cast<int>(m_panels.size()) - 1;
			if (m_selectedTabIndex >= 0)
				--current;
		}
		GUI::DisposeWindow(*panelPtr);
		if (removeSelectedIndex && m_selectedTabIndex >= 0)
		{
			ArgTabBar argsTabBar{ current->Id };
			m_events->TabChanged.Emit(argsTabBar);

			current->PanelPtr->Show();
		}

		BuildItems(index);
		return true;
	}

	int TabBarReactor::Module::FindItem(const Point& position) const
	{
		int i = 0;
		for (auto current = m_panels.cbegin(); current != m_panels.cend(); ++i, ++current)
		{
			if (Rectangle{ current->Position, current->Size }.IsInside(position))
			{
				return i;
			}
		}
		return -1;
	}

	int TabBarReactor::Module::GetSelectedIndex() const
	{
		if (m_panels.empty())
			return -1;

		return m_selectedTabIndex;
	}

	void TabBarReactor::Module::UpdatePanelMoveRect(Panel* panel) const
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
		GUI::MoveWindow(panel->Handle(), rect);
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
		m_reactor.Clear();
	}

	ControlBase* TabBar::PushBack2(const std::string& tabId, ControlBase* control)
	{
		auto controlAsPanel = dynamic_cast<Panel*>(control);
		Panel* result = controlAsPanel;
		if (controlAsPanel)
		{
			GUI::SetParentWindow(control->Handle(), this->Handle());
		}
		else
		{
			result = new Panel(this->Handle());
#if BT_DEBUG
			result->Handle()->Name = "Panel-" + tabId;
#endif
			GUI::SetParentWindow(control->Handle(), result->Handle());
		}

		m_reactor.AddTab(tabId, result);

		return result;
	}

	void TabBar::SetTabPosition(TabBarPosition position)
	{
		m_reactor.SetTabPosition(position);
	}

	ControlBase* TabBar::PushBackTab(const std::string& tabId, std::function<ControlBase*(Window*)> factory)
	{
		auto result = factory(*this);
#if BT_DEBUG
		result->Handle()->Name = "Panel-" + tabId;
#endif
		m_reactor.AddTab(tabId, reinterpret_cast<Panel*>(result));

		return result;
	}

	ControlBase* TabBar::InsertTab(size_t position, const std::string& tabId, std::function<ControlBase* (Window*)> factory)
	{
		auto result = factory(*this);
		m_reactor.InsertTab(position, tabId, reinterpret_cast<Panel*>(result));

		return result;
	}

	bool TabBarReactor::Module::AddTab(const std::string& tabId, Panel* panel)
	{
		auto startIndex = m_panels.size();
		auto& newItem = m_panels.emplace_back();
		newItem.Id = tabId;
		newItem.PanelPtr = panel;

		UpdatePanelMoveRect(panel);

		if (m_selectedTabIndex == -1)
		{
			m_selectedTabIndex = 0;
		}
		else
		{
			panel->Hide();
		}
		BuildItems(startIndex);

		return true;
	}

	
	size_t TabBar::Count() const
	{
		return m_reactor.Count();
	}

	void TabBar::Erase(size_t index)
	{
		m_reactor.EraseTab(index);
	}

	int TabBar::GetSelectedIndex() const
	{
		return m_reactor.GetSelectedIndex();
	}

	TabBarReactor::PanelItem::~PanelItem()
	{
		BT_CORE_DEBUG << ":~PanelItem() id=" << Id << "." <<  std::endl;
	}
}
