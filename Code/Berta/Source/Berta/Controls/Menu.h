/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_MENU_HEADER
#define BT_MENU_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include <string>
#include <functional>
#include <vector>

namespace Berta
{
	class MenuBox;
	constexpr uint32_t ItemTextPadding = 2;
	constexpr uint32_t SeparatorHeight = 3;

	struct Menu
	{
		using ClickCallback = std::function<void()>;
		using DestroyCallback = std::function<void()>;

		void Append(const std::wstring& text, ClickCallback onClick);
		void AppendSeparator();
		void ShowPopup(Window* parent, const Point& position, bool ignoreFirstMouseUp = true);
		Menu* CreateSubMenu(std::size_t index);

		struct Item
		{
			Item() : isSpearator(true) {}
			Item(const std::wstring& _text, ClickCallback _onClick) : 
				text(_text), 
				isSpearator(false),
				onClick(_onClick){}

			std::wstring text;
			bool isSpearator{ false };
			ClickCallback onClick;
			Menu* m_subMenu{ nullptr };
		};

		std::vector<Item*> m_items;
		MenuBox* m_menuBox;
		bool m_popupFromMenuBar{ false };
		Window* m_parent{ nullptr };
		DestroyCallback m_destroyCallback;

		MenuBox* GetMenuBox() const { return m_menuBox; }
		void CloseMenuBox();
	private:
		Size GetMenuBoxSize(Window* parent);
	};


	class MenuBoxReactor : public ControlReactor
	{
	public:
		~MenuBoxReactor();

		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		//void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
		//void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;

		void SetItems(std::vector<Menu::Item*>& items);
		void SetIgnoreFirstMouseUp(bool value) { m_ignoreFirstMouseUp = value; }
	private:
		struct MenuBoxItem
		{
			Point m_position;
			Size m_size;
		};
		MenuBox* m_control{ nullptr };
		bool m_ignoreFirstMouseUp{ true };
		std::vector<Menu::Item*>* m_items{ nullptr };
		std::vector<MenuBoxItem> m_itemSizePositions;

		int m_selectedIndex{ -1 };
	};

	class MenuBox : public Control<MenuBoxReactor, RootEvents>
	{
	public:
		MenuBox(Window* parent, const Rectangle& rectangle);
		~MenuBox();

		void Init(std::vector<Menu::Item*>& items);
		void SetIgnoreFirstMouseUp(bool value);
	private:
	};
}

#endif