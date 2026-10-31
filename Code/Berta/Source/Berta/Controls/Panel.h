/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PANEL_HEADER
#define BT_PANEL_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"

namespace Berta
{
	namespace Internal::Panel
	{
		class Reactor : public ControlReactor
		{
		protected:
			void DoOnInit() override;
		};
	}
	
	class Panel : public Control<Category::PanelTag, Internal::Panel::Reactor>
	{
	public:
		Panel() = default;
		Panel(Window* parent, const Rectangle& rectangle = {}, bool visible = true);
		Panel(Window* parent, bool isUnscaleRect, const Rectangle& rectangle = {}, bool visible = true);
	};
}

#endif
