/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LIST_BOX_HEADER
#define BT_LIST_BOX_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/Panel.h"

#include <string>
#include <list>

namespace Berta
{
	class ListBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

	private:
	};

	class ListBox : public Control<ListBoxReactor>
	{
	public:
		ListBox() = default;
		ListBox(Window* parent, const Rectangle& rectangle);

		void AppendHeader(const std::string& name, uint32_t width);
		void Clear();
	};
}

#endif
