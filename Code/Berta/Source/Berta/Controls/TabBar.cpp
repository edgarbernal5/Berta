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
	}

	void TabBarReactor::Update(Graphics& graphics)
	{
		auto enabled = m_control->GetEnabled();
		graphics.DrawRectangle(m_module.m_owner->Appereance->Background, true);

		if (m_module.Panels.empty())
		{
			graphics.DrawRectangle(m_module.m_owner->Appereance->BoxBorderColor, false);
			return;
		}

		auto tabBarItemHeight = static_cast<int>(m_module.m_owner->Appereance->TabBarItemHeight * m_module.m_owner->DPIScaleFactor);
		auto tabPadding = static_cast<uint32_t>(10 * m_module.m_owner->DPIScaleFactor);
		auto tabMarginUnselected = static_cast<int>(4 * m_module.m_owner->DPIScaleFactor);
		auto one = static_cast<int>(1 * m_module.m_owner->DPIScaleFactor);

		//TODO: drawlineto
		int lastPositionX = 0;
		int selectedPositionX = 0;
		for (size_t i = 0; i < m_module.Panels.size(); i++)
		{
			auto& tabItem = m_module.Panels[i];
			if (m_module.SelectedTabIndex == (int)i)
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
					graphics.DrawLine({ lastPositionX + 1, 0 }, { lastPositionX + (int)tabItem.Size.Width - 1, 0 }, m_module.m_owner->Appereance->BoxBorderColor);
					graphics.DrawLine({ lastPositionX + (int)tabItem.Size.Width - 1, 1 }, { lastPositionX + (int)tabItem.Size.Width - 1, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);
				}

				graphics.DrawString({ (int)tabItem.Center.Width + lastPositionX,(int)tabItem.Center.Height }, tabItem.Id, enabled ? m_module.m_owner->Appereance->Foreground : m_module.m_owner->Appereance->BoxBorderDisabledColor);
				selectedPositionX = lastPositionX;
			}
			else
			{
				//if (i == 0)
				{
					graphics.DrawLine({ lastPositionX, 1 + tabMarginUnselected }, { lastPositionX, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);
				}
				graphics.DrawLine({ lastPositionX + 1, tabMarginUnselected }, { lastPositionX + (int)tabItem.Size.Width - 1, tabMarginUnselected }, m_module.m_owner->Appereance->BoxBorderColor);
				graphics.DrawLine({ lastPositionX + (int)tabItem.Size.Width - 1, tabMarginUnselected + 1 }, { lastPositionX + (int)tabItem.Size.Width - 1, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);

				graphics.DrawString({ (int)tabItem.Center.Width + lastPositionX,(int)tabItem.Center.Height + one }, tabItem.Id, enabled ? m_module.m_owner->Appereance->Foreground : m_module.m_owner->Appereance->BoxBorderDisabledColor);
			}
			lastPositionX += (int)tabItem.Size.Width;
		}
		auto& selectedTabItem = m_module.Panels[m_module.SelectedTabIndex];
		if (selectedPositionX > 0)
		{
			graphics.DrawLine({ 0, tabBarItemHeight }, { selectedPositionX, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);
		}
		graphics.DrawLine({ 0, tabBarItemHeight }, { 0, (int)m_module.m_owner->Size.Height }, m_module.m_owner->Appereance->BoxBorderColor);
		graphics.DrawLine({ 0, (int)m_module.m_owner->Size.Height-1 }, { (int)m_module.m_owner->Size.Width, (int)m_module.m_owner->Size.Height-1 }, m_module.m_owner->Appereance->BoxBorderColor);
		graphics.DrawLine({ (int)m_module.m_owner->Size.Width - 1,  tabBarItemHeight + 1 }, { (int)m_module.m_owner->Size.Width - 1, (int)m_module.m_owner->Size.Height }, m_module.m_owner->Appereance->BoxBorderColor);

		if (selectedPositionX + (int)selectedTabItem.Size.Width < (int)m_module.m_owner->Size.Width)
		{
			graphics.DrawLine({ selectedPositionX + (int)selectedTabItem.Size.Width,tabBarItemHeight }, { (int)m_module.m_owner->Size.Width, tabBarItemHeight }, m_module.m_owner->Appereance->BoxBorderColor);
		}
	}

	void TabBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		int newSelectedIndex = m_module.FindItem(args.Position);
		if (newSelectedIndex == -1)
			return;

		if (m_module.NewSelectedIndex(newSelectedIndex))
		{
			m_module.Panels[m_module.SelectedTabIndex].PanelPtr->Hide();
			m_module.SelectIndex(newSelectedIndex);
			m_module.Panels[newSelectedIndex].PanelPtr->Show();

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

	void TabBarReactor::EraseTab(size_t index)
	{
		m_module.EraseTab(index);
	}

	TabBar::TabBar(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}

	ControlBase* TabBar::PushBackTab(const std::string& tabId, std::function<ControlBase*(Window*)> factory)
	{
		auto newTab = factory(*this);
		auto result = newTab;
		//m_reactor.AddTab(tabId, result);

		return result;
	}

	void TabBarReactor::Module::AddTab(const std::string& tabId, Panel* panel)
	{
		auto startIndex = Panels.size();
		auto& newItem = Panels.emplace_back();
		newItem.Id = tabId;
		newItem.PanelPtr.reset(panel);

		auto tabBarItemHeight = static_cast<uint32_t>(m_owner->Appereance->TabBarItemHeight * m_owner->DPIScaleFactor);
		Rectangle rect{ 2, (int)tabBarItemHeight + 2, m_owner->Size.Width - 4, m_owner->Size.Height - tabBarItemHeight - 4 };
		GUI::MoveWindow(panel->Handle(), rect);

		if (SelectedTabIndex == -1)
		{
			SelectedTabIndex = 0;
		}
		else
		{
			panel->Hide();
			GUI::UpdateTree(*panel);
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

		auto tabBarItemHeight = static_cast<uint32_t>(m_owner->Appereance->TabBarItemHeight * m_owner->DPIScaleFactor);
		auto tabPadding = static_cast<uint32_t>(10u * m_owner->DPIScaleFactor);

		Point offset{ 0, 0 };

		if (startIndex > 0)
		{
			offset.X = Panels[startIndex - 1].Position.X + (int)Panels[startIndex - 1].Size.Width;
		}

		for (size_t i = startIndex; i < Panels.size(); i++)
		{
			auto& itemData = Panels[i];
			auto textSize = m_owner->Renderer.GetGraphics().GetTextExtent(itemData.Id);
			Size itemSize{ textSize.Width + tabPadding, tabBarItemHeight };

			auto center = itemSize - textSize;
			center = center * 0.5f;

			Point itemPos = offset;
			itemData.Position = itemPos;
			itemData.Size = itemSize;
			itemData.Center = center;

			offset.X += (int)itemSize.Width;
		}
	}

	void TabBarReactor::Module::EraseTab(size_t index)
	{
		if (index >= Panels.size())
			return;

		auto& tab = Panels[index];
		
		Panels.erase(Panels.begin() + index);
		if (SelectedTabIndex >= Panels.size())
		{
			SelectedTabIndex = Panels.size() - 1;
		}
		if (index == SelectedTabIndex)
		{
			Panels[index].PanelPtr->Show();
		}

		BuildItems(index);
		GUI::UpdateTree(m_owner);
	}

	int TabBarReactor::Module::FindItem(const Point& position)
	{
		auto& items = Panels;

		for (size_t i = 0; i < items.size(); i++)
		{
			auto& itemData = (items[i]);

			if (Rectangle{ itemData.Position, itemData.Size }.IsInside(position))
			{
				return (int)i;
			}
		}
		return -1;
	}

	void TabBar::Erase(size_t index)
	{
		m_reactor.EraseTab(index);
	}
}
