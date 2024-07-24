/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Label.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void LabelReactor::Init(ControlBase& control)
	{
		m_control = &control;
		//m_image.Open("..\\..\\Resources\\Escudo.png");
		m_image.Open("..\\..\\Resources\\Icono.bmp");
		//m_image.Open("..\\..\\Resources\\Icono.jpg");
		//m_image.Open("..\\..\\Resources\\Icons\\Game.ico");
	}

	void LabelReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->Background, true);
		graphics.DrawString({ 0,0 }, m_control->GetCaption(), window->Appereance->Foreground);

		//m_image.Paste(graphics, { 0, 0 });
	}

	Label::Label(Window* parent, const Rectangle& rectangle, std::wstring text)
	{
		Create(parent, true, rectangle);
		SetCaption(text);
	}
}