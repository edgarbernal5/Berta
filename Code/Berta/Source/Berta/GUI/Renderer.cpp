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

	void Renderer::Shutdown()
	{
		if (m_controlReactor)
		{
			m_controlReactor->Shutdown();
			m_controlReactor = nullptr;
		}
	}

	void Renderer::Map(Window* window, const Rectangle& areaToUpdate)
	{
		window->RootGraphics->Paste(window->RootHandle, areaToUpdate, areaToUpdate.X, areaToUpdate.Y);
	}

	void Renderer::Update()
	{
		if (m_controlReactor && m_updating)
		{
			BT_CORE_TRACE << "Renderer::Update(). it is already updating... " << std::endl;
		}

		if (m_controlReactor && !m_updating && m_graphics.IsValid())
		{
			m_updating = true;
			m_controlReactor->Update(m_graphics);
			m_graphics.Flush();
			m_updating = false;
		}
	}

	void Renderer::MouseEnter(const ArgMouse& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->MouseEnter(m_graphics, args);
	}

	void Renderer::MouseLeave(const ArgMouse& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->MouseLeave(m_graphics, args);
	}

	void Renderer::MouseDown(const ArgMouse& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->MouseDown(m_graphics, args);
	}

	void Renderer::MouseMove(const ArgMouse& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->MouseMove(m_graphics, args);
	}

	void Renderer::MouseUp(const ArgMouse& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->MouseUp(m_graphics, args);
	}

	void Renderer::MouseWheel(const ArgWheel& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->MouseWheel(m_graphics, args);
	}

	void Renderer::Click(const ArgClick& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->Click(m_graphics, args);
	}

	void Renderer::DblClick(const ArgClick& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->DblClick(m_graphics, args);
	}

	void Renderer::Focus(const ArgFocus& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->Focus(m_graphics, args);
	}

	void Renderer::KeyChar(const ArgKeyboard& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->KeyChar(m_graphics, args);
	}

	void Renderer::KeyPressed(const ArgKeyboard& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->KeyPressed(m_graphics, args);
	}

	void Renderer::KeyReleased(const ArgKeyboard& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->KeyReleased(m_graphics, args);
	}

	void Renderer::Resize(const ArgResize& args)
	{
		if (m_controlReactor == nullptr)
			return;

		m_controlReactor->Resize(m_graphics, args);
	}
}