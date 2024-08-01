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

		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;

		void AddTab(const std::string& tabId, Panel* panel);
		void EraseTab(size_t index);

	private:
		struct PanelItem
		{
			PanelItem() = default;
			PanelItem(const std::string& id, Panel* panel) : Id(id), PanelPtr(panel) {}

			Point Position;
			Size Center;
			Size Size;
			std::string Id;
			std::shared_ptr<Panel> PanelPtr;
		};

		struct Module
		{
			void AddTab(const std::string& tabId, Panel* panel);
			void BuildItems(size_t startIndex = 0);
			void EraseTab(size_t index);
			int FindItem(const Point& position);
			bool NewSelectedIndex(int newIndex) { return SelectedTabIndex != newIndex; }
			void SelectIndex(int newIndex) { SelectedTabIndex = newIndex; }

			std::vector<PanelItem> Panels;
			int SelectedTabIndex{ -1 };
			Window* m_owner{ nullptr };
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
			static_assert(std::is_base_of<Panel, TPanel>::value, "TPanel must be derived from Panel");

			auto newPanel = reinterpret_cast<TPanel*>(PushBackTab(tabId, std::bind([](Window* parent, Args & ... tabArgs)
			{
				return new TPanel(parent, std::forward<Args>(tabArgs)...);
			}, std::placeholders::_1, args...)));

			m_reactor.AddTab(tabId, newPanel);
			return newPanel;
		}

		void Erase(size_t index);

	private:
		ControlBase* PushBackTab(const std::string& tabId, std::function<ControlBase*(Window*)> factory);
	};
}

#endif
