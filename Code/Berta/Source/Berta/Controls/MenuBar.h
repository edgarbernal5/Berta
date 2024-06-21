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

		struct MenuBarItemData
		{
			MenuBarItemData(const std::wstring& _text) :text(_text) {}

			std::wstring text;
			Size size;
			Point position;
			Size center;
		};

		struct MenuBarCollectionData
		{
			std::vector<MenuBarItemData*> m_items;

		};

		struct InteractionData
		{
			void PushBack(const std::wstring& text);
			void BuildItems();

			MenuBarCollectionData m_menuCollectionData;
			int m_hoveredItemIndex{ -1 };
			Window* m_owner{ nullptr };

		};
		InteractionData& GetInteractionData() { return m_interactionData; }

	private:
		int FindItem(const Point& position);

		ControlBase* m_control{ nullptr };
		InteractionData m_interactionData;
	};

	class MenuBar : public Control<MenuBarReactor>
	{
	public:
		MenuBar(Window* parent, const Rectangle& rectangle);

		void PushBack(const std::wstring& itemName);
	};
}

#endif