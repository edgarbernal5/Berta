/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_UI_RENDERER_COORDINATOR_HEADER
#define BT_UI_RENDERER_COORDINATOR_HEADER

#include "Berta/GUI/Window.h"

namespace Berta
{
	class UIRendererCoordinator
	{
	public:
		enum class PaintOperation
		{
			None,
			HaveUpdated,
			TryUpdate
		};

		static void Paint(Window* window, const Rectangle* dirtyRect = nullptr);
		static void Map(Window* window, const Rectangle* dirtyRect);

	private:
		static bool GetIntersectionRect(Window* window, Rectangle& result, const Rectangle* dirtyRect);
		static void MapInternal(Window* window, const Rectangle& absoluteParentRect, const Rectangle& activeClip);
	};
}

#endif