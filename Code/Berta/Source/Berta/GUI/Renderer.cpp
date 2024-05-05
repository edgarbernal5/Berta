/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Renderer.h"

#include "Berta/GUI/BasicWindow.h"
#include "Berta/Core/Widget.h"
#include "Berta/Core/WidgetRenderer.h"

namespace Berta
{
	void Renderer::Init(WidgetBase& widget, WidgetRenderer& widgetRenderer)
	{
		m_widgetRenderer = &widgetRenderer;
		m_widgetRenderer->Init(widget);
	}

	void Renderer::Map(BasicWindow* window, const Rectangle& areaToUpdate)
	{
		window->RootGraphics->Paste(window->Root, areaToUpdate, 0, 0);
	}

	void Renderer::Update()
	{
		if (m_widgetRenderer && !m_updating)
		{
			m_updating = true;
			m_widgetRenderer->Update(m_graphics);
			m_graphics.Flush();
			m_updating = false;
		}
	}
}