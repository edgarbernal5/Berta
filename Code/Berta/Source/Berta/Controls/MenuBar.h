/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_MENU_BAR_HEADER
#define BT_MENU_BAR_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/Menu.h"
#include <string>
#include <vector>

namespace Berta
{
	class MenuBar;

	class MenuBarReactor : public ControlReactor, public MenuItemReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;

		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;

		void MoveToNextItem(bool upwards) override;
		bool EnterSubMenu() override { return false; };
		bool ExitSubMenu() override { return false; };
		void Select() override;
		void Quit() override;
		bool IsMenuBar() const override { return true; }

		bool OnClickSubMenu(const ArgMouse& args) override { return false; }

		Window* Owner() const override;

		struct MenuBarItemData
		{
			MenuBarItemData(const std::wstring& _text) :text(_text) {}

			Menu menu;
			std::wstring text;
			Size size;
			Point position;
			Size center;
			bool isEnabled{ true };
		};

		struct InteractionData
		{
			int		m_selectedItemIndex{ -1 };
			Menu*	m_activeMenu{ nullptr };
		};

		struct Module
		{
			void BuildItems(size_t startIndex = 0);
			int FindItem(const Point& position);
			Menu& PushBack(const std::wstring& text);
			void OpenMenu(bool ignoreFirstMouseUp = true);
			void SelectIndex(int index);
			bool IsMenuOpen() const { return m_interactionData.m_activeMenu; }
			MenuBox* GetActiveMenuBox() const;

			MenuBar* m_control{ nullptr };
			Window* m_owner{ nullptr };
			std::vector<MenuBarItemData*> m_items;
			InteractionData m_interactionData;
			MenuItemReactor* m_rootMenuItemReactor{ nullptr };
			Point m_lastMousePosition{ -1,-1 };
		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		MenuItemReactor* GetLastMenuItem() const;
		Module m_module;
	};

	class MenuBar : public Control<MenuBarReactor>
	{
	public:
		MenuBar(Window* parent, const Rectangle& rectangle);

		size_t GetCount() const;
		Menu& PushBack(const std::wstring& itemName);
	};
}

#endif