/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Control.h"

#include <algorithm>

#include "Berta/GUI/Window.h"
#include "Berta/GUI/ControlWindow.h"

namespace Berta
{
	std::string ControlBase::GetCaption() const
	{
		return StringUtils::WideToUTF8(DoOnCaption());
	}

	std::wstring ControlBase::GetCaptionW() const
	{
		return DoOnCaption();
	}

	void ControlBase::SetCaption(const std::wstring& caption)
	{
		DoOnCaption(caption);
	}

	void ControlBase::SetCaption(const std::string& caption)
	{
		std::wstring wCaption = StringUtils::UTF8ToWide(caption);
		DoOnCaption(wCaption);
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

	void ControlBase::SetParent(Window* newParent) const
	{
		GUI::SetParentWindow(m_handle, newParent);
	}

	Window* ControlBase::GetOwner() const
	{
		return GUI::GetOwnerWindow(m_handle);
	}

	Point ControlBase::GetPosition() const
	{
		return GUI::GetWindowPosition(m_handle);
	}

	void ControlBase::SetPosition(const Point& newPosition)
	{
		DoOnMove(newPosition);
	}

	Rectangle ControlBase::GetArea() const
	{
		auto position = GUI::GetWindowRootPosition(m_handle);
		
		auto size = GetSize();
		return { position.X, position.Y, size.Width, size.Height };
	}

	void ControlBase::SetArea(const Rectangle& area)
	{
		DoOnMove(area);
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

	void ControlBase::Show() const
	{
		GUI::ShowWindow(m_handle, true);
	}

	void ControlBase::Hide() const
	{
		GUI::ShowWindow(m_handle, false);
	}

	void ControlBase::Dispose() const
	{
		GUI::DisposeWindow(m_handle);
	}
	
	void ControlBase::Capture(bool redirectToChildren)
	{
		GUI::Capture(m_handle, redirectToChildren);
	}

	void ControlBase::ReleaseCapture()
	{
		GUI::ReleaseCapture(m_handle);
	}

	bool ControlBase::IsBorderless() const
	{
		return GUI::IsWindowBorderless(m_handle);
	}

	Rectangle ControlBase::GetClientArea() const
	{
		int x = 0;
		int y = 0;
		const auto size = GetSize();
		int width = static_cast<int>(size.Width);
		int height = static_cast<int>(size.Height);

		bool hasBorder = !m_handle->Flags.Borderless;
		const int borderThickness = hasBorder ? 1 : 0;

		if (borderThickness > 0)
		{
			x += borderThickness;
			y += borderThickness;
			width -= (borderThickness * 2);
			height -= (borderThickness * 2);

			width = std::max<int>(width, 0);
			height = std::max<int>(height, 0);
		}

		return { x, y, static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	}

	bool ControlBase::IsAutoDraw() const
	{
		return m_handle->Flags.AutoDraw;
	}

	void ControlBase::SetAutoDraw(bool autoDraw) const
	{
		m_handle->Flags.AutoDraw = autoDraw;
	}

	void ControlBase::SetBackgroundColor(const Color& newColor)
	{
		GUI::SetBackgroundColor(m_handle, newColor);
	}

	void ControlBase::MakeActive(bool activated, Window* makeTargetWhenInactive)
	{
		GUI::MakeWindowActive(m_handle, activated, makeTargetWhenInactive);
	}

	void ControlBase::Focus()
	{
		GUI::FocusWindow(m_handle);
	}

	std::wstring ControlBase::DoOnCaption() const
	{
		return GUI::CaptionWindow(m_handle);
	}

	void ControlBase::DoOnCaption(const std::wstring& caption)
	{
		GUI::CaptionWindow(m_handle, caption);
	}

	bool ControlBase::DoOnEnabled() const
	{
		return GUI::IsWindowEnabled(m_handle);
	}

	void ControlBase::DoOnEnabled(bool enabled)
	{
		GUI::EnableWindow(m_handle, enabled);
	}

	Size ControlBase::DoOnSize() const
	{
		return GUI::SizeWindow(m_handle);
	}

	void ControlBase::DoOnSize(const Size& newSize)
	{
		GUI::ResizeWindow(m_handle, newSize);
	}

	void ControlBase::DoOnMove(const Point& newPoint)
	{
		GUI::MoveWindow(m_handle, newPoint);
	}

	void ControlBase::DoOnMove(const Rectangle& newArea)
	{
		GUI::MoveWindow(m_handle, newArea);
	}
}
