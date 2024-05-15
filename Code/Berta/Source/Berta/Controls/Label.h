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
	class LabelRenderer : public ControlRenderer
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

	private:
		ControlBase* m_control;
	};

	class Label : public Control<LabelRenderer>
	{
	public:
		Label(Window* parent, const Rectangle& rectangle, std::wstring text);
	};
}

#endif