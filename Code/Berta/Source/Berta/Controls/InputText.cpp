/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "InputText.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/Caret.h"

namespace Berta
{
	void InputTextRenderer::Init(ControlBase& control)
	{
		m_control = &control;
	}

	void InputTextRenderer::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->Size.ToRectangle(), GUI::GetBoxBackgroundColor(window), true);
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBorderColor, false);
		graphics.DrawString({ 0,0 }, m_control->Caption(), GUI::GetForegroundColor(window));

		if (window->Caret)
		{
			if (window->Caret->IsVisible())
			{
				BT_CORE_TRACE << "draw caret" << std::endl;
				graphics.DrawLine({ 2,3 }, { 2, (int)window->Size.Height - 2 }, { 0 });
				graphics.DrawLine({ 3,3 }, { 3, (int)window->Size.Height - 2 }, { 0 });
			}
			else
			{
				BT_CORE_TRACE << "hide caret" << std::endl;
			}
		}
	}

	void InputTextRenderer::Focus(Graphics& graphics, const ArgFocus& args)
	{
		auto window = m_control->Handle();
		if (args.Focused)
		{
			window->Caret->Activate();
		}
		else
		{
			window->Caret->Deactivate();
		}
	}

	InputText::InputText(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle);
		GUI::CreateCaret(m_handle);
	}
}