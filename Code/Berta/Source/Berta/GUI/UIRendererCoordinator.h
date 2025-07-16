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

		static void Paint(Window* window, PaintOperation operation, bool processChildren);
		static void Map(Window* window, bool haveUpdated, bool processChildren);

	private:
		static bool GetIntersectionRect(Window* window, Rectangle& result);
		static void MapInternal(Window* window, bool processChildren, const Rectangle& parentRect);
	};
}

#endif