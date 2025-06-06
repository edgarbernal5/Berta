/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FORM_HEADER
#define BT_FORM_HEADER

#include <Windows.h>
#include <string>

#include "Berta/EntryPoint.h"
#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/Control.h"
#include "Berta/GUI/Layout.h"

namespace Berta
{
	class ControlBase;

	class FormReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void SetCustomDrawing(std::function<void(Graphics&)>&& fn)
		{
			m_customDrawing = std::move(fn);
		}
	private:
		std::function<void(Graphics&)> m_customDrawing;
	};

	class FormBase : public Control<FormReactor, FormEvents>
	{
	public:
		explicit FormBase(Window* owner, const Size& size, const FormStyle& windowStyle, bool isNested, bool isRenderForm);
		FormBase(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle, bool isNested, bool isRenderForm);
		FormBase(Window* owner, bool isUnscaleRect, const Rectangle& rectangle, const FormStyle& windowStyle, bool isNested, bool isRenderForm);

		Layout& GetLayout()
		{
			return m_layout;
		}

		void SetLayout(const std::string& layoutText);

		void SetCustomDrawing(std::function<void(Graphics&)>&& fn)
		{
			m_reactor.SetCustomDrawing(std::move(fn));
		}
	private:
		Layout m_layout;
	};

	class Form : public FormBase
	{
	public:
		explicit Form(const Size& size, const FormStyle& windowStyle = { true, true, true }, bool isRenderForm = false);
		Form(const Rectangle& rectangle, const FormStyle& windowStyle = { true, true, true }, bool isRenderForm = false);
		explicit Form(Window* owner, const Size& size, const FormStyle& windowStyle = { true, true, true }, bool isRenderForm = false);
		Form(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle = { true, true, true }, bool isRenderForm = false);

		void Exec();
	private:
	};

	class NestedForm : public FormBase
	{
	public:
		NestedForm(const Form& owner, const Rectangle& rectangle, const FormStyle& windowStyle = { false, false, false, false, false, false }, bool isRenderForm = false);
		NestedForm(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle = { false, false, false, false, false, false }, bool isRenderForm = false);

	private:
	};
}

#endif