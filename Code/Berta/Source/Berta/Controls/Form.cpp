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
	namespace ReactorCore::Form
	{
		void Reactor::Init(ControlBase& control, Graphics* graphics)
		{
			m_control = &control;
		}

		void Reactor::Update(Graphics& graphics)
		{
#if BT_DEBUG
			//BT_CORE_TRACE << "  - Form::Update | window=" << m_control->Handle()->Name << ". hWnd = " << m_control->Handle()->RootHandle.Handle << std::endl;
#else
			BT_CORE_TRACE << "  - Form::Update. hWnd = " << m_control->Handle()->RootHandle.Handle << std::endl;
#endif
			graphics.FillRectangle(m_control->Handle()->ClientSize.ToRectangle(), m_control->Handle()->Appearance->Background);
		}
		
		FormBase::FormBase(Window* owner, const Size& size, const FormStyle& windowStyle, bool isNested, bool isRenderForm)
		{
			Create(owner, false, GUI::GetCenteredOnScreen(size), windowStyle, isNested, isRenderForm);

#if BT_DEBUG
			m_handle->Name = "Form";
#endif
		}

		FormBase::FormBase(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle, bool isNested, bool isRenderForm)
		{
			Create(owner, false, rectangle, windowStyle, isNested, isRenderForm);

#if BT_DEBUG
			m_handle->Name = "Form";
#endif
		}

		FormBase::FormBase(Window* owner, bool isUnscaleRect, const Rectangle& rectangle, const FormStyle& windowStyle, bool isNested, bool isRenderForm)
		{
			Create(owner, isUnscaleRect, rectangle, windowStyle, isNested, isRenderForm);

#if BT_DEBUG
			m_handle->Name = "Form";
#endif
		}

		API::NativeWindowHandle FormBase::NativeHandle() const
		{
			return GUI::GetNativeHandle(m_handle);
		}

		void FormBase::SetLayout(const std::string& layoutText)
		{
			m_layout.Create(*this);
			m_layout.Parse(layoutText);
		}

		void FormBase::SetCustomPaintCallback(std::function<void()> callback)
		{
			GUI::SetCustomPaintCallback(Handle(), callback);
		}
	}

	Form::Form(const Size& size, const FormStyle& windowStyle, bool isRenderForm) :
		FormBase(nullptr, size, windowStyle, false, isRenderForm)
	{
	}

	Form::Form(const Rectangle& rectangle, const FormStyle& windowStyle, bool isRenderForm) :
		FormBase(nullptr, rectangle, windowStyle, false, isRenderForm)
	{
	}

	Form::Form(Window* owner, const Size& size, const FormStyle& windowStyle, bool isRenderForm) :
		FormBase(owner, size, windowStyle, false, isRenderForm)
	{
	}

	Form::Form(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle, bool isRenderForm) :
		FormBase(owner, rectangle, windowStyle, false, isRenderForm)
	{
	}

	void Form::Exec()
	{
		Foundation::GetInstance().ProcessMessages();
	}

	NestedForm::NestedForm(const Form& owner, const Rectangle& rectangle, const FormStyle& windowStyle, bool isRenderForm) :
		FormBase(owner.Handle(), true, rectangle, windowStyle, true, isRenderForm)
	{
#if BT_DEBUG
		m_handle->Name = "NestedForm";
#endif
	}

	NestedForm::NestedForm(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle, bool isRenderForm) :
		FormBase(owner, true, rectangle, windowStyle, true, isRenderForm)
	{
#if BT_DEBUG
		m_handle->Name = "NestedForm";
#endif
	}
}