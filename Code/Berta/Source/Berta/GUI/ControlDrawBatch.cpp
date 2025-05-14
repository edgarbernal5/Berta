/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ControlDrawBatch.h"

namespace Berta
{
	ControlDrawBatch::ControlDrawBatch(ControlBase& control) : 
		m_control(control)
	{
		m_control.SetAutoDraw(false);
	}

	ControlDrawBatch::~ControlDrawBatch()
	{
		m_control.SetAutoDraw(true);
		GUI::UpdateWindow(m_control.Handle());
	}
}