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
		using ClickCallback = std::function<void(MenuItem)>;
		using ToggleCallback = std::function<void(MenuItem, bool)>;
		using DestroyCallback = std::function<void()>;
		
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
			ToggleCallback onToggle;  // Callback que recibe el nuevo estado
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

		MenuItem Append(const std::string& text, ClickCallback onClick = {});
		MenuItem Append(const std::wstring& text, ClickCallback onClick = {});
		MenuItem AppendSeparator();
		MenuItem AppendSubMenu(const std::wstring& text, std::unique_ptr<Menu> subMenu);
		MenuItem AppendCheckbox(const std::wstring& text, bool initialState, ToggleCallback onToggle = {});
		
		const std::vector<MenuItemData>& GetItems() const { return m_items; }
		
		MenuItemData& GetItem(size_t index) 
		{ 
			// Usamos .at() para tener protección contra desbordamientos (Out of Bounds) en modo Debug
			return m_items.at(index); 
		}
		
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
		Window* m_parentWindow{ nullptr };
		Menu* m_parentMenu{ nullptr };
		DestroyCallback m_destroyCallback;
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
			
			void InitFromData(Menu& menuData);
			void InitTimer();
			void ExecuteHoveredItem();
			void OpenHoveredSubMenu(bool selectFirstItem);
			
			void MoveSelection(int step);
			
			void DrawCheckmark(Graphics& graphics, const Point& position, int size, Color color);
			
			void SetIgnoreFirstMouseUp(bool value) { m_ignoreFirstMouseUp = value; }
			
			Window* m_owner { nullptr };
			ControlBase* m_control { nullptr };
			
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
		
		class Reactor : public ControlReactor
		{
		public:
			void Init(ControlBase& control, Graphics* graphics) override;
			void Update(Graphics& graphics) override;

			void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
			void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
			void MouseDown(Graphics& graphics, const ArgMouse& args) override;
			void MouseMove(Graphics& graphics, const ArgMouse& args) override;
			void MouseUp(Graphics& graphics, const ArgMouse& args) override;

			void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;

			//void BuildItems();
			//void SetItems(std::vector<std::unique_ptr<Menu::MenuItemData>>& items);
			//void SetMenuOwner(Menu* menuOwner);
			
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
			
			
			//void OpenSubMenu(Menu* subMenu, Menu* parentMenu, int selectedIndex, bool ignoreFirstMouseUp = true);
			//int FindItem(const ArgMouse& args);
			//bool MouseMoveInternal(const ArgMouse& args);

			//Berta::MenuBox* m_menuBox{ nullptr };
			//Menu* m_menuOwner{ nullptr };
			
			Module m_module;
		};
	}

	class MenuBox : public Control<ReactorCore::MenuBox::Reactor, FormEvents, ReactorCore::MenuBox::Appearance>
	{
	public:
		using MenuItem = Berta::MenuItem;

	public:
		MenuBox(Window* parent, const Point& position);
		~MenuBox() override;

		//void Init(Menu* menuOwner, std::vector<std::unique_ptr<Menu::MenuItemData>>& items);
		void InitFromData(Menu& menuData);
		//void SetIgnoreFirstMouseUp(bool value);
	private:
#if BT_DEBUG
		static int g_globalId;
#endif
	};
}

#endif