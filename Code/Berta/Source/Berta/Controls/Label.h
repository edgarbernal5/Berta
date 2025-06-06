/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LABEL_HEADER
#define BT_LABEL_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include <string>

namespace Berta
{
	class LabelReactor : public ControlReactor
	{
	public:
		void Update(Graphics& graphics) override;

	private:
	};

	class Label : public Control<LabelReactor>
	{
	public:
		Label() = default;
		Label(Window* parent, const Rectangle& rectangle, const std::wstring& text);
		Label(Window* parent, const Rectangle& rectangle, const std::string& text);
	};
}

#endif