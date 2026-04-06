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
		using ClickCallback = std::function<void(MenuItem&)>;
		using DestroyCallback = std::function<void()>;
		
		Menu() = default;
		~Menu() = default;
		
		Menu(const Menu&) = delete;
		Menu& operator=(const Menu&) = delete;
		Menu(Menu&&) = default;
		Menu& operator=(Menu&&) = default;
		
		struct MenuSeparator {};

		struct MenuAction
		{
			std::wstring text;
			std::wstring shortcutText; //Ej. L"Ctrl+S"
			Image image;
			ClickCallback onClick;
			wchar_t accessKey{ 0 };
			std::size_t accessKeyPosition{ 0 };
			bool isEnabled{ true };
		};
		
		struct MenuCheckbox 
		{
			std::wstring text;
			bool isChecked{ false };             // Estado actual
			std::function<void(bool)> onToggle;  // Callback que recibe el nuevo estado
			bool isEnabled{ true };
		};

		struct MenuSubMenu
		{
			std::wstring text;
			Image image;
			std::unique_ptr<Menu> subMenu;
			bool isEnabled{ true };
		};
		
		/*struct Item
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
		};*/
		
		using MenuItemData = std::variant<MenuSeparator, MenuAction, MenuSubMenu, MenuCheckbox>;

		void Append(const std::string& text, ClickCallback onClick = {});
		void Append(const std::wstring& text, ClickCallback onClick = {});
		void AppendSeparator();
		void AppendSubMenu(const std::wstring& text, std::unique_ptr<Menu> subMenu);
		void AppendCheckbox(const std::wstring& text, bool initialState, std::function<void(bool)> onToggle = {});
		
		const std::vector<MenuItemData>& GetItems() const { return m_items; }
		
		MenuItemData& GetItem(size_t index) 
		{ 
			// Usamos .at() para tener protección contra desbordamientos (Out of Bounds) en modo Debug
			return m_items.at(index); 
		}
		
		void SetImage(size_t index, const Image& image);
		void SetEnabled(size_t index, bool enabled);
		void SetChecked(size_t index, bool checked);
		bool IsChecked(size_t index) const;
		void ToggleCheckbox(size_t index);
		
	private:
		Size GetMenuBoxSize(Window* parent) const;

		std::vector<MenuItemData> m_items;
		Window* m_parentWindow{ nullptr };
		Menu* m_parentMenu{ nullptr };
		DestroyCallback m_destroyCallback;
	};
	
	struct MenuItem
	{
		MenuItem(Menu::MenuItemData& target) : m_target(target) {}

		bool GetEnabled() const;
		void SetEnabled(bool isEnabled);
		void SetText(const std::wstring& text);
		
	private:
		Menu::MenuItemData& m_target;
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
		
		struct ItemLayoutCache
		{
			Rectangle bounds;       // Área total (para detectar clics y dibujar el fondo 'hover')
			Point textPosition;     // Origen del texto principal
			Point shortcutPosition; // Origen del texto del atajo de teclado (ej. Ctrl+C)
			Point arrowPosition;    // Origen de la flecha si es un submenú
		};
		
		struct Module
		{
			void CalculateLayout(const Menu& menuData);
			Size CalculateMenuBoxSize();
			
			void InitFromData(Menu& menuData);
			void InitTimer();
			void LoadItems(const std::vector<Menu::MenuItemData>& items);
			void ExecuteHoveredItem();
			void OpenHoveredSubMenu(bool selectFirstItem);
			
			void DrawCheckmark(Graphics& graphics, const Point& position, int size, Color color);
			
			void SetIgnoreFirstMouseUp(bool value) { m_ignoreFirstMouseUp = value; }
			Window* m_owner { nullptr };
			
			std::vector<Menu::MenuItemData> m_itemsData;
			
			Menu* m_menuData{ nullptr };
			std::vector<ItemLayoutCache> m_layoutCache;
			Size m_calculatedBoxSize;
			
			bool m_ignoreFirstMouseUp{ true };
			std::optional<std::size_t> m_hoveredIndex;
			std::optional<std::size_t> m_pendingSubMenuIndex;
			std::optional<std::size_t> m_openedSubMenuIndex;
			Timer m_hoverTimer; // El temporizador de la vista
			static constexpr float SubMenuDelayMs = 400.0f;
		};

		class MenuItemReactor
		{
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
			void SetItems(std::vector<std::unique_ptr<Menu::MenuItemData>>& items);
			void SetMenuOwner(Menu* menuOwner);
			
			//Size GetMenuBoxSize();

			Module& GetModule() { return m_module; }
			const Module& GetModule() const { return m_module; }
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
			
			Module m_module;
		};
	}

	class MenuBox : public Control<ReactorCore::MenuBox::Reactor, FormEvents, ReactorCore::MenuBox::Appearance>
	{
	public:
		using MenuItem = Berta::MenuItem;
		using MenuItemReactor = ReactorCore::MenuBox::MenuItemReactor;

	public:
		MenuBox(Window* parent, const Point& position);
		~MenuBox() override;

		//void Init(Menu* menuOwner, std::vector<std::unique_ptr<Menu::MenuItemData>>& items);
		void InitFromData(const Menu& menuData);
		//void SetIgnoreFirstMouseUp(bool value);
	private:
#if BT_DEBUG
		static int g_globalId;
#endif

		MenuItemReactor* GetItemReactor() { return &GetReactor(); }
	};
}

#endif