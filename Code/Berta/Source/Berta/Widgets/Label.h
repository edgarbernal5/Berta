/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LABEL_HEADER
#define BT_LABEL_HEADER

#include "Berta/Core/Widget.h"
#include "Berta/GUI/BasicWindow.h"
#include <string>

namespace Berta
{
	class LabelRenderer : public WidgetRenderer
	{
	public:
		void Update() override {}
	};

	class Label : public Widget<LabelRenderer>
	{
	public:
		Label(BasicWindow* parent, const Rectangle& rectangle, std::wstring text);
	};
}

#endif