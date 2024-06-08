/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SCROLL_BAR_HEADER
#define BT_SCROLL_BAR_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include <string>

namespace Berta
{
	class ScrollBarReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

	private:
		ControlBase* m_control{ nullptr };
	};

	class ScrollBar : public Control<ScrollBarReactor>
	{
	public:
		ScrollBar(Window* parent, const Rectangle& rectangle);
	};
}

#endif