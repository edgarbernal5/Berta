/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BUTTON_HEADER
#define BT_BUTTON_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include <string>

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
			
		private:
			enum class State : uint8_t
			{
				Normal,
				Pressed,
				Hovered
			};
			State m_status{ State::Normal };
		};
	}
	
	class Button : public Control<Category::ControlTag, Internal::Button::Reactor>
	{
	public:
		Button() = default;
		Button(Window* parent, const Rectangle& rectangle, const std::string& text);
		Button(Window* parent, const Rectangle& rectangle, const std::wstring& text);
	};
}

#endif