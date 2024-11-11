/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_INTERCTION_DATA_HEADER
#define BT_INTERCTION_DATA_HEADER

#include <string>
#include <vector>
#include "Berta/Paint/Image.h"

namespace Berta::Float
{
	struct InteractionData
	{
		struct ItemType
		{
			std::wstring Text;
			Image Icon;
		};
		std::vector<ItemType> Items;
		bool DrawImages{ false };

		size_t MaxItemsToDisplay = 5;
		int m_selectedIndex{ -1 };
		bool m_isSelected{ false };
	};
}

#endif