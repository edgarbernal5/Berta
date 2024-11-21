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
		m_size(size),
		m_timer(owner)
	{
		m_timer.Connect([this](const ArgTimer& args)
		{
			Show(!m_visible);
		});

		m_timer.SetInterval(600);
	}

	Caret::~Caret()
	{
		m_timer.Stop();
	}

	void Caret::Activate()
	{
		m_visible = true;
		m_timer.Start();

		GUI::UpdateWindow(m_owner);
	}

	void Caret::Show(bool visible)
	{
		m_visible = visible;

		GUI::UpdateWindow(m_owner);
	}

	void Caret::Deactivate()
	{
		m_visible = false;

		GUI::UpdateWindow(m_owner);

		m_timer.Stop();
	}
}