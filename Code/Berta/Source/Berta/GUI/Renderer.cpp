/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Renderer.h"

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Widget.h"
#include "Berta/GUI/WidgetRenderer.h"

namespace Berta
{
	void Renderer::Init(WidgetBase& widget, WidgetRenderer& widgetRenderer)
	{
		m_widgetRenderer = &widgetRenderer;
		m_widgetRenderer->Init(widget);
	}

	void Renderer::Map(Window* window, const Rectangle& areaToUpdate)
	{
		window->RootGraphics->Paste(window->Root, areaToUpdate, areaToUpdate.X, areaToUpdate.Y);
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