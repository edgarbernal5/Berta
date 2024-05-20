/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Renderer.h"

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/GUI/ControlReactor.h"

namespace Berta
{
	void Renderer::Init(ControlBase& control, ControlReactor& controlReactor)
	{
		m_controlReactor = &controlReactor;
		m_controlReactor->Init(control);
	}

	void Renderer::Map(Window* window, const Rectangle& areaToUpdate)
	{
		window->RootGraphics->Paste(window->RootHandle, areaToUpdate, areaToUpdate.X, areaToUpdate.Y);
	}

	void Renderer::Update()
	{
		if (m_controlReactor && !m_updating)
		{
			m_updating = true;
			m_controlReactor->Update(m_graphics);
			m_graphics.Flush();
			m_updating = false;
		}
	}

	void Renderer::MouseEnter(const ArgMouse& args)
	{
		m_controlReactor->MouseEnter(m_graphics, args);
	}

	void Renderer::MouseLeave(const ArgMouse& args)
	{
		m_controlReactor->MouseLeave(m_graphics, args);
	}

	void Renderer::MouseDown(const ArgMouse& args)
	{
		m_controlReactor->MouseDown(m_graphics, args);
	}

	void Renderer::MouseMove(const ArgMouse& args)
	{
		m_controlReactor->MouseMove(m_graphics, args);
	}

	void Renderer::MouseUp(const ArgMouse& args)
	{
		m_controlReactor->MouseUp(m_graphics, args);
	}

	void Renderer::Click(const ArgClick& args)
	{
		m_controlReactor->Click(m_graphics, args);
	}

	void Renderer::Focus(const ArgFocus& args)
	{
		m_controlReactor->Focus(m_graphics, args);
	}
	void Renderer::KeyPressed(const ArgKeyboard& args)
	{
		m_controlReactor->KeyPressed(m_graphics, args);
	}
	void Renderer::KeyReleased(const ArgKeyboard& args)
	{
		m_controlReactor->KeyReleased(m_graphics, args);
	}
}