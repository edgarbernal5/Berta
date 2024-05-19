/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Caret.h"

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Interface.h"

namespace Berta
{
	Caret::Caret(Window* owner, const Size& size) :
		m_owner(owner),
		m_size(size)
	{
		m_timer.GetTickEvent().Connect([this](const ArgTimer& args)
		{
			Show(!m_visible);
		});
	}

	void Caret::Activate()
	{
		m_timer.Start();
	}

	void Caret::Show(bool visible)
	{
		m_visible = visible;

		m_owner->Renderer.Update();
		GUI::RefreshWindow(m_owner);

	}

	void Caret::Deactivate()
	{
	}
}