/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_DRAW_BATCHER_HEADER
#define BT_DRAW_BATCHER_HEADER

#include "Berta/Core/BasicTypes.h"
#include <vector>
#include <unordered_map>

namespace Berta
{
	struct Window;

	struct WindowAreaBatch
	{
		Window* Target{ nullptr };
		Rectangle Area{};
		int Index{ 0 };
	};

	struct WindowAreaBatchComparer
	{
		bool operator()(WindowAreaBatch a, WindowAreaBatch b) const;
	};

	struct DrawBatcherContext
	{
		Window* m_rootWindow{ nullptr };
		std::vector<WindowAreaBatch> m_updateRequests;
	};

	class DrawBatch
	{
	public:
		DrawBatch(Window* rootWindow);
		~DrawBatch();

		void Clear();

		void AddWindow(Window* window, const Rectangle& areaToUpdate);
		bool Exists(Window* window, const Rectangle& areaToUpdate);

	private:
		DrawBatcherContext& m_context;
		static std::unordered_map<Window*, DrawBatcherContext> g_contexts;
	};
}

#endif