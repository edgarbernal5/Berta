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

		if (window->Type == WindowType::RenderForm)
			return;

		auto& rootGraphics = *(window->RootGraphics);
		rootGraphics.Begin();
		if (window->Type != WindowType::Panel && operation == PaintOperation::TryUpdate && window->Renderer.GetGraphics().IsValid())
		{
			window->Flags.isUpdating = true;
			window->Renderer.Update(window->ClientSize.ToRectangle());
			window->Flags.isUpdating = false;
		}
		Map(window, operation != PaintOperation::None, processChildren);
		rootGraphics.Flush();
	}

	void UIRendererCoordinator::Map(Window* window, bool haveUpdated, bool processChildren)
	{
		auto checkOpaque = window->FindFirstNonPanelAncestor();
		if (checkOpaque && checkOpaque->Flags.isUpdating)
		{
			return;
		}

		if (window->Type == WindowType::RenderForm)
		{
			return;
		}
		
		Rectangle rect;
		if (!GetIntersectionRect(window, rect))
			return;

		MapInternal(window, processChildren, rect);
	}

	bool UIRendererCoordinator::GetIntersectionRect(Window* window, Rectangle& result)
	{
		Rectangle requestRectangle = window->ClientSize.ToRectangle();
		auto absolutePosition = GUI::GetWindowRootPosition(window);
		requestRectangle.X = absolutePosition.X;
		requestRectangle.Y = absolutePosition.Y;

		auto container = window->FindFirstPanelOrFormAncestor();
		auto containerPosition = GUI::GetWindowRootPosition(container);
		Rectangle containerRectangle{ containerPosition.X, containerPosition.Y, container->ClientSize.Width, container->ClientSize.Height };

		return LayoutUtils::GetIntersectionRect(containerRectangle, requestRectangle, result);
	}

	void UIRendererCoordinator::MapInternal(Window* window, bool processChildren, const Rectangle& parentRect)
	{
		for (size_t i = 0; i < window->Children.size(); i++)
		{
			auto child = window->Children[i];
			if (!child->Visible || (child->Type != WindowType::Panel && !child->Renderer.GetGraphics().IsValid()))
			{
				continue;
			}

			if (child->IsNative())
			{
				Paint(child, (processChildren ? PaintOperation::TryUpdate : PaintOperation::None), processChildren);
				continue;
			}

			Rectangle childRect = parentRect;
			childRect.X += child->Position.X;
			childRect.Y += child->Position.Y;
			childRect.Width = child->ClientSize.Width;
			childRect.Height = child->ClientSize.Height;
			
			Rectangle clipRect;
			if (LayoutUtils::GetIntersectionRect(childRect, parentRect, clipRect))
			{
				if (child->Type != WindowType::Panel && processChildren && !child->Flags.isUpdating)
				{
					child->Flags.isUpdating = true;
					child->Renderer.Update(clipRect);
					child->Flags.isUpdating = false;
				}
				MapInternal(child, processChildren, clipRect);
			}
		}
	}
}
