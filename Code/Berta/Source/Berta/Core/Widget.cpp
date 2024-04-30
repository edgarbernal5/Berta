/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Widget.h"

#include "Berta/GUI/BasicWindow.h"
#include "Berta/GUI/Interface.h"

namespace Berta
{
	void Widget::Caption(const std::wstring& caption)
	{
		GUI::CaptionWindow(m_handle, caption);
	}

	void Widget::Create(const Rectangle& rectangle)
	{
		m_handle = GUI::CreateWidget(rectangle);
	}

	void Widget::Create(const Rectangle& rectangle, const WindowStyle& windowStyle)
	{
		m_handle = GUI::CreateBasicWindow(rectangle, windowStyle);
	}

	void Widget::Show()
	{
		GUI::ShowBasicWindow(m_handle, true);
	}
}
