/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Form.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void FormReactor::Init(ControlBase& control)
	{
		m_control = &control;
	}

	void FormReactor::Update(Graphics& graphics)
	{
#if BT_DEBUG
		BT_CORE_TRACE << "  - Form::Update | window=" << m_control->Handle()->Name << ". hWnd = " << m_control->Handle()->RootHandle.Handle << std::endl;
#else
		BT_CORE_TRACE << "  - Form::Update. hWnd = " << m_control->Handle()->RootHandle.Handle << std::endl;
#endif
		if (m_customDrawing)
		{
			m_customDrawing(graphics);
			return;
		}

		graphics.DrawRectangle(m_control->Handle()->Appearance->Background, true);
	}

	FormBase::FormBase(Window* owner, const Size& size, const FormStyle& windowStyle, bool isNested)
	{
		Create(owner, false, GUI::GetCenteredOnScreen(size), windowStyle, isNested);

#if BT_DEBUG
		m_handle->Name = "Form";
#endif
	}

	FormBase::FormBase(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle, bool isNested)
	{
		Create(owner, false, rectangle, windowStyle, isNested);

#if BT_DEBUG
		m_handle->Name = "Form";
#endif
	}

	FormBase::FormBase(Window* owner, bool isUnscaleRect, const Rectangle& rectangle, const FormStyle& windowStyle, bool isNested)
	{
		Create(owner, isUnscaleRect, rectangle, windowStyle, isNested);

#if BT_DEBUG
		m_handle->Name = "Form";
#endif
	}

	void FormBase::SetLayout(const std::string& layoutText)
	{
		m_layout.Create(*this);
		m_layout.Parse(layoutText);
	}

	Form::Form(const Size& size, const FormStyle& windowStyle) : 
		FormBase(nullptr, size, windowStyle, false)
	{
	}

	Form::Form(const Rectangle& rectangle, const FormStyle& windowStyle) : 
		FormBase(nullptr, rectangle, windowStyle, false)
	{
	}

	Form::Form(Window* owner, const Size& size, const FormStyle& windowStyle) :
		FormBase(owner, size, windowStyle, false)
	{
	}

	Form::Form(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle) :
		FormBase(owner, rectangle, windowStyle, false)
	{
	}

	void Form::Exec()
	{
		Foundation::GetInstance().ProcessMessages();
	}

	NestedForm::NestedForm(const Form& owner, const Rectangle& rectangle, const FormStyle& windowStyle) : 
		FormBase(owner.Handle(), true, rectangle, windowStyle, true)
	{
#if BT_DEBUG
		m_handle->Name = "NestedForm";
#endif
	}

	NestedForm::NestedForm(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle) : 
		FormBase(owner, true, rectangle, windowStyle, true)
	{
#if BT_DEBUG
		m_handle->Name = "NestedForm";
#endif
	}
}