/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Widget.h"

#include "Berta/GUI/BasicWindow.h"

namespace Berta
{
	void WidgetBase::Caption(const std::wstring& caption)
	{
		GUI::CaptionWindow(m_handle, caption);
	}

	std::wstring WidgetBase::Caption()
	{
		return GUI::GetCaptionWindow(m_handle);
	}

	void WidgetBase::Show()
	{
		GUI::ShowBasicWindow(m_handle, true);
	}
}