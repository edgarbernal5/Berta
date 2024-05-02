/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_RENDERER_HEADER
#define BT_RENDERER_HEADER

#include "Berta/Paint/Graphics.h"

namespace Berta
{
	class WidgetRenderer
	{
	public:
		WidgetRenderer() = default;
		virtual ~WidgetRenderer() = default;

		virtual void Update();

	private:
		
	};
}

#endif