/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_DRAW_BATCH_ACTIVATOR
#define BT_DRAW_BATCH_ACTIVATOR

#include <unordered_map>

namespace Berta
{
	struct Window;

	struct DrawBatcherContext
	{
		Window* m_rootWindow{ nullptr };
	};

	class DrawBatchActivator
	{
	public:
		DrawBatchActivator(Window* rootWindow);
		~DrawBatchActivator();

	private:
		DrawBatcherContext& m_context;
		static std::unordered_map<Window*, DrawBatcherContext> g_contexts;
	};
}

#endif
