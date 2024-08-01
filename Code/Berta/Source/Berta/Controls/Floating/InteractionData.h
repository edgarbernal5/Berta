/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_INTERCTION_DATA_HEADER
#define BT_INTERCTION_DATA_HEADER

#include <string>
#include <vector>

namespace Berta::GUI
{
	struct InteractionData
	{
		std::vector<std::wstring> m_items;
		int m_selectedIndex{ -1 };
		bool m_isSelected{ false };

		size_t m_maxItemsToDisplay = 5;
	};
}

#endif