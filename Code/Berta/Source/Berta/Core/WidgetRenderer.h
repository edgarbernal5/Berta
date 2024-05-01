/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_RENDERER_HEADER
#define BT_RENDERER_HEADER

namespace Berta
{
	class WidgetRenderer
	{
	public:
		virtual ~WidgetRenderer() = default;

		virtual void Update();
	};
}

#endif