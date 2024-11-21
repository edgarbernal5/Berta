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
			BT_CORE_WARN << " - Renderer::Update(). it is already updating... " << std::endl;
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
		ProcessEvent(&ControlReactor::MouseEnter, args);
	}

	void Renderer::MouseLeave(const ArgMouse& args)
	{
		ProcessEvent(&ControlReactor::MouseLeave, args);
	}

	void Renderer::MouseDown(const ArgMouse& args)
	{
		ProcessEvent(&ControlReactor::MouseDown, args);
	}

	void Renderer::MouseMove(const ArgMouse& args)
	{
		ProcessEvent(&ControlReactor::MouseMove, args);
	}

	void Renderer::MouseUp(const ArgMouse& args)
	{
		ProcessEvent(&ControlReactor::MouseUp, args);
	}

	void Renderer::MouseWheel(const ArgWheel& args)
	{
		ProcessEvent(&ControlReactor::MouseWheel, args);
	}

	void Renderer::Click(const ArgClick& args)
	{
		ProcessEvent(&ControlReactor::Click, args);
	}

	void Renderer::DblClick(const ArgMouse& args)
	{
		ProcessEvent(&ControlReactor::DblClick, args);
	}

	void Renderer::Focus(const ArgFocus& args)
	{
		ProcessEvent(&ControlReactor::Focus, args);
	}

	void Renderer::KeyChar(const ArgKeyboard& args)
	{
		ProcessEvent(&ControlReactor::KeyChar, args);
	}

	void Renderer::KeyPressed(const ArgKeyboard& args)
	{
		ProcessEvent(&ControlReactor::KeyPressed, args);
	}

	void Renderer::KeyReleased(const ArgKeyboard& args)
	{
		ProcessEvent(&ControlReactor::KeyReleased, args);
	}

	void Renderer::Resize(const ArgResize& args)
	{
		ProcessEvent(&ControlReactor::Resize, args);
	}
}