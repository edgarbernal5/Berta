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

	//TODO:
	/*
	Ok. Habría que implementar otros mensajes wm_syschar o wm_syschardown o wm_char (realmente no recuerdo bien los nombres pero revisar en mí implementación en Foundation de win32
	*/
	namespace ReactorCore::MenuBar
	{
		class Reactor : public ControlReactor
		{
		public:
			void Init(ControlBase& control, Graphics* graphics) override;
			void Update(Graphics& graphics) override;

			void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
			void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
			void MouseDown(Graphics& graphics, const ArgMouse& args) override;
			void MouseMove(Graphics& graphics, const ArgMouse& args) override;
			void Resize(Graphics& graphics, const ArgResize& args) override;

			void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;

			struct MenuBarItemData
			{
				MenuBarItemData(std::wstring _text, wchar_t _accessKey, std::size_t _accessKeyPosition) :
					text(std::move(_text)),
					accessKey(_accessKey),
					accessKeyPosition(_accessKeyPosition)
				{
				}

				Menu menu;
				std::wstring text;
				wchar_t accessKey;
				std::size_t accessKeyPosition;
				bool isEnabled{ true };
			};

			struct ItemLayoutCache
			{
				Rectangle bounds;
				Point textPosition;
			};
			
			struct InteractionData
			{
				int		m_selectedItemIndex{ -1 };
				Menu*	m_activeMenu{ nullptr };
			};

			struct Module
			{
				Menu& At(size_t index);
				void BuildItems(size_t startIndex = 0);
				int FindItem(const Point& position) const;
				Menu& PushBack(const std::wstring& text);
				void OpenMenu(bool ignoreFirstMouseUp = false);
				void SelectIndex(int index);
				bool IsMenuOpen() const { return m_interactionData.m_activeMenu; }
				Berta::MenuBox* GetActiveMenuBox() const;

				Berta::MenuBar* m_control{ nullptr };
				Window* m_owner{ nullptr };
				std::vector<MenuBarItemData> m_items;
				InteractionData m_interactionData;
				Point m_lastMousePosition{ -1,-1 };
			};

			Module& GetModule() { return m_module; }
			const Module& GetModule() const { return m_module; }

		private:
			Module m_module;
		};
	}

	class MenuBar : public Control<ReactorCore::MenuBar::Reactor>
	{
	public:
		MenuBar() = default;
		MenuBar(Window* parent, const Rectangle& rectangle);
		
		Menu& At(size_t index);
		size_t GetCount() const;
		Menu& PushBack(const std::wstring& itemName);
		Menu& PushBack(const std::string& itemName);
	};
}

#endif