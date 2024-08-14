/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LABEL_HEADER
#define BT_LABEL_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Paint/Image.h"
#include <string>

namespace Berta
{
	class LabelReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

	private:
		Image m_image;
	};

	class Label : public Control<LabelReactor>
	{
	public:
		Label(Window* parent, const Rectangle& rectangle, const std::wstring& text);
	};
}

#endif