/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ComboBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/Caret.h"
#include "Berta/Controls/TextEditors/TextEditor.h"
#include "Berta/Controls/Floating/FloatBox.h"

namespace Berta
{
	ComboBoxReactor::~ComboBoxReactor()
	{
		if (m_textEditor)
		{
			delete m_textEditor;
			m_textEditor = nullptr;
		}
	}

	void ComboBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_textEditor = new TextEditor(*m_control);

		auto window = m_control->Handle();
		window->Events->Focus.Connect([&](const ArgFocus& args) {
			if (!args.Focused && m_floatBox)
			{
				delete m_floatBox;
				m_floatBox = nullptr;
			}
		});
	}

	void ComboBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->Size.ToRectangle(), GUI::GetBoxBackgroundColor(window), true);

		//m_textEditor->Render();

		graphics.DrawLine({ (int)window->Size.Width - 25, 1 }, { (int)window->Size.Width - 25, (int)window->Size.Height - 1 }, window->Appereance->BoxBorderColor);
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBorderColor, false);
	}

	void ComboBoxReactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
		//GUI::ChangeCursor(*m_control, Cursor::IBeam);
	}

	void ComboBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		//GUI::ChangeCursor(*m_control, Cursor::Default);
	}

	void ComboBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (args.ButtonState.LeftButton)
		{
			if (m_floatBox)
			{
				delete m_floatBox;
				m_floatBox = nullptr;
			}
			else
			{
				auto window = m_control->Handle();
				auto point = GUI::GetPointClientToScreen(window, m_control->Handle()->Position);
				m_floatBox = new FloatBox(window, { point.X,point.Y + (int)window->Size.Height,window->Size.Width,300 });
				m_floatBox->SetItems(m_items);
				m_floatBox->Show();
			}
		}
		/*m_textEditor->OnMouseDown(args);
		m_control->Handle()->Renderer.Update();
		GUI::UpdateDeferred(m_control->Handle());*/
	}

	void ComboBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		//m_textEditor->OnMouseMove(args);
		//m_control->Handle()->Renderer.Update();
		//GUI::UpdateDeferred(m_control->Handle());
	}

	void ComboBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		//m_textEditor->OnMouseUp(args);
	}

	void ComboBoxReactor::Focus(Graphics& graphics, const ArgFocus& args)
	{
		//auto window = m_control->Handle();
		//m_textEditor->OnFocus(args);
	}

	void ComboBoxReactor::KeyChar(Graphics& graphics, const ArgKeyboard& args)
	{
		BT_CORE_DEBUG << "key char: " << (int)args.Key << ". " << std::endl;
		if (m_textEditor->OnKeyChar(args))
		{
			//GUI::CaptionWindow(m_control->Handle(), m_textEditor->GetContent());
			//m_control->Handle()->Renderer.Update();
			//GUI::UpdateDeferred(m_control->Handle());
		}
	}

	void ComboBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		//bool redraw = m_textEditor->OnKeyPressed(args);
		//if (redraw)
		{
			//auto window = m_control->Handle();
			//window->Renderer.Update();
			//GUI::UpdateDeferred(window);
		}
	}

	ComboBox::ComboBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle);
	}

	void ComboBox::PushItem(const std::wstring& text)
	{
		m_reactor.m_items.push_back(text);
	}

	void ComboBox::DoOnCaption(const std::wstring& caption)
	{
		//auto editor = m_reactor.GetEditor();
		//if (editor)
		//{
		//	editor->SetContent(caption);
		//}
	}

	std::wstring ComboBox::DoOnCaption()
	{
		return std::wstring();
	}
}