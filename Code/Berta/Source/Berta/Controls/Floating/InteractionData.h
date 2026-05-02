/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_INTERACTION_DATA_HEADER
#define BT_INTERACTION_DATA_HEADER

#include <any>
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
			std::any m_userData;
			
			ItemType(const std::wstring& text, const std::any& userData) : m_text{ text }, m_userData{ userData } {}
			ItemType(const std::wstring& text, const Image& icon, const std::any& userData) : m_text{ text }, m_icon{ icon }, m_userData{ userData } {}
		};
		
		std::vector<ItemType> m_items;
		bool m_drawImages{ false };

		size_t m_maxItemsToDisplay = 5;
		std::optional<size_t> m_selectedIndex = std::nullopt;
		bool m_isSelected{ false };
	};
}

#endif