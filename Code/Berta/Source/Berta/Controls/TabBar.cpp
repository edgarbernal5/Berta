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

		BT_CORE_TRACE << "TabBarReactor Init" << std::endl;
		m_module.m_owner->Events->Resize.Connect([this](const ArgResize& args)
			{
				auto tabBarSize = m_module.m_owner->Size;
				int i = 0;
				for (auto tabItem = m_module.Panels.cbegin(); tabItem != m_module.Panels.cend(); ++i, ++tabItem)
				{
					auto tabItemPanelPosition = tabItem->PanelPtr->GetPosition();
					tabItem->PanelPtr->SetSize({ tabBarSize.Width - tabItemPanelPosition.X, tabBarSize.Height - tabItemPanelPosition.Y });
				}
			});
	}

	void TabBarReactor::Update(Graphics& graphics)
	{
		auto enabled = m_control->GetEnabled();
		graphics.DrawRectangle(m_module.m_owner->Appereance->Background, true);

		if (m_module.Panels.empty() || m_module.SelectedTabIndex == -1)
		{
			graphics.DrawRectangle(m_module.m_owner->Appereance->BoxBorderColor, false);
			return;
		}

		auto tabBarItemHeight = m_module.m_owner->ToScale((int)m_module.m_owner->Appereance->TabBarItemHeight);
		auto tabPadding = m_module.m_owner->ToScale(10);
		auto tabMarginUnselected = m_module.m_owner->ToScale(4);
		auto one = m_module.m_owner->ToScale(1);

		//TODO: drawlineto
		int lastPositionX = 0;
		int selectedPositionX = 0;
		int i = 0;
		
		for (auto tabItem = m_module.Panels.cbegin(); tabItem != m_module.Panels.cend(); ++i, ++tabItem)
		{
			if (m_module.SelectedTabIndex == i)
			{
				/*if (lastPositionX == 0)
				{
					graphics.DrawLine({ lastPositionX, 1 }, { lastPositionX, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);
					graphics.DrawLine({ lastPositionX + 1, 0 }, { lastPositionX + (int)tabItem.Size.Width - 1, 0 }, m_module.m_owner->Appereance->BoxBorderColor);
					graphics.DrawLine({ lastPositionX + (int)tabItem.Size.Width - 1, 1 }, { lastPositionX + (int)tabItem.Size.Width - 1, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);
				}
				else*/
				{
					graphics.DrawLine({ lastPositionX, 1 }, { lastPositionX, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);
					graphics.DrawLine({ lastPositionX + 1, 0 }, { lastPositionX + (int)tabItem->Size.Width - 1, 0 }, m_module.m_owner->Appereance->BoxBorderColor);
					graphics.DrawLine({ lastPositionX + (int)tabItem->Size.Width - 1, 1 }, { lastPositionX + (int)tabItem->Size.Width - 1, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);
				}

				graphics.DrawString({ (int)tabItem->Center.Width + lastPositionX,(int)tabItem->Center.Height }, tabItem->Id, enabled ? m_module.m_owner->Appereance->Foreground : m_module.m_owner->Appereance->BoxBorderDisabledColor);
				selectedPositionX = lastPositionX;
			}
			else
			{
				//if (i == 0)
				{
					graphics.DrawLine({ lastPositionX, 1 + tabMarginUnselected }, { lastPositionX, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);
				}
				graphics.DrawLine({ lastPositionX + 1, tabMarginUnselected }, { lastPositionX + (int)tabItem->Size.Width - 1, tabMarginUnselected }, m_module.m_owner->Appereance->BoxBorderColor);
				graphics.DrawLine({ lastPositionX + (int)tabItem->Size.Width - 1, tabMarginUnselected + 1 }, { lastPositionX + (int)tabItem->Size.Width - 1, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);

				graphics.DrawString({ (int)tabItem->Center.Width + lastPositionX,(int)tabItem->Center.Height + one }, tabItem->Id, enabled ? m_module.m_owner->Appereance->Foreground : m_module.m_owner->Appereance->BoxBorderDisabledColor);
			}
			lastPositionX += (int)tabItem->Size.Width;
		}
		auto selectedTabItem = m_module.At(m_module.SelectedTabIndex);
		if (selectedPositionX > 0)
		{
			graphics.DrawLine({ 0, tabBarItemHeight }, { selectedPositionX, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);
		}
		graphics.DrawLine({ 0, tabBarItemHeight }, { 0, (int)m_module.m_owner->Size.Height }, m_module.m_owner->Appereance->BoxBorderColor);
		graphics.DrawLine({ 0, (int)m_module.m_owner->Size.Height-1 }, { (int)m_module.m_owner->Size.Width, (int)m_module.m_owner->Size.Height-1 }, m_module.m_owner->Appereance->BoxBorderColor);
		graphics.DrawLine({ (int)m_module.m_owner->Size.Width - 1,  tabBarItemHeight + 1 }, { (int)m_module.m_owner->Size.Width - 1, (int)m_module.m_owner->Size.Height }, m_module.m_owner->Appereance->BoxBorderColor);

		if (selectedPositionX + (int)selectedTabItem->Size.Width < (int)m_module.m_owner->Size.Width)
		{
			graphics.DrawLine({ selectedPositionX + (int)selectedTabItem->Size.Width,tabBarItemHeight }, { (int)m_module.m_owner->Size.Width, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);
		}
	}

	void TabBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		int newSelectedIndex = m_module.FindItem(args.Position);
		if (newSelectedIndex == -1)
			return;

		if (m_module.NewSelectedIndex(newSelectedIndex))
		{
			auto selectedTabItem = m_module.At(m_module.SelectedTabIndex);

			selectedTabItem->PanelPtr->Hide();
			m_module.SelectIndex(newSelectedIndex);

			auto newSelectedTabItem = m_module.At(newSelectedIndex);
			newSelectedTabItem->PanelPtr->Show();

			Update(graphics);
			GUI::UpdateDeferred(m_module.m_owner);
		}
	}

	void TabBarReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.BuildItems();
	}

	void TabBarReactor::AddTab(const std::string& tabId, Panel* panel)
	{
		m_module.AddTab(tabId, panel);
	}

	void TabBarReactor::Clear()
	{
		m_module.Clear();
	}

	void TabBarReactor::InsertTab(size_t position, const std::string& tabId, Panel* panel)
	{
		m_module.InsertTab(position, tabId, panel);
	}

	void TabBarReactor::EraseTab(size_t position)
	{
		m_module.EraseTab(position);
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

	ControlBase* TabBar::PushBackTab(const std::string& tabId, std::function<ControlBase*(Window*)> factory)
	{
		auto result = factory(*this);
		m_reactor.AddTab(tabId, reinterpret_cast<Panel*>(result));

		return result;
	}

	ControlBase* TabBar::InsertTab(size_t position, const std::string& tabId, std::function<ControlBase* (Window*)> factory)
	{
		auto result = factory(*this);
		m_reactor.InsertTab(position, tabId, reinterpret_cast<Panel*>(result));

		return result;
	}

	void TabBarReactor::Module::AddTab(const std::string& tabId, Panel* panel)
	{
		auto startIndex = Panels.size();
		auto& newItem = Panels.emplace_back();
		newItem.Id = tabId;
		newItem.PanelPtr.reset(panel);

		//TODO: maybe we need to move this after selected tab index is set.
		UpdatePanelMoveRect(panel);

		if (SelectedTabIndex == -1)
		{
			SelectedTabIndex = 0;
		}
		else
		{
			panel->Hide();
		}
		BuildItems(startIndex);

		GUI::UpdateTree(m_owner);
	}

	void TabBarReactor::Module::Clear()
	{
		SelectedTabIndex = -1;
		Panels.clear();
		GUI::UpdateTree(m_owner);
	}

	void TabBarReactor::Module::InsertTab(size_t index, const std::string& tabId, Panel* panel)
	{
		if (index >= Panels.size())
		{
			AddTab(tabId, panel);
			return;
		}
		int startIndex = static_cast<int>(index);

		auto newIt = Panels.emplace(At(index), std::move(tabId), std::move(panel));

		UpdatePanelMoveRect(panel);

		if (SelectedTabIndex == -1)
		{
			SelectedTabIndex = 0;
		}
		else if (SelectedTabIndex == index)
		{
			++newIt;
			newIt->PanelPtr->Hide();
		}

		BuildItems(startIndex);

		GUI::UpdateTree(m_owner);
	}

	void TabBarReactor::Module::BuildItems(size_t startIndex)
	{
		if (startIndex >= Panels.size())
		{
			return;
		}

		auto tabBarItemHeight = m_owner->ToScale(m_owner->Appereance->TabBarItemHeight);
		auto tabPadding = m_owner->ToScale(10u);

		Point offset{ 0, 0 };

		if (startIndex > 0)
		{
			auto element = At(startIndex - 1);
			offset.X = element->Position.X + (int)element->Size.Width;
		}

		auto current = At(startIndex);
		for (size_t i = startIndex; i < Panels.size(); ++i, ++current)
		{
			auto textSize = m_owner->Renderer.GetGraphics().GetTextExtent(current->Id);
			Size itemSize{ textSize.Width + tabPadding, tabBarItemHeight };

			auto center = itemSize - textSize;
			center = center * 0.5f;

			Point itemPos = offset;
			current->Position = itemPos;
			current->Size = itemSize;
			current->Center = center;

			offset.X += (int)itemSize.Width;
		}
	}

	void TabBarReactor::Module::EraseTab(size_t index)
	{
		if (index >= Panels.size())
		{
			return;
		}

		auto current = At(index);
		
		bool removeSelectedIndex = index == SelectedTabIndex;
		current = Panels.erase(current);
		if (SelectedTabIndex >= static_cast<int>(Panels.size()))
		{
			SelectedTabIndex = static_cast<int>(Panels.size()) - 1;
			--current;
		}
		if (removeSelectedIndex)
		{
			current->PanelPtr->Show();
		}

		BuildItems(index);
		GUI::UpdateTree(m_owner);
	}

	int TabBarReactor::Module::FindItem(const Point& position)
	{
		auto& items = Panels;
		int i = 0;
		for (auto current = Panels.cbegin(); current != Panels.cend(); ++i, ++current)
		{
			if (Rectangle{ current->Position, current->Size }.IsInside(position))
			{
				return i;
			}
		}
		return -1;
	}

	void TabBarReactor::Module::UpdatePanelMoveRect(Panel* panel)
	{
		auto tabBarItemHeight = m_owner->ToScale(m_owner->Appereance->TabBarItemHeight);
		Rectangle rect{ 2, (int)tabBarItemHeight + 2, m_owner->Size.Width - 4, m_owner->Size.Height - tabBarItemHeight - 4 };
		GUI::MoveWindow(panel->Handle(), rect);
	}

	void TabBar::Erase(size_t index)
	{
		m_reactor.EraseTab(index);
	}

	TabBarReactor::PanelItem::~PanelItem()
	{
		BT_CORE_DEBUG << ":~PanelItem() id=" << Id << "." <<  std::endl;
	}
}
