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
#include <optional>

namespace Berta
{
	enum class TabBarPosition
	{
		Top,
		Bottom
	};

	struct ArgTabBar
	{
		std::string id;
	};

	struct TabBarEvents : public ControlEvents
	{
		Event<ArgTabBar> TabChanged;
	};
	struct TabBarAppearance : public ControlAppearance
	{
		uint32_t TabBarItemHeight = 27;
	};

	class TabBarReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control, Graphics* graphics) override;
		void Update(Graphics& graphics) override;

		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;

		void AddTab(const std::string& tabId, Window* window);
		void Clear();
		void InsertTab(size_t position, const std::string& tabId, Window* window);
		void EraseTab(size_t position);
		int GetSelectedIndex() const;
		size_t Count() const;

		void SetTabPosition(TabBarPosition position);
	private:
		struct PanelItem
		{
			PanelItem() = default;
			PanelItem(const std::string& id, Window* panel) : Id(id), PanelPtr(panel) {}
			~PanelItem();

			Point Position{};
			Point Center{};
			Size Size{};
			Rectangle PanelArea{};

			std::string Id;
			Window* PanelPtr{ nullptr };
		};

		struct Module
		{
			bool AddTab(const std::string& tabId, Window* window);
			bool Clear();
			bool InsertTab(size_t index, const std::string& tabId, Window* window);
			void BuildItems(size_t startIndex = 0);
			bool EraseTab(size_t index);
			int FindItem(const Point& position) const;
			bool NewSelectedIndex(int newIndex) const { return m_selectedTabIndex != newIndex; }
			void SelectIndex(int newIndex) { m_selectedTabIndex = newIndex; }
			std::optional<size_t> GetSelectedIndex() const;

			std::vector<PanelItem> m_panels;
			std::optional<size_t> m_selectedTabIndex{ std::nullopt };
			Window* m_owner{ nullptr };
			TabBarEvents* m_events{ nullptr };
			TabBarAppearance* m_appearance{ nullptr };
			TabBarPosition m_tabPosition{ TabBarPosition::Top };

		private:
			void UpdatePanelMoveRect(Window* window) const;
		};
		Module m_module;
	};

	class TabBar : public Control<TabBarReactor, TabBarEvents, TabBarAppearance>
	{
	public:
		TabBar() = default;
		TabBar(Window* parent, const Rectangle& rectangle);

		void Clear();
		
		size_t Count() const;
		void Erase(size_t index);
		int GetSelectedIndex() const;

		void Insert(size_t position, const std::string& tabId, Window* window);
		void PushBack(const std::string& tabId, Window* window);
		void SetTabBarPosition(TabBarPosition position);

	private:
		
	};
}

#endif
