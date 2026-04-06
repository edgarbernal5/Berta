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
#include <optional>

namespace Berta
{
	class MenuBar;

	//TODO:
	/*
	Ok. Habría que implementar otros mensajes wm_syschar o wm_syschardown o wm_char (realmente no recuerdo bien los nombres pero revisar en mí implementación en Foundation de win32
	*/
	namespace ReactorCore::MenuBar
	{
		struct Appearance : public ControlAppearance
		{
			uint32_t ItemPaddingInner = 8;
		};
		
		class Reactor : public ControlReactor
		{
		public:
			void Init(ControlBase& control, Graphics* graphics) override;
			void Update(Graphics& graphics) override;
			
			void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
			void MouseDown(Graphics& graphics, const ArgMouse& args) override;
			void MouseMove(Graphics& graphics, const ArgMouse& args) override;
			void Resize(Graphics& graphics, const ArgResize& args) override;

			void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;

			struct MenuBarItemData
			{
				std::wstring text;
				wchar_t accessKey{ 0 };
				std::size_t accessKeyPosition{ 0 };
				bool isEnabled{ true };
            
				// Cada categoría de la barra (Archivo, Edición) es dueña de su propio Menu
				Menu menu; 
			};
			
			struct ItemLayoutCache
			{
				Rectangle bounds;
				Point textPosition;
			};
			
			struct InteractionData
			{
				std::optional<std::size_t> m_selectedIndex; // C++17: Reemplaza al clásico -1
				bool m_isMenuOpen{ false };
			};
			
			struct Module
			{
				Menu& At(size_t index);
				Menu& PushBack(const std::wstring& text);
				void CalculateLayout();
				void OpenMenu(bool focusFirstItem = false);
				void MoveSelection(int step);

				Window* m_owner{ nullptr };
				ControlBase* m_control{ nullptr };
            
				std::vector<MenuBarItemData> m_items;
				std::vector<ItemLayoutCache> m_layoutCache;
				InteractionData m_interaction;
			};
			
			Module& GetModule() { return m_module; }
			const Module& GetModule() const { return m_module; }

		private:
			Module m_module;
		};
	}

	class MenuBar : public Control<ReactorCore::MenuBar::Reactor, ControlEvents, ReactorCore::MenuBar::Appearance>
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