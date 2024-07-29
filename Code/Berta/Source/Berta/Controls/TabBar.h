/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TAB_BAR_HEADER
#define BT_TAB_BAR_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/Panel.h"

#include <string>
#include <vector>

namespace Berta
{
	class TabBarReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void AddTab(const std::string& tabId, Panel* panel);
	private:
		struct PanelWithId
		{
			Panel* PanelPtr;
			std::string Id;
		};
		struct Module
		{
			void AddTab(const std::string& tabId, Panel* panel);

			std::vector<PanelWithId> Panels;
			int SelectedTabIndex{ -1 };
		};
		ControlBase* m_control{ nullptr };
		Module m_module;
	};

	class TabBar : public Control<TabBarReactor>
	{
	public:
		TabBar(Window* parent, const Rectangle& rectangle);

		template<typename TPanel, typename ...Args>
		TPanel* PushBack(const std::string& tabId, Args&& ... args)
		{
			auto newPanel = reinterpret_cast<TPanel*>(PushBackTab(tabId, std::bind([](Window* parent, Args & ... tabArgs)
			{
				return std::unique_ptr<ControlBase>(new TPanel(parent, std::forward<Args>(tabArgs)...));
			}, std::placeholders::_1, args...)));

			m_reactor.AddTab(tabId, newPanel);
			return newPanel;
		}

	private:
		ControlBase* PushBackTab(const std::string& tabId, std::function<std::unique_ptr<ControlBase>(Window*)> factory);
	};
}

#endif
