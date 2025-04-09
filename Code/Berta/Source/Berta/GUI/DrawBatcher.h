/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_DRAW_BATCHER_HEADER
#define BT_DRAW_BATCHER_HEADER

#include "Berta/Core/BasicTypes.h"
#include <vector>

namespace Berta
{
	struct Window;

	class DrawBatcher
	{
	public:
		DrawBatcher(Window* window);
		~DrawBatcher();

		void AddWindow(Window* window, const Rectangle& areaToUpdate);
		bool Exists(Window* window, const Rectangle& areaToUpdate);

	private:
		struct WindowArea
		{
			Window* Target{ nullptr };
			Rectangle Area{};
		};

		Window* m_rootWindow{ nullptr };
		std::vector<WindowArea> m_updateRequests;
	};
}

#endif