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
	}

	void TabBarReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		auto enabled = m_control->GetEnabled();
		graphics.DrawRectangle(window->Appereance->Background, true);

		if (m_module.Panels.empty())
		{
			graphics.DrawRectangle(window->Appereance->BoxBorderColor, false);
			return;
		}

		auto tabBarItemHeight = static_cast<int>(window->Appereance->TabBarItemHeight * window->DPIScaleFactor);
		auto tabPadding = static_cast<uint32_t>(10 * window->DPIScaleFactor);
		
		int startXPosition = 0;
		int lastPositionX = 0;
		for (size_t i = 0; i < m_module.Panels.size(); i++)
		{
			auto& tabItem = m_module.Panels[i];
			if (m_module.SelectedTabIndex == (int)i)
			{
				if (i == 0)
				{
					auto idTextSize = graphics.GetTextExtent(tabItem.Id);
					auto tabSize = idTextSize;
					tabSize.Width += tabPadding;
					tabSize.Height = tabBarItemHeight;

					auto center = tabSize - idTextSize;
					center = center * 0.5f;
					graphics.DrawLine({ 0, 1 }, { 0, tabBarItemHeight }, window->Appereance->BoxBorderColor);
					graphics.DrawLine({ 1, 0 }, { (int)tabSize.Width - 1, 0 }, window->Appereance->BoxBorderColor);
					graphics.DrawLine({ (int)tabSize.Width - 1, 1 }, { (int)tabSize.Width - 1, tabBarItemHeight }, window->Appereance->BoxBorderColor);
					graphics.DrawString({ (int)center.Width,(int)center.Height }, tabItem.Id, enabled ? window->Appereance->Foreground : window->Appereance->BoxBorderDisabledColor);

					lastPositionX += (int)tabSize.Width;
				}
			}
		}
		//TODO: drawlineto
		graphics.DrawLine({ 0, tabBarItemHeight }, { 0, (int)window->Size.Height }, window->Appereance->BoxBorderColor);
		graphics.DrawLine({ 0, (int)window->Size.Height-1 }, { (int)window->Size.Width, (int)window->Size.Height-1 }, window->Appereance->BoxBorderColor);
		graphics.DrawLine({ (int)window->Size.Width - 1,  tabBarItemHeight + 1 }, { (int)window->Size.Width - 1, (int)window->Size.Height }, window->Appereance->BoxBorderColor);

		if (lastPositionX < (int)window->Size.Width)
			graphics.DrawLine({ lastPositionX,tabBarItemHeight }, { (int)window->Size.Width, tabBarItemHeight }, window->Appereance->BoxBorderColor);
	}

	void TabBarReactor::AddTab(const std::string& tabId, Panel* panel)
	{
		m_module.AddTab(tabId, panel);
	}

	TabBar::TabBar(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}

	ControlBase* TabBar::PushBackTab(const std::string& tabId, std::function<std::unique_ptr<ControlBase>(Window*)> factory)
	{
		auto newTab = factory(*this);
		auto result = newTab.get();
		//m_reactor.AddTab(tabId, result);

		return result;
	}

	void TabBarReactor::Module::AddTab(const std::string& tabId, Panel* panel)
	{
		Panels.emplace_back(PanelWithId{ panel , tabId });
		if (SelectedTabIndex == -1)
			SelectedTabIndex = 0;
	}
}
