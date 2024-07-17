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

	class MenuBarReactor : public ControlReactor, public MenuBarItemReactor, public MenuItemReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;

		bool OnMBIKeyPressed(const ArgKeyboard& args) override;
		void OnMBIMoveRight() override;
		void OnKeyUpPressed() override {};
		bool OnKeyRightPressed() override { return false; };

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
			Menu& PushBack(const std::wstring& text);
			void BuildItems(size_t startIndex = 0);
			int FindItem(const Point& position);
			void OpenMenu(bool ignoreFirstMouseUp = true);
			void SelectIndex(int index);

			MenuBox* GetActiveMenuBox() const;

			MenuBar* m_control{ nullptr };
			Window* m_owner{ nullptr };
			std::vector<MenuBarItemData*> m_items;
			InteractionData m_interactionData;
			MenuItemReactor* m_rootMenuItemReactor{ nullptr };
		};
		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
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