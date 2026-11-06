/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ControlReactor.h"

#include "Control.h"
#include "Berta/Paint/Graphics.h"

namespace Berta
{
	void ControlReactor::Init(ControlBase& control, Graphics* graphics)
	{
		m_control = &control;
		m_graphics = graphics;
		DoOnInit();
	}

	void ControlReactor::Shutdown()
	{
	}

	void ControlReactor::Update(Graphics& graphics)
	{
	}

	void ControlReactor::UpdateBorders(Graphics& graphics)
	{
		auto window = m_control->Handle();

		const CornerRadii& radii = window->Appearance->BorderRadii;
		Color borderColor = window->Appearance->BoxBorderColor;
		float thickness = 1.0f;

		Rectangle localRect = { 0, 0, window->ClientSize.Width, window->ClientSize.Height };
    
		graphics.DrawRoundedRectangle(localRect, radii, borderColor, thickness);
	}

	void ControlReactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
	}

	void ControlReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
	}

	void ControlReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
	}

	void ControlReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
	}

	void ControlReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
	}

	void ControlReactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
	{
	}

	void ControlReactor::Click(Graphics& graphics, const ArgClick& args)
	{
	}

	void ControlReactor::DblClick(Graphics& graphics, const ArgMouse& args)
	{
	}

	void ControlReactor::Focus(Graphics& graphics, const ArgFocus& args)
	{
	}

	void ControlReactor::KeyChar(Graphics& graphics, const ArgKeyboard& args)
	{
	}

	void ControlReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
	}

	void ControlReactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
	{
	}

	void ControlReactor::SysChar(Graphics& graphics, const ArgSysChar& args)
	{
	}

	void ControlReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
	}

	void ControlReactor::Move(Graphics& graphics, const ArgMove& args)
	{
	}

	void ControlReactor::DpiChanged(Graphics& graphics)
	{
	}

	void ControlReactor::DoOnInit()
	{
	}
}
