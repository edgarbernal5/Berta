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
#include <list>

namespace Berta
{
	enum class TabBarPosition
	{
		Top,
		Bottom
	};

	class TabBarReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;

		void AddTab(const std::string& tabId, Panel* panel);
		void Clear();
		void InsertTab(size_t position, const std::string& tabId, Panel* panel);
		void EraseTab(size_t position);

		void SetTabPosition(TabBarPosition position);
	private:
		struct PanelItem
		{
			PanelItem() = default;
			PanelItem(const std::string& id, Panel* panel) : Id(id), PanelPtr(panel) {}
			~PanelItem();

			Point Position{};
			Point Center{};
			Size Size{};
			Rectangle PanelArea{};

			std::string Id;
			std::shared_ptr<Panel> PanelPtr;
		};

		struct Module
		{
			using PanelIterator = std::list<PanelItem>::iterator;
			using ConstPanelIterator = std::list<PanelItem>::const_iterator;

			bool AddTab(const std::string& tabId, Panel* panel);
			bool Clear();
			bool InsertTab(size_t index, const std::string& tabId, Panel* panel);
			void BuildItems(size_t startIndex = 0);
			bool EraseTab(size_t index);
			int FindItem(const Point& position) const;
			bool NewSelectedIndex(int newIndex) const { return m_selectedTabIndex != newIndex; }
			void SelectIndex(int newIndex) { m_selectedTabIndex = newIndex; }

			PanelIterator At(std::size_t position)
			{
				auto it = m_panels.begin();
				std::advance(it, position);
				return it;
			}

			ConstPanelIterator At(std::size_t position) const
			{
				auto it = m_panels.cbegin();
				std::advance(it, position);
				return it;
			}

			std::list<PanelItem> m_panels;
			int m_selectedTabIndex{ -1 };
			Window* m_owner{ nullptr };
			TabBarPosition m_tabPosition{ TabBarPosition::Top };

		private:
			void UpdatePanelMoveRect(Panel* panel) const;
		};
		Module m_module;
	};

	class TabBar : public Control<TabBarReactor>
	{
	public:
		TabBar() = default;
		TabBar(Window* parent, const Rectangle& rectangle);

		void Clear();

		template<typename PanelType, typename ...Args>
		PanelType* Insert(size_t position, const std::string& tabId, Args&& ... args)
		{
			static_assert(std::is_base_of<Panel, PanelType>::value, "PanelType must be derived from Panel");

			auto newPanel = reinterpret_cast<PanelType*>(InsertTab(position, tabId, std::bind([](Window* parent, Args & ... tabArgs)
			{
				return new PanelType(parent, std::forward<Args>(tabArgs)...);
			}, std::placeholders::_1, args...)));

			return newPanel;
		}
		
		void Erase(size_t index);

		template<typename PanelType, typename ...Args>
		PanelType* PushBack(const std::string& tabId, Args&& ... args)
		{
			static_assert(std::is_base_of<Panel, PanelType>::value, "PanelType must be derived from Panel");

			auto newPanel = reinterpret_cast<PanelType*>(PushBackTab(tabId, std::bind([](Window* parent, Args & ... tabArgs)
			{
				return new PanelType(parent, std::forward<Args>(tabArgs)...);
			}, std::placeholders::_1, args...)));

			return newPanel;
		}

		ControlBase* PushBack2(const std::string& tabId, ControlBase* control);
		void SetTabPosition(TabBarPosition position);

	private:
		ControlBase* PushBackTab(const std::string& tabId, std::function<ControlBase*(Window*)> factory);
		ControlBase* InsertTab(size_t position, const std::string& tabId, std::function<ControlBase*(Window*)> factory);
	};
}

#endif
