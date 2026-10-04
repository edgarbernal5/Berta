/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Helpers.h"

#include "LayoutNodes.h"

namespace Berta::Layouts
{
	Berta::Thickness GetScaledThickness(LayoutNode* node, const std::string& propName, float dpi)
	{
		if (auto* t = node->TryGetProperty<Berta::Thickness>(propName))
		{
			// Escalamos por el factor del monitor para soportar 4K / High DPI
			return Berta::Thickness(t->Left * dpi, t->Top * dpi, t->Right * dpi, t->Bottom * dpi);
		}
		return Berta::Thickness(0.0);
	}
}
