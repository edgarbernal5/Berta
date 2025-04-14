/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_DRAW_BATCH_HEADER
#define BT_DRAW_BATCH_HEADER

#include "Berta/Core/Base.h"
#include "Berta/Core/BasicTypes.h"
#include <vector>
#include <unordered_map>

namespace Berta
{
	struct Window;

	enum class DrawOperation : uint8_t
	{
		None = 0,
		NeedUpdate = 1,
		NeedMap = 2,
		Refresh = 4,
	};
	BT_DEFINITION_FLAG_FROM_ENUM(DrawOperation);

	struct BatchItem
	{
		Window* Target{ nullptr };
		Rectangle Area{};
		DrawOperation Operation{ DrawOperation::None };
		int Index{ 0 };
	};

	struct BatchItemComparer
	{
		bool operator()(BatchItem a, BatchItem b) const;
	};

	struct DrawBatcherContext
	{
		Window* m_rootWindow{ nullptr };
		std::vector<BatchItem> m_batchItemRequests;
	};

	class DrawBatch
	{
	public:
		DrawBatch(Window* rootWindow);
		~DrawBatch();

		void Clear();

		void AddWindow(Window* window, const Rectangle& areaToUpdate, const DrawOperation& operation);
		bool Exists(Window* window, const Rectangle& areaToUpdate);

	private:
		DrawBatcherContext& m_context;
		static std::unordered_map<Window*, DrawBatcherContext> g_contexts;
	};
}

#endif