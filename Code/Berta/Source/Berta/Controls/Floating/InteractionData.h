/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_INTERCTION_DATA_HEADER
#define BT_INTERCTION_DATA_HEADER

#include <optional>
#include <string>
#include <vector>
#include "Berta/Paint/Image.h"

namespace Berta::Float
{
	struct InteractionData
	{
		struct ItemType
		{
			std::wstring m_text;
			Image m_icon;
		};
		std::vector<ItemType> m_items;
		bool m_drawImages{ false };

		size_t m_maxItemsToDisplay = 5;
		std::optional<size_t> m_selectedIndex = std::nullopt;
		bool m_isSelected{ false };
	};
}

#endif