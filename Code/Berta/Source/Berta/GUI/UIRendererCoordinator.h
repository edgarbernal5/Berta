/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_UI_RENDERER_COORDINATOR_HEADER
#define BT_UI_RENDERER_COORDINATOR_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/Paint/DrawBatch.h"

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
		static void TryAddWindowToBatch(Window* window, const DrawOperation& operation);
		static void TryAddWindowToBatchInternal(Window* window, const DrawOperation& operation, bool haveUpdated, bool processChildren, const Rectangle& parentRect);

		static void MapInternal(Window* window, bool haveUpdated, bool processChildren, const Rectangle& parentRect, Graphics& rootGraphics);

		static void AddWindowToBatch(Window* window, const Rectangle& areaToUpdate, const DrawOperation& operation);
		static void AddWindowToBatch(DrawBatch* batch, Window* window, const Rectangle& areaToUpdate, const DrawOperation& operation);
	};
}

#endif