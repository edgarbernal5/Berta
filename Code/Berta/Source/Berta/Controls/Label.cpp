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
	namespace Internal::Label
	{
		void Reactor::DoOnInit()
		{
			m_module.m_owner = m_control->Handle();
		}

		void Reactor::Update(Graphics& graphics)
		{
			auto clientRect = m_module.m_owner->ClientSize.ToRectangle();
			auto caption = m_control->GetCaptionW();
			graphics.FillRectangle(clientRect, m_module.m_owner->Appearance->Background);
			
			graphics.DrawString(clientRect, caption, m_module.m_owner->Appearance->Foreground, {m_module.m_isWordWrap, m_module.m_horizontalAlign, m_module.m_verticalAlignment});
		}

		void Reactor::Module::Update()
		{
			GUI::UpdateWindow(m_owner);
		}
	}
	
	Label::Label(Window* parent, const Rectangle& rectangle, const std::wstring& text)
	{
		Create(parent, true, rectangle);
		SetCaption(text);

#if BT_DEBUG
		m_handle->Name = "Label";
#endif
	}

	Label::Label(Window* parent, const Rectangle& rectangle, const std::string& text)
	{
		Create(parent, true, rectangle);
		SetCaption(text);

#if BT_DEBUG
		m_handle->Name = "Label";
#endif
	}

	bool Label::IsWordWrap() const
	{
		return GetReactor().GetModule().m_isWordWrap;
	}

	void Label::SetWordWrap(bool wordWrap)
	{
		auto& module = GetReactor().GetModule();
		module.m_isWordWrap = wordWrap;
		module.Update();
	}

	void Label::SetHorizontalAlignment(HorizontalAlign horizontalAlignment)
	{
		auto& module = GetReactor().GetModule();
		module.m_horizontalAlign = horizontalAlignment;
		module.Update();
	}

	void Label::SetVerticalAlignment(VerticalAlign verticalAlignment)
	{
		auto& module = GetReactor().GetModule();
		module.m_verticalAlignment = verticalAlignment;
		module.Update();
	}
}
