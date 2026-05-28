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
	
	namespace Internal::Form
	{
		class Reactor : public ControlReactor
		{
		public:
			void Update(Graphics& graphics) override;
			
		private:
		};
		
		class FormBase : public Control<Category::RootTag, Internal::Form::Reactor, FormEvents>
		{
		public:
			explicit FormBase(Window* owner, const Size& size, const FormStyle& windowStyle, bool isNested, bool isRenderForm);
			FormBase(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle, bool isNested, bool isRenderForm);
			FormBase(Window* owner, bool isUnscaleRect, const Rectangle& rectangle, const FormStyle& windowStyle, bool isNested, bool isRenderForm);

			API::NativeWindowHandle NativeHandle() const;

			Layout& GetLayout() { return m_layout; }
			void SetLayout(const std::string& layoutText);

			void SetCustomPaintCallback(std::function<void()> callback);

		private:
			Layout m_layout;
		};
	}

	class Form : public Internal::Form::FormBase
	{
	public:
		explicit Form(const Size& size, const FormStyle& windowStyle = { true, true, true }, bool isRenderForm = false);
		Form(const Rectangle& rectangle, const FormStyle& windowStyle = { true, true, true }, bool isRenderForm = false);
		explicit Form(Window* owner, const Size& size, const FormStyle& windowStyle = { true, true, true }, bool isRenderForm = false);
		Form(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle = { true, true, true }, bool isRenderForm = false);

		void Exec();
		DialogResult Exec(Window* owner);
		
		void Close(DialogResult result);
		
		[[nodiscard]] DialogResult GetDialogResult() const { return m_dialogResult; }
		
	protected:
		void DoNotifyClose() override;
		
	private:
		bool m_isClosed{ false };
		DialogResult m_dialogResult{ DialogResult::None };
	};

	class NestedForm : public Internal::Form::FormBase
	{
	public:
		NestedForm(const Form& owner, const Rectangle& rectangle, const FormStyle& windowStyle = { false, false, false, false, false, false }, bool isRenderForm = false);
		NestedForm(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle = { false, false, false, false, false, false }, bool isRenderForm = false);

	private:
	};
}

#endif