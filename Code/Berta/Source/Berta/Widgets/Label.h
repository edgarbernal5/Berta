/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LABEL_HEADER
#define BT_LABEL_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Widget.h"
#include <string>

namespace Berta
{
	class LabelRenderer : public WidgetRenderer
	{
	public:
		void Init(WidgetBase& widget) override;
		void Update(Graphics& graphics) override;

	private:
		WidgetBase* m_widget;
	};

	class Label : public Widget<LabelRenderer>
	{
	public:
		Label(Window* parent, const Rectangle& rectangle, std::wstring text);
	};
}

#endif