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
		Flush();
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
				if (HasFlag(operation, DrawOperation::MoveSize))
				{
					if (areaToUpdate.Width == 0 && areaToUpdate.Height == 0)
					{
						batchItem.Area.X = areaToUpdate.X;
						batchItem.Area.Y = areaToUpdate.Y;
					}
					else
					{
						batchItem.Area = areaToUpdate;
					}
				}
				else
				{
					batchItem.Area = areaToUpdate;
				}
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

	void DrawBatch::Flush(bool forceManual)
	{
		//std::cout << ">> END.... \twindow = " << m_context.m_rootWindow->Name << std::endl;
		if (!m_context.m_rootWindow || m_context.m_rootWindow->Flags.IsDisposed)
		{
			m_context.m_rootWindow = nullptr;
			return;
		}

		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		auto& rootGraphics = *(m_context.m_rootWindow->RootGraphics);

		if (m_context.m_rootWindow->Flags.IsDeferredCount > 0 && !forceManual)
			return;

		if (!forceManual)
			m_context.m_rootWindow->RootWindow->Batcher = nullptr;

		if (m_context.m_batchItemRequests.empty())
		{
			if(!forceManual)
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
		BT_CORE_TRACE << "- Draw Batch size = " << m_context.m_batchItemRequests.size() << ". root window=" << m_context.m_rootWindow->Name << std::endl;
		/*for (size_t i = 0; i < m_context.m_batchItemRequests.size(); i++)
		{
			auto& item = m_context.m_batchItemRequests[i];
			BT_CORE_TRACE << "  - batch item = " << item.Target->Name << ". flags="  << (uint32_t)item.Operation << std::endl;
		}*/
		std::cout << std::endl;
#endif // BT_PRINT_DRAW_BATCH_MESSAGES

		m_context.m_rootWindow->Flags.isBatching = true;

		if (m_context.m_rootWindow->Type != WindowType::RenderForm)
		{
			std::vector<BatchChildItem> childCacheItems;
			AddToCache(m_context.m_batchItemRequests[0].Target, m_context.m_batchItemRequests[0].Area, childCacheItems);

			rootGraphics.Begin();
			bool fullMap = !m_context.m_batchItemRequests.empty() && m_context.m_batchItemRequests[0].Target->Type == WindowType::Form;
			for (auto& batchItem : m_context.m_batchItemRequests)
			{
				if (batchItem.Target->Flags.IsDisposed)
					continue;
				/*if (batchItem.Target->IsNative() && batchItem.Target->Type==WindowType::RenderForm)
					continue;*/

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

			}

			for (auto& batchItem : m_context.m_batchItemRequests)
			{
				if (HasFlag(batchItem.Operation, DrawOperation::MoveSize))
				{
					if (batchItem.Area.Width == 0 && batchItem.Area.Height == 0)
					{
						BT_CORE_TRACE << " hegiht width =0" << ". root window=" << m_context.m_rootWindow->Name << std::endl;
						API::MoveWindow(batchItem.Target->RootHandle, Point{ batchItem.Area.X,  batchItem.Area.Y });
					}
					else
					{
						BT_CORE_TRACE << " area completa" << ". root window=" << m_context.m_rootWindow->Name << std::endl;
						API::MoveWindow(batchItem.Target->RootHandle, batchItem.Area);
					}
					//API::RefreshWindow(batchItem.Target->RootHandle);
				}
				if (HasFlag(batchItem.Operation, DrawOperation::Refresh))
				{
					API::RefreshWindow(batchItem.Target->RootHandle);
				}
			}
		}
		
		if (!m_context.m_rootWindow)
			return;
#ifdef BT_PRINT_DRAW_BATCH_MESSAGES
		BT_CORE_TRACE << "- End Draw Batch size = " << m_context.m_batchItemRequests.size() << ". root window=" << m_context.m_rootWindow->Name << std::endl;
		std::cout << std::endl;
#endif // BT_PRINT_DRAW_BATCH_MESSAGES
		m_context.m_rootWindow->Flags.isBatching = false;

		if (!forceManual)
			m_context.m_rootWindow = nullptr;

		m_context.m_batchItemRequests.clear();
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
					bool needRefresh = oldArea.Width != currentArea.Width || oldArea.Height != currentArea.Height;

					if (child->IsNative())
					{
						if (needRefresh)
						{
							API::RefreshWindow(child->RootHandle);
						}
						continue;
					}
					//if ((!existsInBatch || !HasFlag(existingOperation, DrawOperation::NeedUpdate)) && !child->Flags.isUpdating)
					if (needRefresh && !child->Flags.isUpdating)
					{
						child->Flags.isUpdating = true;
						child->Renderer.Update();
						child->Flags.isUpdating = false;
					}

					if (!existsInBatch)
					{
						rootGraphics.BitBlt(currentArea, child->Renderer.GetGraphics(), { 0,0 });
					}
					else if (needRefresh)
					{
						Update(child, currentArea, DrawOperation::NeedMap);
					}
				}
				else if (existsInBatch && !child->IsNative())
				{
					if (!HasFlag(existingOperation, DrawOperation::NeedMap))
					{
						rootGraphics.BitBlt(currentArea, child->Renderer.GetGraphics(), { 0,0 });
					}
				}
				if (child->IsNative())
				{
					continue;
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
				break;
			}
		}
	}

	bool BatchItemComparer::operator()(BatchItem a, BatchItem b) const
	{
		return a.Index < b.Index;
	}
}