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
	InputTextReactor::~InputTextReactor()
	{
		if (m_textEditor)
		{
			delete m_textEditor;
			m_textEditor = nullptr;
		}
	}

	void InputTextReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_textEditor = new TextEditor(*m_control);

		m_textEditor->SetValueChangedCallback([this]()
		{
			ArgTextChanged args;
			args.NewValue = m_textEditor->GetContent();
			reinterpret_cast<InputTextEvents*>(m_control->Handle()->Events.get())->ValueChanged.Emit(args);
		});
	}

	void InputTextReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		graphics.DrawRectangle(window->ClientSize.ToRectangle(), window->Appearance->BoxBackground, true);

		m_textEditor->Render();
		
		graphics.DrawRectangle(window->ClientSize.ToRectangle(), enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);
	}

	void InputTextReactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
		GUI::ChangeCursor(*m_control, Cursor::IBeam);
	}

	void InputTextReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		GUI::ChangeCursor(*m_control, Cursor::Default);
	}

	void InputTextReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		GUI::Capture(*m_control);

		m_textEditor->OnMouseDown(args);
		GUI::MarkAsNeedUpdate(m_control->Handle());
	}

	void InputTextReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto window = m_control->Handle();
		
		m_textEditor->OnMouseMove(args);
		GUI::MarkAsNeedUpdate(window);
	}

	void InputTextReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		m_textEditor->OnMouseUp(args);
		GUI::ReleaseCapture(*m_control);
	}

	void InputTextReactor::Focus(Graphics& graphics, const ArgFocus& args)
	{
		auto window = m_control->Handle();
		m_textEditor->OnFocus(args);
	}

	void InputTextReactor::KeyChar(Graphics& graphics, const ArgKeyboard& args)
	{
		BT_CORE_DEBUG << "key char: " << (int)args.Key << ". " << std::endl;
		if (m_textEditor->OnKeyChar(args))
		{
			GUI::CaptionWindow(m_control->Handle(), m_textEditor->GetContent());
			GUI::MarkAsNeedUpdate(m_control->Handle());
		}
	}

	void InputTextReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		bool redraw = m_textEditor->OnKeyPressed(args);
		if (redraw)
		{
			auto window = m_control->Handle();
			GUI::MarkAsNeedUpdate(window);
		}
	}

	void InputTextReactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
	{
		m_textEditor->OnKeyReleased(args);
	}

	void InputTextReactor::DblClick(Graphics& graphics, const ArgMouse& args)
	{
		if (m_textEditor->OnDblClick(args))
		{
			auto window = m_control->Handle();
			GUI::MarkAsNeedUpdate(window);
		}
	}

	InputText::InputText(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "InputText";
#endif
	}

	void InputText::DoOnCaption(const std::wstring& caption)
	{
		auto editor = m_reactor.GetEditor();
		if (editor)
		{
			editor->SetContent(caption);
		}
	}

	std::wstring InputText::DoOnCaption() const
	{
		return m_reactor.GetEditor()->GetContent();
	}
}