/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "InputText.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/Caret.h"
#include "Berta/Controls/TextEditors/TextEditor.h"

namespace Berta
{
	void InputTextReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_textEditor = new TextEditor(*m_control);
	}

	void InputTextReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->Size.ToRectangle(), GUI::GetBoxBackgroundColor(window), true);

		m_textEditor->Render();
		
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBorderColor, false);
	}

	void InputTextReactor::Focus(Graphics& graphics, const ArgFocus& args)
	{
		auto window = m_control->Handle();
		if (args.Focused)
		{
			m_textEditor->ActivateCaret();
		}
		else
		{
			m_textEditor->DeactivateCaret();
		}
	}

	void InputTextReactor::KeyChar(Graphics& graphics, const ArgKeyboard& args)
	{
		BT_CORE_DEBUG << "key char: " << (int)args.Key << ". " << std::endl;
		if (std::isprint(static_cast<int>(args.Key)))
		{
			m_textEditor->Insert(args.Key);

			auto window = m_control->Handle();
			GUI::CaptionWindow(window, m_textEditor->GetContent());
			window->Renderer.Update();
			GUI::RefreshWindow(window);
		}
	}

	void InputTextReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		bool redraw = false;
		if (args.Key == VK_LEFT && m_textEditor->GetCaretPosition() > 0)
		{
			m_textEditor->MoveCaretLeft();
			redraw = true;
		}
		else if (args.Key == VK_RIGHT && m_textEditor->GetCaretPosition() < m_textEditor->GetContent().size())
		{
			m_textEditor->MoveCaretRight();
			redraw = true;
		}
		else if (args.Key == VK_BACK && m_textEditor->GetCaretPosition() > 0) {
			
			m_textEditor->DeleteBack();
			redraw = true;
		}
		else if (args.Key == VK_DELETE && m_textEditor->GetCaretPosition() < m_textEditor->GetContent().size()) {
			
			m_textEditor->Delete();
			redraw = true;
		}

		if (redraw)
		{
			auto window = m_control->Handle();
			window->Renderer.Update();
			GUI::RefreshWindow(window);
		}
	}

	InputText::InputText(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle);
	}
}