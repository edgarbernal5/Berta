/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "DrawBatcher.h"

#include "Berta/GUI/Window.h"
#include "Berta/Core/Foundation.h"

namespace Berta
{
	DrawBatcher::DrawBatcher(Window* window) : 
		m_rootWindow(window)
	{
		m_rootWindow->Batcher = this;
	}

	DrawBatcher::~DrawBatcher()
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		auto& rootGraphics = *(m_rootWindow->RootGraphics);
		m_rootWindow->Batcher = nullptr;

		for (auto& windowArea : m_updateRequests)
		{
			windowManager.Update(windowArea.Target);
			rootGraphics.BitBlt(windowArea.Area, windowArea.Target->Renderer.GetGraphics(), { 0,0 });
		}
		m_updateRequests.clear();
	}

	void DrawBatcher::AddWindow(Window* window, const Rectangle& areaToUpdate)
	{
		for (size_t i = 0; i < m_updateRequests.size(); i++)
		{
			if (m_updateRequests[i].Target == window)
			{
				m_updateRequests.erase(m_updateRequests.begin() + i);
				--i;
			}
		}

		m_updateRequests.emplace_back(WindowArea{ window, areaToUpdate });
	}

	bool DrawBatcher::Exists(Window* window, const Rectangle& areaToUpdate)
	{
		for (auto& windowArea : m_updateRequests)
		{
			if (windowArea.Target == window && windowArea.Area == areaToUpdate)
				return true;
		}

		return false;
	}
}