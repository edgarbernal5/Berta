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
	void LabelReactor::Init(ControlBase& control, Graphics* graphics)
	{
		ControlReactor::Init(control, graphics);
		
		m_module.m_owner = control.Handle();
	}

	void LabelReactor::Update(Graphics& graphics)
	{
		auto clientRect = m_module.m_owner->ClientSize.ToRectangle();
		auto caption = m_control->GetCaption();
		auto rect = graphics.GetTextExtent(caption, clientRect);
		graphics.DrawRectangle(clientRect, m_module.m_owner->Appearance->Background, true);
		
		
		graphics.DrawString(clientRect, m_control->GetCaption(), m_module.m_owner->Appearance->Foreground, m_module.m_isWordWrap);
	}

	void LabelReactor::Module::Update()
	{
		GUI::UpdateWindow(m_owner);
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
