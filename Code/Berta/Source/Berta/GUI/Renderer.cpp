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
		m_control = &control;
		m_graphics = control.Handle()->RootGraphics;

		m_controlReactor = &controlReactor;
		m_controlReactor->Init(control, m_graphics);
	}

	void Renderer::Shutdown()
	{
		if (m_controlReactor)
		{
			m_controlReactor->Shutdown();
			m_controlReactor = nullptr;
		}
		m_control = nullptr;
		m_graphics = nullptr;
	}

	void Renderer::Map(Window* window, const Rectangle& areaToUpdate)
	{
		if (window->HasCustomPaint())
			return;

		window->RootGraphics->Paste(window->RootPaintHandle, areaToUpdate, areaToUpdate.X, areaToUpdate.Y);
	}

	void Renderer::Update(const Rectangle& clipRect)
	{
		BT_ASSERT(!m_controlReactor || !m_updating, "Renderer Update is already updating.");

		if (m_controlReactor && !m_updating && m_graphics->IsValid())
		{
			m_updating = true;
			//asumimos que el rootgraphics hizo el begindraw
			auto absoluteArea = m_control->GetArea();
			m_graphics->SetTransform(absoluteArea);

			/*if (!m_control->IsBorderless())
			{
				Rectangle localBorderRect = { 0, 0, absoluteArea.Width, absoluteArea.Height };
				m_graphics->DrawRectangle(localBorderRect, m_control->Handle()->Appearance->BoxBorderColor, false);
			}
			
			Rectangle controlClipRect = clipRect;
			if (!m_control->IsBorderless())
			{
				controlClipRect.X += 1;
				controlClipRect.Y += 1;
				controlClipRect.Height -= 2u;
				controlClipRect.Width -= 2u;
			}*/
			Rectangle relativeClipRect = clipRect;
			relativeClipRect.X -= absoluteArea.X;
			relativeClipRect.Y -= absoluteArea.Y;
			
			m_graphics->SetClipping(relativeClipRect);
			m_controlReactor->Update(*m_graphics);
			m_graphics->EndClipping();

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

	void Renderer::Move(const ArgMove& args)
	{
		ProcessEvent(&ControlReactor::Move, args);
	}

	void Renderer::DpiChanged()
	{
		ProcessEvent(&ControlReactor::DpiChanged);
	}

	void Renderer::SetGraphics(Graphics* newGraphics)
	{
		m_graphics = newGraphics;
	}

	void Renderer::ProcessEvent(void(ControlReactor::* reactorEventPtr)(Graphics&))
	{
		if (m_controlReactor == nullptr) //Added this check due to panels that don't have either reactor or graphics.
		{
			return;
		}

		(m_controlReactor->*reactorEventPtr)(*m_graphics);
	}
}
