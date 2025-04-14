/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "DrawBatch.h"

#include "Berta/GUI/Window.h"
#include "Berta/Core/Foundation.h"

namespace Berta
{
	std::unordered_map< Window*, DrawBatcherContext> DrawBatch::g_contexts;

	DrawBatch::DrawBatch(Window* rootWindow) :
		m_context(g_contexts[rootWindow->RootWindow])
	{
		////std::cout << ">> START.... \twindow = " << rootWindow->Name << std::endl;
		if (m_context.m_rootWindow)
			return;

		m_context.m_rootWindow = rootWindow->RootWindow;
		rootWindow->RootWindow->Batcher = this;
	}

	DrawBatch::~DrawBatch()
	{
		//std::cout << ">> END.... \twindow = " << m_context.m_rootWindow->Name << std::endl;
		if (!m_context.m_rootWindow || m_context.m_rootWindow->Flags.IsDisposed)
			return;

		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		auto& rootGraphics = *(m_context.m_rootWindow->RootGraphics);

		if (m_context.m_rootWindow->Flags.IsDeferredCount > 0)
			return;

		m_context.m_rootWindow->RootWindow->Batcher = nullptr;

		if (m_context.m_updateRequests.empty())
		{
			m_context.m_rootWindow = nullptr;
			return;
		}

		for (auto& windowAreaIndex : m_context.m_updateRequests)
		{
			windowAreaIndex.Index = windowAreaIndex.Target->GetHierarchyIndex();
		}

		WindowAreaBatchComparer comparer;
		std::sort(m_context.m_updateRequests.begin(), m_context.m_updateRequests.end(), comparer);

		bool fullMap = !m_context.m_updateRequests.empty() && m_context.m_updateRequests[0].Target->Type == WindowType::Form;
		for (auto& windowArea : m_context.m_updateRequests)
		{
			if (windowArea.Target->Flags.IsDisposed)
				continue;

			if (windowArea.Target->Status == WindowStatus::None && !windowArea.Target->Flags.isUpdating)
			{
				windowArea.Target->Flags.isUpdating = true;
				windowArea.Target->Renderer.Update();
				windowArea.Target->Flags.isUpdating = false;
			}
			rootGraphics.BitBlt(windowArea.Area, windowArea.Target->Renderer.GetGraphics(), { 0,0 });

			if (!fullMap)
			{
				m_context.m_rootWindow->Renderer.Map(m_context.m_rootWindow, windowArea.Area);
			}
			windowArea.Target->Status = WindowStatus::None;
		}

		if (fullMap)
		{
			m_context.m_rootWindow->Renderer.Map(m_context.m_rootWindow, m_context.m_rootWindow->ClientSize.ToRectangle());
		}

		m_context.m_rootWindow = nullptr;
		m_context.m_updateRequests.clear();
	}

	void DrawBatch::Clear()
	{
		m_context.m_updateRequests.clear();
	}

	void DrawBatch::AddWindow(Window* window, const Rectangle& areaToUpdate)
	{
		for (size_t i = 0; i < m_context.m_updateRequests.size(); i++)
		{
			if (m_context.m_updateRequests[i].Target == window)
			{
				m_context.m_updateRequests[i].Area = areaToUpdate;
				return;
			}
		}

		m_context.m_updateRequests.emplace_back(WindowAreaBatch{ window, areaToUpdate });
	}

	bool DrawBatch::Exists(Window* window, const Rectangle& areaToUpdate)
	{
		for (auto& windowArea : m_context.m_updateRequests)
		{
			if (windowArea.Target == window && windowArea.Area == areaToUpdate)
				return true;
		}

		return false;
	}

	bool WindowAreaBatchComparer::operator()(WindowAreaBatch a, WindowAreaBatch b) const
	{
		return a.Index < b.Index;
	}
}