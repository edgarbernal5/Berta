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
	/*
	 *Bien, ahora sí anda todo bien, de forma correcta. Ahora dado el estado actual del código, si quisiera hacer la interacción de la tecla Alt para que el foco lo tenga ahora mi menubar, qué cambios tengo que hacer? Consolidar todos los cambios de todas las clases y hacerlo de forma prolija y profesional con las buenas prácticas y rendimiento, por ahora nos enfocaremos en la plataforma WINDOWS. Tengo manera de saber si mi ventana principal tiene un menubar con un apuntador (en Window tengo un apuntador Window* m_menubar)
	 */
	
	namespace Internal::MenuBar
	{
		struct Appearance : public ControlAppearance
		{
			uint32_t ItemPaddingInner = 8;
		};
		
		class Reactor : public ControlReactor
		{
		public:
			void Update(Graphics& graphics) override;
			
			void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
			void MouseDown(Graphics& graphics, const ArgMouse& args) override;
			void MouseMove(Graphics& graphics, const ArgMouse& args) override;
			void Resize(Graphics& graphics, const ArgResize& args) override;
			void DpiChanged(Graphics& graphics) override;

			void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;

			struct MenuBarItemData
			{
				std::wstring text;
				wchar_t accessKey{ 0 };
				std::size_t accessKeyPosition{ 0 };
				bool isEnabled{ true };
				
				Menu menu; 
			};
			
			struct ItemLayoutCache
			{
				Rectangle bounds;
				Point textPosition;
			};
			
			struct InteractionData
			{
				std::optional<std::size_t> m_selectedIndex;
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
				std::optional<Point> m_lastMousePos{ std::nullopt };
				uint32_t m_closeListenerId { 0 };
				InteractionData m_interaction;
			};
			
			Module& GetModule() { return m_module; }
			const Module& GetModule() const { return m_module; }
		
		protected:
			void DoOnInit() override;
			
		private:
			Module m_module;
		};
	}

	class MenuBar : public Control<Internal::MenuBar::Reactor, ControlEvents, Internal::MenuBar::Appearance>
	{
	public:
		MenuBar() = default;
		MenuBar(Window* parent, const Rectangle& rectangle);
		~MenuBar() override;
		
		Menu& At(size_t index);
		size_t GetCount() const;
		Menu& PushBack(const std::wstring& itemName);
		Menu& PushBack(const std::string& itemName);
	};
}

#endif