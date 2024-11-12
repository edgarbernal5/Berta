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
#include <vector>

namespace Berta
{
	constexpr uint32_t ItemTextPadding = 2;
	constexpr uint32_t SeparatorHeight = 3;

	class MenuBox;

	class MenuItemReactor
	{
	public:
		virtual bool OnClickSubMenu(const ArgMouse& args) = 0;

		virtual void MoveToNextItem(bool upwards) = 0;
		virtual bool ExitSubMenu() = 0;
		virtual bool EnterSubMenu() = 0;
		virtual void Select() = 0;
		virtual void Quit() = 0;

		virtual MenuItemReactor* Next() const { return m_next; }
		virtual MenuItemReactor* Prev() const { return m_prev; }

		virtual void Prev(MenuItemReactor* prev) { m_prev = prev; }
		virtual void Clear()
		{
			m_prev = nullptr;
			m_next = nullptr;
		}

		virtual Window* Owner() const = 0;
		virtual bool IsMenuBar() const = 0;
	protected:
		MenuItemReactor* m_next{ nullptr };
		MenuItemReactor* m_prev{ nullptr };
	};

	struct MenuItem;

	struct Menu
	{
		friend class MenuBoxReactor;
		friend class MenuBarReactor;

		using ClickCallback = std::function<void(MenuItem&)>;
		using DestroyCallback = std::function<void()>;

		void Append(const std::wstring& text, ClickCallback onClick = {});
		void AppendSeparator();
		void ShowPopup(Window* owner, const ArgMouse& args);
		Menu* CreateSubMenu(std::size_t index);
		void SetImage(size_t index, const Image& image);
		void SetEnabled(size_t index, bool enabled);

		struct Item
		{
			Item() : m_isSpearator(true) {}
			Item(const std::wstring& _text, ClickCallback _onClick) : 
				m_text(_text), 
				m_isSpearator(false),
				m_onClick(_onClick){}

			std::wstring m_text;
			bool m_isSpearator{ false };
			bool m_isEnabled{ true };
			ClickCallback m_onClick;
			std::unique_ptr<Menu> m_subMenu;
			Image m_image;
		};

		std::vector<std::unique_ptr<Item>> m_items;
		MenuBox* m_menuBox{ nullptr };
		Window* m_parentWindow{ nullptr };
		Menu* m_parentMenu{ nullptr };
		DestroyCallback m_destroyCallback;

		MenuBox* GetMenuBox() const { return m_menuBox; }
		void CloseMenuBox();
	private:
		void ShowPopup(Window* owner, const Point& position, Menu* parentMenu = nullptr, bool ignoreFirstMouseUp = true, Rectangle menuBarItem = {});

		Size GetMenuBoxSize(Window* parent);
	};

	struct MenuItem
	{
		MenuItem(Menu::Item& target) : m_target(target) {}

		bool GetEnabled() const;
		void SetEnabled(bool isEnabled);
		void SetText(const std::wstring& text);
	private:
		Menu::Item& m_target;
	};

	class MenuBoxReactor : public ControlReactor, public MenuItemReactor
	{
	public:
		~MenuBoxReactor();

		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;

		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;

		bool OnClickSubMenu(const ArgMouse& args) override;
		Window* Owner() const override;

		void MoveToNextItem(bool upwards) override;
		bool ExitSubMenu() override;
		bool EnterSubMenu() override;
		void Select() override;
		void Quit() override;
		bool IsMenuBar() const override { return false; }

		Menu* GetMenuOwner() const { return m_menuOwner; }
		void SetItems(std::vector<std::unique_ptr<Menu::Item>>& items);
		void SetMenuOwner(Menu* menuOwner);
		void SetIgnoreFirstMouseUp(bool value) { m_ignoreFirstMouseUp = value; }
		void SetMenuBarItemRect(const Rectangle& rect) { m_menuBarItemRect = rect; }
		Size GetMenuBoxSize();

	private:
		struct MenuBoxItem
		{
			Point m_position;
			Size m_size;
		};
		enum SubMenuAction
		{
			None,
			Open,
			Close
		};
		void OpenSubMenu(Menu* subMenu, Menu* parentMenu, int selectedIndex, bool ignoreFirstMouseUp = true);
		int FindItem(const ArgMouse& args);
		bool MouseMoveInternal(const ArgMouse& args);
		MenuItemReactor* GetLastMenuItem() const;

		MenuBox* m_control{ nullptr };
		Menu* m_menuOwner{ nullptr };
		bool m_ignoreFirstMouseUp{ true };
		Rectangle m_menuBarItemRect{  };
		std::vector<std::unique_ptr<Menu::Item>>* m_items{ nullptr };
		std::vector<MenuBoxItem> m_itemSizePositions;
		Timer m_subMenuTimer;
		int m_selectedIndex{ -1 };
		int m_selectedSubMenuIndex{ -1 };
		int m_openedSubMenuIndex{ -1 };
		SubMenuAction m_subMenuAction{ SubMenuAction::None };
	};

	class MenuBox : public Control<MenuBoxReactor, FormEvents>
	{
	public:
		friend struct Menu;
		friend class MenuBoxReactor;
		friend class MenuBarReactor;

	public:
		MenuBox(Window* parent, const Rectangle& rectangle);
		~MenuBox();

		void Init(Menu* menuOwner, std::vector<std::unique_ptr<Menu::Item>>& items, const Rectangle& rect);
		void SetIgnoreFirstMouseUp(bool value);

		void Popup();
	private:
#if BT_DEBUG
		static int g_globalId;
#endif

		MenuItemReactor* GetItemReactor() const { return (MenuItemReactor*)(&m_reactor); }
		Size GetMenuBoxSize();
	};
}

#endif