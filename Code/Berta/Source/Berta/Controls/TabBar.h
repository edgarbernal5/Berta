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
		size_t Index;
		std::string_view Id;
	};
	
	struct ArgTabClosing
	{
		size_t Index;
		std::string_view Id;
		mutable bool Cancel{ false };
	};
	
	struct ArgTabMouse : public ArgTabBar
	{
		ArgMouse Mouse;
	};
	
	struct TabBarEvents : public ControlEvents
	{
		Event<ArgTabBar> TabChanged;
		Event<ArgTabClosing> TabClosing;
		Event<ArgTabBar> TabClosed;
		
		Event<ArgTabMouse> TabMouseDown;
		Event<ArgTabMouse> TabMouseMove;
		Event<ArgTabMouse> TabMouseUp;
	};
	
	struct TabBarAppearance : public ControlAppearance
	{
		uint32_t TabBarItemHeight = 27;
		Color SelectedBackgroundColor { Colors::Light_ButtonBackground };
		Color AccentColor{ Colors::Light_SelectionHighlightColor };
	};
	
	struct WindowDeleter
	{
		void operator()(Window* window) const;
	};
	
	class TabBarReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control, Graphics* graphics) override;
		void Update(Graphics& graphics) override;

		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;
		
		struct PanelItem
		{
			PanelItem() = default;
			
			std::string Id;
			std::unique_ptr<Window, WindowDeleter> PanelPtr;
			
			Point Position{};
			Point Center{};
			Size Size{};
			Rectangle ContentArea{};
		};
		
		struct Module
		{
			bool AddTab(std::string tabId, Window* window);
			bool Clear();
			bool InsertTab(size_t index, std::string tabId, Window* window);
			void BuildItems(size_t startIndex = 0);
			bool EraseTab(size_t index);
			Window* DetachTab(size_t index);
			
			void Draw();
			
			std::optional<size_t> FindItem(const Point& position) const;
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
		
		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }
		
	private:
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
		std::optional<size_t> GetSelectedIndex() const;
		
		void Insert(size_t position, std::string tabId, Window* window);
		void PushBack(std::string tabId, Window* window);
		
		Window* Detach(size_t index);
		
		void SetTabBarPosition(TabBarPosition position);

	private:
		
	};
}

#endif
