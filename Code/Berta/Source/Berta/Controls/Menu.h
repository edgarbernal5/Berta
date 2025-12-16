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
	struct Menu;
	struct MenuItem;
	
	namespace ReactorCore::MenuBar
	{
		class Reactor;
	}
	
	namespace ReactorCore::MenuBox
	{
		class Reactor;
	}
	
	struct Menu
	{
		friend class Berta::MenuBox;
		friend class ReactorCore::MenuBox::Reactor;
		friend class ReactorCore::MenuBar::Reactor;

		using ClickCallback = std::function<void(MenuItem&)>;
		using DestroyCallback = std::function<void()>;

		void Append(const std::string& text, ClickCallback onClick = {});
		void Append(const std::wstring& text, ClickCallback onClick = {});
		void AppendSeparator();
		void ShowPopup(Window* owner, const ArgMouse& args);
		Menu* CreateSubMenu(std::size_t index);
		void SetImage(size_t index, const Image& image);
		void SetEnabled(size_t index, bool enabled);

		Berta::MenuBox* GetMenuBox() const { return m_menuBox; }
		void CloseMenuBox();

		struct Item
		{
			Item() : m_isSeparator(true) {}
			Item(const std::wstring& _text, ClickCallback _onClick) : 
				m_text(_text), 
				m_isSeparator(false),
				m_onClick(_onClick)
			{
				m_text = GUI::GetAccessKeyText(_text, m_accessKey, &m_accessKeyPosition);
			}

			std::wstring m_text;
			bool m_isSeparator{ false };
			bool m_isEnabled{ true };
			ClickCallback m_onClick;
			std::unique_ptr<Menu> m_subMenu;
			Image m_image;
			wchar_t m_accessKey{ 0 };
			std::size_t	m_accessKeyPosition{ 0 };
		};
	private:
		void ShowPopup(Window* owner, const Point& position, bool fromMenuBar, bool ignoreFirstMouseUp = true);
		Size GetMenuBoxSize(Window* parent);

		std::vector<std::unique_ptr<Item>> m_items;
		Berta::MenuBox* m_menuBox{ nullptr };
		Window* m_parentWindow{ nullptr };
		Menu* m_parentMenu{ nullptr };
		DestroyCallback m_destroyCallback;
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
	namespace ReactorCore::MenuBox
	{		
		struct Appearance : public ControlAppearance
		{
			uint32_t MenuBarItemHeight = 18;
			uint32_t MenuBoxLeftPaneWidth = 32;
			uint32_t MenuBoxItemHeight = 20;
			uint32_t MenuBoxSubMenuArrowWidth = 20;
			uint32_t MenuBoxShortcutWidth = 20;
		};

		class MenuItemReactor
		{
		public:
			friend class ReactorCore::MenuBox::Reactor;
		
		public:
			virtual ~MenuItemReactor() = default;

			virtual bool OnClickSubMenu(const ArgMouse& args) = 0;

			virtual void MoveToNextItem(bool upwards) = 0;
			virtual bool ExitSubMenu() = 0;
			virtual bool EnterSubMenu() = 0;
			virtual void Select() = 0;
			virtual void Quit() = 0;

			virtual MenuItemReactor* Prev() const { return m_prev; }
			virtual MenuItemReactor* Next() const { return m_next; }

			virtual Window* Owner() const = 0;

		protected:
			MenuItemReactor* m_next{ nullptr };
			MenuItemReactor* m_prev{ nullptr };
		};
		
		class Reactor : public ControlReactor, public MenuItemReactor
		{
		public:
			~Reactor() = default;

			void Init(ControlBase& control, Graphics* graphics) override;
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

			Menu* GetMenuOwner() const { return m_menuOwner; }

			void BuildItems();
			void SetItems(std::vector<std::unique_ptr<Menu::Item>>& items);
			void SetMenuOwner(Menu* menuOwner);
			void SetIgnoreFirstMouseUp(bool value) { m_ignoreFirstMouseUp = value; }
			Size GetMenuBoxSize();

		private:
			struct MenuBoxItem
			{
				Point m_position;
				Size m_size;
			};
			enum SubMenuAction : uint8_t
			{
				None,
				Open,
				Close
			};

			void OpenSubMenu(Menu* subMenu, Menu* parentMenu, int selectedIndex, bool ignoreFirstMouseUp = true);
			int FindItem(const ArgMouse& args);
			bool MouseMoveInternal(const ArgMouse& args);
			MenuItemReactor* GetLastMenuItem() const;

			Berta::MenuBox* m_menuBox{ nullptr };
			Menu* m_menuOwner{ nullptr };
			Appearance* m_appearance{ nullptr };
			bool m_ignoreFirstMouseUp{ true };
			std::vector<std::unique_ptr<Menu::Item>>* m_items{ nullptr };
			std::vector<MenuBoxItem> m_itemSizePositions;
			Timer m_subMenuTimer;
			int m_selectedIndex{ -1 };
			int m_selectedSubMenuIndex{ -1 };
			int m_openedSubMenuIndex{ -1 };
		};
	}

	class MenuBox : public Control<ReactorCore::MenuBox::Reactor, FormEvents, ReactorCore::MenuBox::Appearance>
	{
	public:
		using MenuItem = Berta::MenuItem;
		using MenuItemReactor = ReactorCore::MenuBox::MenuItemReactor;
		
		friend struct Menu;
		friend class ReactorCore::MenuBox::Reactor;
		friend class ReactorCore::MenuBar::Reactor;

	public:
		MenuBox(Window* parent, const Point& position);
		~MenuBox();

		void Init(Menu* menuOwner, std::vector<std::unique_ptr<Menu::Item>>& items);
		void SetIgnoreFirstMouseUp(bool value);

		void Popup(bool fromMenuBar = false);
	private:
#if BT_DEBUG
		static int g_globalId;
#endif

		MenuItemReactor* GetItemReactor() { return &GetReactor(); }
		Size GetMenuBoxSize();
	};
}

#endif