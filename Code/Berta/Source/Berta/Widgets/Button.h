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
	class ButtonRenderer : public WidgetRenderer
	{
	public:
		void Init(WidgetBase& widget) override;
		void Update(Graphics& graphics) override;

	private:
		WidgetBase* m_widget;
	};

	class Button : public Widget<ButtonRenderer>
	{
	public:
		Button(Window* parent, const Rectangle& rectangle, std::wstring text);
	};
}

#endif