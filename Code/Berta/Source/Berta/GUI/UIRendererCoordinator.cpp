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
	
	void UIRendererCoordinator::Paint(Window* window, const Rectangle* dirtyRect)
	{
		if (window->Flags.isUpdating)
		{
			return;
		}

		if (window->Type == WindowType::RenderForm)
		{
			return;
		}

		auto& rootGraphics = *(window->RootGraphics);
		rootGraphics.Begin();
		{
			rootGraphics.ResetTransform();
			ScopedClip controlClip(rootGraphics, *dirtyRect);
			if (window->Type != WindowType::Panel && window->Renderer.GetGraphics().IsValid())
			{
				ScopedUpdatingFlag guard(window);
			
				window->Renderer.Update();
			}
			Map(window, dirtyRect);
		}
		
		rootGraphics.Flush();
	}

	void UIRendererCoordinator::Map(Window* window, const Rectangle* dirtyRect)
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
		if (!GetIntersectionRect(window, activeClip, dirtyRect))
		{
			return;
		}

		// 1. Obtenemos el rectángulo absoluto real para las coordenadas
		auto absolutePosition = GUI::GetWindowRootPosition(window);
		Rectangle absoluteParentRect{ absolutePosition.X, absolutePosition.Y, window->ClientSize.Width, window->ClientSize.Height };

		// Pasamos ambos de forma independiente
		MapInternal(window, absoluteParentRect, activeClip);
	}

	bool UIRendererCoordinator::GetIntersectionRect(Window* window, Rectangle& result, const Rectangle* dirtyRect)
	{
		Rectangle requestRectangle = window->ClientSize.ToRectangle();
		auto absolutePosition = GUI::GetWindowRootPosition(window);
		requestRectangle.X = absolutePosition.X;
		requestRectangle.Y = absolutePosition.Y;

		auto container = window->FindFirstPanelOrFormAncestor();
		auto containerPosition = GUI::GetWindowRootPosition(container);
		Rectangle containerRectangle{ containerPosition.X, containerPosition.Y, container->ClientSize.Width, container->ClientSize.Height };

		// 1. Primera intersección: Ventana vs Contenedor
		Rectangle windowVisibleRect;
		if (!LayoutUtils::GetIntersectionRect(containerRectangle, requestRectangle, windowVisibleRect))
		{
			return false; // Está fuera de su panel, la descartamos.
		}

		// 2. LA MAGIA: Segunda intersección contra el área sucia de Win32
		if (dirtyRect != nullptr)
		{
			// Si no intersecta con el parche que hay que repintar, la descartamos.
			return LayoutUtils::GetIntersectionRect(windowVisibleRect, *dirtyRect, result);
		}

		// Si dirtyRect es nullptr (repintado completo), devolvemos la intersección normal.
		result = windowVisibleRect;
		return true;
	}

	void UIRendererCoordinator::MapInternal(Window* window, const Rectangle& absoluteParentRect, const Rectangle& activeClip)
	{
		auto& rootGraphics = *(window->RootGraphics);

		for (auto* child : window->Children)
		{
			if (!child->Visible)
			{
				continue;
			}
			
			if (child->IsNative())
			{
				// NO HACER Paint(child, ...);
				// Win32 se encargará de enviarle un WM_PAINT a ese HWND específicamente,
				// y tu WndProc lo atrapará e iniciará un ciclo de renderizado limpio para él solo.
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

					if (child->Type != WindowType::Panel && !child->Flags.isUpdating)
					{
						ScopedUpdatingFlag guard(child);
						child->Renderer.Update(); 
					}

					// --- INICIO CLIP DE LOS HIJOS (Box Model) ---
					// No encogemos el clip, encogemos el rectángulo absoluto del padre
					Rectangle parentContentAbsoluteRect = childAbsoluteRect;
					if (!GUI::IsWindowBorderless(child))
					{
						parentContentAbsoluteRect.X += 1;
						parentContentAbsoluteRect.Y += 1;
						parentContentAbsoluteRect.Width -= 2;
						parentContentAbsoluteRect.Height -= 2;
					}

					Rectangle finalChildrenClip;
					// Intersectamos el área de contenido real del padre con el clip que veníamos arrastrando
					if (LayoutUtils::GetIntersectionRect(parentContentAbsoluteRect, childClip, finalChildrenClip))
					{
						rootGraphics.ResetTransform();
        
						// RECURSIÓN: Pasamos el childAbsoluteRect (coordenadas intactas) 
						// y el finalChildrenClip (tijera ajustada)
						MapInternal(child, childAbsoluteRect, finalChildrenClip);
					}
				}
			}
		}
	}
}
