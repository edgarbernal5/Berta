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
#define BT_PRINT_DRAW_BATCH_MESSAGES
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

			std::vector<BatchChildItem> childCacheItems;
			AddToCache(batchItem.Target, batchItem.Area, childCacheItems);

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

			PasteToChildren(batchItem.Target, rootGraphics, batchItem.Area, childCacheItems);

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
			{
				return true;
			}
		}

		return false;
	}

	bool DrawBatch::Exists(Window* window, DrawOperation& outOperation)
	{
		outOperation = DrawOperation::None;
		for (auto& batchItem : m_context.m_batchItemRequests)
		{
			if (batchItem.Target == window)
			{
				outOperation = batchItem.Operation;
				return true;
			}
		}

		return false;
	}

	void DrawBatch::AddToCache(Window* window, Point parentPosition, std::vector<BatchChildItem>& cache)
	{
		for (auto child : window->Children)
		{
			Rectangle childRect;
			childRect.X = parentPosition.X + child->Position.X;
			childRect.Y = parentPosition.Y + child->Position.Y;
			childRect.Width = child->ClientSize.Width;
			childRect.Height = child->ClientSize.Height;

			if (child->Type != WindowType::Panel)
				cache.emplace_back(BatchChildItem{ child , childRect });

			AddToCache(child, childRect, cache);
		}
	}

	void DrawBatch::PasteToChildren(Window* window, Graphics& rootGraphics, const Point& parentPosition, std::vector<BatchChildItem>& cache)
	{
		for (auto child : window->Children)
		{
			if (!child->Visible || child->ClientSize.IsEmpty())
				continue;

			Rectangle currentArea;

			currentArea.X = parentPosition.X + child->Position.X;
			currentArea.Y = parentPosition.Y + child->Position.Y;
			currentArea.Width = child->ClientSize.Width;
			currentArea.Height = child->ClientSize.Height;

			if (child->Type != WindowType::Panel)
			{
				Rectangle oldArea;
				GetOldAreaFromCache(oldArea, cache, child);

				DrawOperation existingOperation;
				bool existsInBatch = Exists(child, existingOperation);
				if (oldArea != currentArea || !existsInBatch)
				{
					//if ((!existsInBatch || !HasFlag(existingOperation, DrawOperation::NeedUpdate)) && !child->Flags.isUpdating)
					if (!child->Flags.isUpdating)
					{
						child->Flags.isUpdating = true;
						child->Renderer.Update();
						child->Flags.isUpdating = false;
					}

					if (!existsInBatch)
					{
						rootGraphics.BitBlt(currentArea, child->Renderer.GetGraphics(), { 0,0 });
					}
					else
					{
						Update(child, currentArea, DrawOperation::NeedMap);
					}
				}
				else if (existsInBatch)
				{
					if (!HasFlag(existingOperation, DrawOperation::NeedUpdate))
					{
						if (!child->Flags.isUpdating)
						{
							child->Flags.isUpdating = true;
							child->Renderer.Update();
							child->Flags.isUpdating = false;
						}

						if (!HasFlag(existingOperation, DrawOperation::NeedMap))
						{
							rootGraphics.BitBlt(currentArea, child->Renderer.GetGraphics(), { 0,0 });
						}
					}
				}
			}

			PasteToChildren(child, rootGraphics, currentArea, cache);
		}
	}

	bool DrawBatch::GetOldAreaFromCache(Rectangle& oldArea, std::vector<BatchChildItem>& cache, Window* child)
	{
		bool found = false;
		for (size_t i = 0; i < cache.size(); i++)
		{
			if (cache[i].Target == child)
			{
				found = true;
				oldArea = cache[i].Area;
				break;
			}
		}
		return found;
	}

	void DrawBatch::Update(Window* window, const Rectangle& newArea)
	{
		for (auto& batchItem : m_context.m_batchItemRequests)
		{
			if (batchItem.Target == window)
			{
				batchItem.Area = newArea;
			}
		}
	}

	void DrawBatch::Update(Window* window, const Rectangle& newArea, const DrawOperation& newOperation)
	{
		for (auto& batchItem : m_context.m_batchItemRequests)
		{
			if (batchItem.Target == window)
			{
				batchItem.Area = newArea;
				batchItem.Operation = newOperation;
			}
		}
	}

	bool BatchItemComparer::operator()(BatchItem a, BatchItem b) const
	{
		return a.Index < b.Index;
	}
}