/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Renderer.h"

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/GUI/ControlRenderer.h"

namespace Berta
{
	void Renderer::Init(ControlBase& control, ControlRenderer& controlRenderer)
	{
		m_controlRenderer = &controlRenderer;
		m_controlRenderer->Init(control);
	}

	void Renderer::Map(Window* window, const Rectangle& areaToUpdate)
	{
		window->RootGraphics->Paste(window->RootHandle, areaToUpdate, areaToUpdate.X, areaToUpdate.Y);
	}

	void Renderer::Update()
	{
		if (m_controlRenderer && !m_updating)
		{
			m_updating = true;
			m_controlRenderer->Update(m_graphics);
			m_graphics.Flush();
			m_updating = false;
		}
	}

	void Renderer::MouseEnter(const ArgMouse& args)
	{
		m_controlRenderer->MouseEnter(m_graphics, args);
	}

	void Renderer::MouseLeave(const ArgMouse& args)
	{
		m_controlRenderer->MouseLeave(m_graphics, args);
	}

	void Renderer::MouseDown(const ArgMouse& args)
	{
		m_controlRenderer->MouseDown(m_graphics, args);
	}

	void Renderer::MouseMove(const ArgMouse& args)
	{
		m_controlRenderer->MouseMove(m_graphics, args);
	}

	void Renderer::MouseUp(const ArgMouse& args)
	{
		m_controlRenderer->MouseUp(m_graphics, args);
	}

	void Renderer::Click(const ArgClick& args)
	{
		m_controlRenderer->Click(m_graphics, args);
	}

	void Renderer::Focus(const ArgFocus& args)
	{
		m_controlRenderer->Focus(m_graphics, args);
	}
}