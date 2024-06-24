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
	class MenuBarReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;

		struct MenuBarItemData
		{
			MenuBarItemData(const std::wstring& _text) :text(_text) {}

			Menu menu;
			std::wstring text;
			Size size;
			Point position;
			Size center;
		};

		struct InteractionData
		{
			int						m_selectedItemIndex{ -1 };
			Menu*					m_activeMenu{ nullptr };
		};
		struct Module
		{
			Menu& PushBack(const std::wstring& text);
			void BuildItems();
			int FindItem(const Point& position);
			void OpenMenu();

			ControlBase* m_control{ nullptr };
			Window* m_owner{ nullptr };
			std::vector<MenuBarItemData*> m_items;
			InteractionData m_interactionData;
		};
		Module& GetModule() { return m_module; }

	private:

		Module m_module;
	};

	class MenuBar : public Control<MenuBarReactor>
	{
	public:
		MenuBar(Window* parent, const Rectangle& rectangle);

		Menu& PushBack(const std::wstring& itemName);
	};
}

#endif