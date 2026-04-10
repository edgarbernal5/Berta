/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TAB_BAR_HEADER
#define BT_TAB_BAR_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Paint/Image.h"

#include <string>
#include <vector>
#include <optional>

namespace Berta
{
	enum class TabRowPosition
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
		Color InnerHighlightColor{255, 255, 255, 128};
		Color TabBackgroundColor{204, 200, 192, 255};
	};
	
	struct TabWindowDeleter
	{
		void operator()(Window* window) const;
	};
	
	class TabBarReactor : public ControlReactor
	{
	public:
		void Update(Graphics& graphics) override;

		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;
		
		struct PanelItem
		{
			PanelItem() = default;
			
			std::string Id;
			std::unique_ptr<Window, TabWindowDeleter> PanelPtr;
			Image Icon;
			
			Point Position{};
			Point Center{};
			Size Size{};
			Rectangle ContentArea{};
			Rectangle CloseButtonArea{};
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
			
			Rectangle GetTabPageArea(bool includePadding) const;
			
			void MoveTabPage(Window* window) const;
			
			std::optional<size_t> FindItem(const Point& position) const;
			std::optional<size_t> GetSelectedIndex() const;

			std::vector<PanelItem> m_panels;
			std::optional<size_t> m_selectedTabIndex{ std::nullopt };
			
			Window* m_owner{ nullptr };
			TabBarEvents* m_events{ nullptr };
			TabRowPosition m_tabRowPosition{ TabRowPosition::Top };
			Padding m_tabPagePadding;
			bool m_showCloseButton { true };
			std::optional<size_t> m_hoveredTabIndex { std::nullopt };
			std::optional<size_t> m_hoveredCloseBtnIndex { std::nullopt };
			std::optional<size_t> m_mouseDownCloseBtnIndex { std::nullopt };
		};
		
		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }
		
	protected:
		void DoOnInit() override;
		
	private:
		Module m_module;
	};
	
	struct TabBarItem
	{
		TabBarItem() = default;
		TabBarItem(size_t logicalIndex, TabBarReactor::Module* module) :
			m_logicalIndex(logicalIndex), m_module(module)
		{
		}
		
		void SetTitle(const std::string& title);
		void SetIcon(const Image& image);

		explicit operator bool() const
		{
			return m_module;
		}

	private:
		size_t m_logicalIndex{ static_cast<size_t>(-1) };
		TabBarReactor::Module* m_module{ nullptr };
	};

	class TabBar : public Control<Category::ControlTag, TabBarReactor, TabBarEvents, TabBarAppearance>
	{
	public:
		TabBar() = default;
		TabBar(Window* parent, const Rectangle& rectangle);

		TabBarItem At(size_t index);
		void Clear();
		size_t Count() const;
		void Erase(size_t index);
		std::optional<size_t> GetSelectedIndex() const;
		
		void Insert(size_t position, std::string tabId, Window* window);
		void PushBack(std::string tabId, Window* window);
		
		Window* Detach(size_t index);
		
		TabRowPosition GetTabRowPosition() const;
		void SetTabRowPosition(TabRowPosition position);
		void SetTabPagePadding(Padding padding);
		
		void ShowCloseButton(bool show);
	private:
		
	};
}

#endif
