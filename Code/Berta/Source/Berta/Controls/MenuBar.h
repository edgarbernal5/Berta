/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_MENU_BAR_HEADER
#define BT_MENU_BAR_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
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

		struct MenuItemData
		{
			std::wstring name;
			Size size;
			Point position;

			MenuItemData(const std::wstring& _name) :name(_name){}
		};
		struct MenuCollectionData
		{
			std::vector<MenuItemData> m_items;
			int m_hoveredItemIndex{ -1 };
		};
		MenuCollectionData& GetCollectionData() { return m_menuCollectionData; }
	private:
		enum class State
		{
			Normal,
			Pressed,
			Hovered
		};

		ControlBase* m_control{ nullptr };
		State m_status{ State::Normal };
		MenuCollectionData m_menuCollectionData;
	};

	class MenuBar : public Control<MenuBarReactor>
	{
	public:
		MenuBar(Window* parent, const Rectangle& rectangle);

		void PushBack(const std::wstring& itemName);
	};
}

#endif