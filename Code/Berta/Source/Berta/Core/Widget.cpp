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
	void Widget::Show()
	{
		GUI::ShowBasicWindow(m_handle, true);
	}

	BasicWindow* Widget::Handle() const
	{
		return m_handle;
	}
}
