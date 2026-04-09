/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_MENU_HEADER
#define BT_MENU_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Core/Timer.h"
#include "Berta/Paint/Image.h"

#include <string>
#include <functional>
#include <optional>
#include <variant>
#include <vector>

namespace Berta
{
	constexpr uint32_t ItemTextPadding = 2;
	constexpr uint32_t SeparatorHeight = 3;

	class MenuBox;
	struct Menu;
	struct MenuItem;
	
	namespace Internal::MenuBox
	{
		class Reactor;
	}
	
	struct Menu
	{
		using ClickCallback = std::function<void(MenuItem)>;
		using ToggleCallback = std::function<void(MenuItem, bool)>;
		
		Menu() = default;
		~Menu() = default;
		
		Menu(const Menu&) = delete;
		Menu& operator=(const Menu&) = delete;
		Menu(Menu&&) noexcept = default;
		Menu& operator=(Menu&&) noexcept = default;
		
		struct MenuSeparator {};

		struct MenuAction
		{
			std::wstring text;
			std::wstring shortcutText;
			Image image;
			ClickCallback onClick;
			wchar_t accessKey{ 0 };
			std::size_t accessKeyPosition{ 0 };
			bool isEnabled{ true };
		};
		
		struct MenuCheckbox 
		{
			std::wstring text;
			bool isChecked{ false };
			ToggleCallback onToggle;
			bool isEnabled{ true };
		};

		struct MenuSubMenu
		{
			std::wstring text;
			Image image;
			std::unique_ptr<Menu> subMenu;
			bool isEnabled{ true };
		};
		
		using MenuItemData = std::variant<MenuSeparator, MenuAction, MenuSubMenu, MenuCheckbox>;

		MenuItem Append(const std::string& text, ClickCallback onClick = {});
		MenuItem Append(const std::wstring& text, ClickCallback onClick = {});
		MenuItem AppendSeparator();
		MenuItem AppendSubMenu(const std::wstring& text, std::unique_ptr<Menu> subMenu);
		MenuItem AppendCheckbox(const std::wstring& text, bool initialState, ToggleCallback onToggle = {});
		
		const std::vector<MenuItemData>& GetItems() const { return m_items; }
		
		MenuItemData& GetItem(size_t index) 
		{ 
			return m_items.at(index); 
		}
		
		void ShowPopup(Window* owner, const ArgMouse& args);
		
		void SetText(size_t index, const std::wstring& text);
		std::wstring GetText(size_t index) const;
		
		void SetImage(size_t index, const Image& image);
		
		bool GetEnabled(size_t index) const;
		void SetEnabled(size_t index, bool enabled);
		
		bool IsChecked(size_t index) const;
		void SetChecked(size_t index, bool checked);
		void ToggleCheckbox(size_t index);
		
	private:
		std::vector<MenuItemData> m_items;
	};
	
	struct MenuItem
	{
		MenuItem() = default;
		MenuItem(Menu* owner, size_t index) : m_owner(owner), m_index(index) {}
		
		bool IsValid() const { return m_owner != nullptr; }
		operator bool() const { return IsValid(); }
		
		//method chaining
		
		bool GetEnabled() const;
		MenuItem& SetEnabled(bool enabled);
		
		std::wstring GetText() const;
		MenuItem& SetText(const std::wstring& text);
		
		MenuItem& SetImage(const Image& image);
		
		MenuItem& SetChecked(bool checked);
		bool IsChecked() const;
		MenuItem& Toggle();
		
	private:
		Menu* m_owner{ nullptr };
		size_t m_index{ 0 };
	};
	
	namespace Internal::MenuBox
	{		
		struct Appearance : public ControlAppearance
		{
			uint32_t ItemTextPadding = 4;
			uint32_t SeparatorHeight = 3;
        
			uint32_t MenuBoxLeftPaneWidth = 32;
			uint32_t MenuBoxItemHeight = 22;
			uint32_t MenuBoxSubMenuArrowWidth = 20;
			uint32_t MenuBoxShortcutWidth = 40;
        
			uint32_t CheckboxSize = 12;
		};
		
		struct ItemLayoutCache
		{
			Rectangle bounds;
			Point textPosition;
			Point shortcutPosition;
			Point arrowPosition;
		};
		
		struct Module
		{
			void CalculateLayout(const Menu& menuData);
			
			void InitFromData(Menu& menuData);
			void InitTimer();
			void ExecuteHoveredItem();
			void OpenHoveredSubMenu(bool focusFirstItem);
			bool IsHoveredItemSubMenu() const;
			void MoveSelection(int step);
			
			void DrawCheckmark(Graphics& graphics, const Point& position, int size, Color color);
			
			void SetIgnoreFirstMouseUp(bool value) { m_ignoreFirstMouseUp = value; }
			
			Window* m_owner { nullptr };
			ControlBase* m_control { nullptr };
			
			Menu* m_menuData{ nullptr };
			
			std::vector<ItemLayoutCache> m_layoutCache;
			Size m_calculatedBoxSize;
			
			std::optional<Point> m_lastMousePos { std::nullopt };
			bool m_ignoreFirstMouseUp{ false };
			std::optional<std::size_t> m_hoveredIndex;
			std::optional<std::size_t> m_pendingSubMenuIndex;
			std::optional<std::size_t> m_openedSubMenuIndex;
			Timer m_hoverTimer;
			static constexpr uint32_t SubMenuDelayMs = 400u;
		};
		
		class Reactor : public ControlReactor
		{
		public:
			void Update(Graphics& graphics) override;

			void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
			void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
			void MouseDown(Graphics& graphics, const ArgMouse& args) override;
			void MouseMove(Graphics& graphics, const ArgMouse& args) override;
			void MouseUp(Graphics& graphics, const ArgMouse& args) override;

			void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;

			Module& GetModule() { return m_module; }
			const Module& GetModule() const { return m_module; }
			
		protected:
			void DoOnInit() override;
			
		private:
			Module m_module;
		};
	}

	class MenuBox : public Control<Internal::MenuBox::Reactor, FormEvents, Internal::MenuBox::Appearance>
	{
	public:
		using MenuItem = Berta::MenuItem;

	public:
		MenuBox(Window* parent, const Point& position);
		~MenuBox() override;

		void InitFromData(Menu& menuData);
		
	private:
#if BT_DEBUG
		static int g_globalId;
#endif
	};
}

#endif