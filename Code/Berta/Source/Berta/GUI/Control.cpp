/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Control.h"

#include "Berta/GUI/Window.h"

namespace Berta
{
	void ControlBase::Caption(const std::wstring& caption)
	{
		DoOnCaption(caption);
	}

	std::wstring ControlBase::Caption()
	{
		return DoOnCaption();
	}

	void ControlBase::Show()
	{
		GUI::ShowWindow(m_handle, true);
	}

	void ControlBase::Hide()
	{
	}

	void ControlBase::Dispose()
	{
		GUI::DisposeWindow(m_handle);
	}

	void ControlBase::DoOnCaption(const std::wstring& caption)
	{
		GUI::CaptionWindow(m_handle, caption);
	}

	std::wstring ControlBase::DoOnCaption()
	{
		return GUI::GetCaptionWindow(m_handle);
	}
}
