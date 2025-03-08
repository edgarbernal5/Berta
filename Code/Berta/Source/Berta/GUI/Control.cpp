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

	Window* ControlBase::GetParent() const
	{
		return GUI::GetParentWindow(m_handle);
	}

	Point ControlBase::GetPosition() const
	{
		return GUI::GetLocalPosition(m_handle);
	}

	Rectangle ControlBase::GetArea() const
	{
		auto position = GUI::GetAbsolutePosition(m_handle);
		auto size = GetSize();
		return { position.X, position.Y, size.Width, size.Height };
	}

	Size ControlBase::GetSize() const
	{
		return DoOnSize();
	}

	void ControlBase::SetSize(const Size& newSize)
	{
		DoOnSize(newSize);
	}

	bool ControlBase::IsVisible() const
	{
		return GUI::IsWindowVisible(m_handle);
	}

	void ControlBase::Show()
	{
		GUI::ShowWindow(m_handle, true);
	}

	void ControlBase::Hide()
	{
		GUI::ShowWindow(m_handle, false);
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
		return GUI::SizeWindow(m_handle);
	}
}
