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
		graphics.DrawRectangle(m_control->Handle()->Appereance->Background, true);

		if (m_module.Panels.empty())
		{
			graphics.DrawRectangle(m_control->Handle()->Appereance->BoxBorderColor, false);
			return;
		}


	}

	TabBar::TabBar(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}

	ControlBase* TabBar::PushBackTab(const std::string& tabId, std::function<std::unique_ptr<ControlBase>(Window*)> factory)
	{
		auto newTab = factory(*this);
		return newTab.release();
	}

}
