/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BUTTON_HEADER
#define BT_BUTTON_HEADER

#include <optional>

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include <string>

#include "Berta/Paint/Image.h"
#include "Buttons/EnumTypes.h"

namespace Berta
{
	namespace Internal::Button
	{
		class Reactor : public ControlReactor
		{
		public:
			void Update(Graphics& graphics) override;

			void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
			void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
			void MouseDown(Graphics& graphics, const ArgMouse& args) override;
			void MouseUp(Graphics& graphics, const ArgMouse& args) override;
			
			enum class State : uint8_t
			{
				Normal,
				Pressed,
				Hovered
			};
			struct Module
			{
				
				void SetContentHorizontalAlign(HorizontalAlign halign);
				void SetContentVerticalAlign(VerticalAlign valign);
				void SetIcon(const Image& icon);
				
				State m_status{ State::Normal };
				std::optional<Berta::Image> Icon;
				HorizontalAlign ContentHAlign{ HorizontalAlign::Center };
				VerticalAlign ContentVAlign{ VerticalAlign::Center };
			};
			Module& GetModule() { return m_module; }
			const Module& GetModule() const { return m_module; }
			
		private:
			Module m_module;
			
		};
	}
	
	class Button : public Control<Category::ControlTag, Internal::Button::Reactor, ControlEvents, Internal::Buttons::Appearance>
	{
	public:
		Button() = default;
		Button(Window* parent, const Rectangle& rectangle, const std::string& text);
		Button(Window* parent, const Rectangle& rectangle, const std::wstring& text);
		Button(Window* parent, const Rectangle& rectangle, const std::string& text, const Image& icon);
		Button(Window* parent, const Rectangle& rectangle, const std::wstring& text, const Image& icon);
		Button(Window* parent, const Rectangle& rectangle, const Image& icon);
		
		void SetContentHorizontalAlign(const HorizontalAlign& halign);
		void SetContentVerticalAlign(const VerticalAlign& valign);
		void SetIcon(const Image& icon);
	};
}

#endif