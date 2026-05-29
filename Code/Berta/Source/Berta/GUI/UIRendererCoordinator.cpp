/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "UIRendererCoordinator.h"

#include "Berta/GUI/ScopedClip.h"
#include "Berta/GUI/Interface.h"

namespace Berta
{
	struct ScopedUpdatingFlag
	{
		Window* window;

		ScopedUpdatingFlag(Window* w) : window(w)
		{ 
			if (window) window->Flags.isUpdating = true; 
		}

		~ScopedUpdatingFlag()
		{ 
			if (window) window->Flags.isUpdating = false; 
		}
	};
	
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
			ScopedUpdatingFlag guard(window);
			
			window->Renderer.Update();
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
		
		Rectangle activeClip;
		if (!GetIntersectionRect(window, activeClip))
			return;

		// 1. Obtenemos el rectángulo absoluto real para las coordenadas
		auto absolutePosition = GUI::GetWindowRootPosition(window);
		Rectangle absoluteParentRect{ absolutePosition.X, absolutePosition.Y, window->ClientSize.Width, window->ClientSize.Height };

		// Pasamos ambos de forma independiente
		MapInternal(window, processChildren, absoluteParentRect, activeClip);
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

	void UIRendererCoordinator::MapInternal(Window* window, bool processChildren, const Rectangle& absoluteParentRect, const Rectangle& activeClip)
	{
		auto& rootGraphics = *(window->RootGraphics);

		for (auto* child : window->Children)
		{
			if (!child->Visible) continue;

			if (child->IsNative())
			{
				Paint(child, (processChildren ? PaintOperation::TryUpdate : PaintOperation::None), processChildren);
				continue;
			}

			Rectangle childAbsoluteRect = absoluteParentRect;
			childAbsoluteRect.X += child->Position.X;
			childAbsoluteRect.Y += child->Position.Y;
			childAbsoluteRect.Width = child->ClientSize.Width;
			childAbsoluteRect.Height = child->ClientSize.Height;
        
			Rectangle childClip;
			// 2. CLIPPING: Comparamos contra el activeClip heredado
			if (LayoutUtils::GetIntersectionRect(childAbsoluteRect, activeClip, childClip))
			{
				rootGraphics.ResetTransform();
				
				// --- INICIO CLIP DEL CONTROL ---
				{
					ScopedClip controlClip(rootGraphics, childClip);

					if (child->Type != WindowType::Panel && processChildren && !child->Flags.isUpdating)
					{
						ScopedUpdatingFlag guard(child);
						child->Renderer.Update(); 
					}

					// --- INICIO CLIP DE LOS HIJOS (Box Model) ---
					Rectangle contentClip = childClip;
					if (!GUI::IsWindowBorderless(child))
					{
						contentClip.X += 1;
						contentClip.Y += 1;
						contentClip.Width -= 2;
						contentClip.Height -= 2;
					}

					Rectangle finalChildrenClip;
					if (LayoutUtils::GetIntersectionRect(contentClip, childClip, finalChildrenClip))
					{
						rootGraphics.ResetTransform();
                    
						// RECURSIÓN: Pasamos el childAbsoluteRect (coordenadas intactas) 
						// y el finalChildrenClip (tijera ajustada)
						MapInternal(child, processChildren, childAbsoluteRect, finalChildrenClip);
					}
				}
			}
		}
	}
}
