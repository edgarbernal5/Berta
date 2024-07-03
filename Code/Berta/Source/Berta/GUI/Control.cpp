/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Control.h"

#include "Berta/GUI/Window.h"
#include "Berta/GUI/ControlWindow.h"

namespace Berta
{
	void ControlBase::SetCaption(const std::wstring& caption)
	{
		DoOnCaption(caption);
	}

	std::wstring ControlBase::GetCaption() const
	{
		return DoOnCaption();
	}

	bool ControlBase::GetEnabled() const
	{
		return DoOnEnabled();
	}

	void ControlBase::SetEnabled(bool enabled)
	{
		DoOnEnabled(enabled);
	}

	Size ControlBase::GetSize() const
	{
		return DoOnSize();
	}

	void ControlBase::SetSize(const Size& newSize)
	{
		DoOnSize(newSize);
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

	std::wstring ControlBase::DoOnCaption() const
	{
		return GUI::CaptionWindow(m_handle);
	}

	void ControlBase::DoOnEnabled(bool enabled)
	{
		GUI::EnableWindow(m_handle, enabled);
	}

	bool ControlBase::DoOnEnabled() const
	{
		return GUI::EnableWindow(m_handle);
	}

	void ControlBase::DoOnSize(const Size& newSize)
	{
		GUI::ResizeWindow(m_handle, newSize);
	}

	Size ControlBase::DoOnSize() const
	{
		return GUI::ResizeWindow(m_handle);
	}

	/*void ControlBase::NotifyDestroy()
	{
		m_handle = nullptr;
		DoOnNotifyDestroy();
	}*/

	//void ControlWindow::Destroy()
	//{
	//	m_control.NotifyDestroy();
	//}

	
}
