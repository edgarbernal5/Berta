/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "DrawBatch.h"

#include "Berta/GUI/Window.h"
#include "Berta/Core/Foundation.h"

#if BT_DEBUG
#ifndef BT_PRINT_DRAW_BATCH_MESSAGES
//#define BT_PRINT_DRAW_BATCH_MESSAGES
#endif // !BT_PRINT_DRAW_BATCH_MESSAGES
#endif

namespace Berta
{
	std::unordered_map< Window*, DrawBatchContext> DrawBatch::g_contexts;

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

		if (m_context.m_batchItemRequests.empty())
		{
			m_context.m_rootWindow = nullptr;
			return;
		}

		for (auto& batchItem : m_context.m_batchItemRequests)
		{
			batchItem.Index = batchItem.Target->GetHierarchyIndex();
		}

		BatchItemComparer comparer;
		std::sort(m_context.m_batchItemRequests.begin(), m_context.m_batchItemRequests.end(), comparer);

#ifdef BT_PRINT_DRAW_BATCH_MESSAGES
		BT_CORE_TRACE << "Draw Batch size = " << m_context.m_batchItemRequests.size() << ". root window=" << m_context.m_rootWindow->Name << std::endl;
		for (size_t i = 0; i < m_context.m_batchItemRequests.size(); i++)
		{
			auto& item = m_context.m_batchItemRequests[i];
			BT_CORE_TRACE << "  - batch item = " << item.Target->Name << ". flags="  << (uint32_t)item.Operation << std::endl;
		}
		std::cout << std::endl;
#endif // BT_PRINT_DRAW_BATCH_MESSAGES

		rootGraphics.Begin();
		bool fullMap = !m_context.m_batchItemRequests.empty() && m_context.m_batchItemRequests[0].Target->Type == WindowType::Form;
		for (auto& batchItem : m_context.m_batchItemRequests)
		{
			if (batchItem.Target->Flags.IsDisposed)
				continue;

			if (HasFlag(batchItem.Operation, DrawOperation::NeedUpdate) && !batchItem.Target->Flags.isUpdating)
			{
				batchItem.Target->Flags.isUpdating = true;
				batchItem.Target->Renderer.Update();
				batchItem.Target->Flags.isUpdating = false;
			}

			if (HasFlag(batchItem.Operation, DrawOperation::NeedMap))
			{
				rootGraphics.BitBlt(batchItem.Area, batchItem.Target->Renderer.GetGraphics(), { 0,0 });
			}

			//if there are children, paste it too!

			batchItem.Target->DrawStatus = DrawWindowStatus::None;
		}
		rootGraphics.Flush();

		if (fullMap)
		{
			m_context.m_rootWindow->Renderer.Map(m_context.m_rootWindow, m_context.m_rootWindow->ClientSize.ToRectangle());
		}

		//TODO: Remove this! or put an ASSERT
		for (size_t i = 0; i < m_context.m_batchItemRequests.size(); i++)
		{
			for (size_t j = i + 1; j < m_context.m_batchItemRequests.size(); j++)
			{
				if (m_context.m_batchItemRequests[i].Target == m_context.m_batchItemRequests[j].Target) {
					break;
				}
			}
		}

		for (auto& batchItem : m_context.m_batchItemRequests)
		{
			if (!fullMap)
			{
				m_context.m_rootWindow->Renderer.Map(m_context.m_rootWindow, batchItem.Area);
			}
			if (HasFlag(batchItem.Operation, DrawOperation::Refresh))
			{
				API::RefreshWindow(batchItem.Target->RootHandle);
			}
		}

		m_context.m_rootWindow = nullptr;
		m_context.m_batchItemRequests.clear();
	}

	void DrawBatch::Clear()
	{
		m_context.m_batchItemRequests.clear();
	}

	void DrawBatch::AddWindow(Window* window, const Rectangle& areaToUpdate, const DrawOperation& operation)
	{
		for (auto& batchItem : m_context.m_batchItemRequests)
		{
			if (batchItem.Target == window)
			{
				batchItem.Area = areaToUpdate;
				batchItem.Operation = batchItem.Operation | operation;
				return;
			}
		}

		m_context.m_batchItemRequests.emplace_back(BatchItem{ window, areaToUpdate, operation });
	}

	bool DrawBatch::Exists(Window* window, const Rectangle& areaToUpdate, const DrawOperation& operation)
	{
		for (auto& batchItem : m_context.m_batchItemRequests)
		{
			if (batchItem.Target == window && batchItem.Area == areaToUpdate && batchItem.Operation == operation)
				return true;
		}

		return false;
	}

	bool BatchItemComparer::operator()(BatchItem a, BatchItem b) const
	{
		return a.Index < b.Index;
	}
}