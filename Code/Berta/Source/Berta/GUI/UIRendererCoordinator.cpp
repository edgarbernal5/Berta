/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "UIRendererCoordinator.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void UIRendererCoordinator::Paint(Window* window, PaintOperation operation, bool processChildren)
	{
		if (window->Flags.isUpdating && operation == PaintOperation::TryUpdate)
			return;

		if (operation == PaintOperation::TryUpdate && window->Renderer.GetGraphics().IsValid())
		{
			if (window->IsBatchActive())
			{
				TryAddWindowToBatch(window, DrawOperation::NeedUpdate);
			}
			else
			{
				window->Flags.isUpdating = true;
				window->Renderer.Update();
				window->Flags.isUpdating = false;
			}
		}
		Map(window, operation != PaintOperation::None, processChildren);

		//
		////if (window->Type == WindowType::RenderForm && window->CustomPaint)
		////{
		////	API::RefreshWindow(window->RootHandle);
		////	return;
		////}

		//auto& rootGraphics = *(window->RootGraphics);

		//Rectangle requestRectangle = window->ClientSize.ToRectangle();
		//auto absolutePosition = GetAbsoluteRootPosition(window);
		//requestRectangle.X = absolutePosition.X;
		//requestRectangle.Y = absolutePosition.Y;

		//auto container = window->FindFirstPanelOrFormAncestor();
		//auto containerPosition = GetAbsoluteRootPosition(container);
		//Rectangle containerRectangle{ containerPosition.X, containerPosition.Y, container->ClientSize.Width, container->ClientSize.Height };

		//if (now || !window->IsBatchActive())
		//{
		//	if (!window->Flags.isUpdating)
		//	{
		//		window->Flags.isUpdating = true;
		//		window->Renderer.Update();
		//		window->Flags.isUpdating = false;
		//		window->DrawStatus = DrawWindowStatus::Updated;
		//	}
		//	rootGraphics.Begin();
		//	if (LayoutUtils::GetIntersectionRect(containerRectangle, requestRectangle, requestRectangle))
		//	{
		//		rootGraphics.BitBlt(requestRectangle, window->Renderer.GetGraphics(), { 0,0 });
		//	}
		//}
		//else
		//{
		//	if (LayoutUtils::GetIntersectionRect(containerRectangle, requestRectangle, requestRectangle))
		//	{
		//		AddWindowToBatch(window, requestRectangle, DrawOperation::NeedUpdate | DrawOperation::NeedMap);
		//	}
		//}

		//UpdateTreeInternal(window, rootGraphics, now, absolutePosition, containerRectangle);

		//if (now || !window->IsBatchActive())
		//{
		//	rootGraphics.Flush();
		//}
	}

	void UIRendererCoordinator::Map(Window* window, bool haveUpdated, bool processChildren)
	{
		auto checkOpaque = window->FindFirstNonPanelAncestor();
		if (checkOpaque && checkOpaque->Flags.isUpdating)
			return;

		Rectangle rect;
		if (!GetIntersectionRect(window, rect))
			return;

		if (window->IsBatchActive())
		{
			if (haveUpdated)
			{
				TryAddWindowToBatch(window, DrawOperation::NeedMap);
				TryAddWindowToBatchInternal(window, DrawOperation::NeedMap, haveUpdated, processChildren, rect);
			}
		}
		else
		{
			auto& rootGraphics = *(window->RootGraphics);
			rootGraphics.Begin();
			if (window->Type != WindowType::Panel)
			{
				rootGraphics.BitBlt(rect, window->Renderer.GetGraphics(), { 0,0 });
			}
			MapInternal(window, haveUpdated, processChildren, rect, rootGraphics);
			rootGraphics.Flush();
		}
	}

	bool UIRendererCoordinator::GetIntersectionRect(Window* window, Rectangle& result)
	{
		Rectangle requestRectangle = window->ClientSize.ToRectangle();
		auto absolutePosition = GUI::GetAbsoluteRootPosition(window);
		requestRectangle.X = absolutePosition.X;
		requestRectangle.Y = absolutePosition.Y;

		auto container = window->FindFirstPanelOrFormAncestor();
		auto containerPosition = GUI::GetAbsoluteRootPosition(container);
		Rectangle containerRectangle{ containerPosition.X, containerPosition.Y, container->ClientSize.Width, container->ClientSize.Height };

		return LayoutUtils::GetIntersectionRect(containerRectangle, requestRectangle, result);
	}

	void UIRendererCoordinator::TryAddWindowToBatch(Window* window, const DrawOperation& operation)
	{
		Rectangle requestRectangle = window->ClientSize.ToRectangle();
		auto absolutePosition = GUI::GetAbsoluteRootPosition(window);
		requestRectangle.X = absolutePosition.X;
		requestRectangle.Y = absolutePosition.Y;

		auto container = window->FindFirstPanelOrFormAncestor();
		auto containerPosition = GUI::GetAbsoluteRootPosition(container);
		Rectangle containerRectangle{ containerPosition.X, containerPosition.Y, container->ClientSize.Width, container->ClientSize.Height };
		Rectangle output;
		if (LayoutUtils::GetIntersectionRect(containerRectangle, requestRectangle, output))
		{
			AddWindowToBatch(window, requestRectangle, operation);
		}
	}

	void UIRendererCoordinator::TryAddWindowToBatchInternal(Window* window, const DrawOperation& operation, bool haveUpdated, bool processChildren, const Rectangle& parentRect)
	{
		for (size_t i = 0; i < window->Children.size(); i++)
		{
			auto child = window->Children[i];
			if (!child->Visible || (!child->Renderer.GetGraphics().IsValid()) && child->Type != WindowType::Panel)
			{
				continue;
			}

			Rectangle rect;
			Rectangle childRect = parentRect;
			childRect.X += child->Position.X;
			childRect.Y += child->Position.Y;
			childRect.Width = child->ClientSize.Width;
			childRect.Height = child->ClientSize.Height;
			if (LayoutUtils::GetIntersectionRect(childRect, parentRect, rect))
			{
				if (child->Type != WindowType::Panel)
				{
					AddWindowToBatch(child, rect, haveUpdated ? (operation | DrawOperation::NeedUpdate) : operation);
				}
				TryAddWindowToBatchInternal(child, operation, processChildren, processChildren, rect);
			}
		}
	}

	void UIRendererCoordinator::MapInternal(Window* window, bool haveUpdated, bool processChildren, const Rectangle& parentRect, Graphics& rootGraphics)
	{
		for (size_t i = 0; i < window->Children.size(); i++)
		{
			auto child = window->Children[i];
			if (!child->Visible || (!child->Renderer.GetGraphics().IsValid()) && child->Type != WindowType::Panel)
			{
				continue;
			}

			Rectangle rect;
			Rectangle childRect = parentRect;
			childRect.X += child->Position.X;
			childRect.Y += child->Position.Y;
			childRect.Width = child->ClientSize.Width;
			childRect.Height = child->ClientSize.Height;
			if (LayoutUtils::GetIntersectionRect(childRect, parentRect, rect))
			{
				if (child->Type != WindowType::Panel)
				{
					if (processChildren && !child->Flags.isUpdating)
					{
						child->Flags.isUpdating = true;
						child->Renderer.Update();
						child->Flags.isUpdating = false;
					}
					rootGraphics.BitBlt(rect, child->Renderer.GetGraphics(), { 0,0 });
				}
				MapInternal(child, processChildren, processChildren, rect, rootGraphics);
			}
		}
	}

	void UIRendererCoordinator::AddWindowToBatch(Window* window, const Rectangle& areaToUpdate, const DrawOperation& operation)
	{
		if (!window->RootWindow->Batcher)
			return;

		//if (window->Type == WindowType::RenderForm)
		//{
		//	return;
		//}
		AddWindowToBatch(window->RootWindow->Batcher, window, areaToUpdate, operation);
	}

	void UIRendererCoordinator::AddWindowToBatch(DrawBatch* batch, Window* window, const Rectangle& areaToUpdate, const DrawOperation& operation)
	{
		if (batch->Exists(window, areaToUpdate, operation))
			return;

		batch->AddWindow(window, areaToUpdate, operation);
	}
}
