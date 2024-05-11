/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_COMMON_EVENTS_HEADER
#define BT_COMMON_EVENTS_HEADER

#include "Berta/Core/Event.h"
#include "Berta/Core/BasicTypes.h"

namespace Berta
{
	struct ArgMouseMove
	{
		Point Position;
		struct MouseButtonState
		{
			bool LeftButton : 1;
			bool RightButton : 1;
			bool MiddleButton : 1;
		}ButtonState;
	};

	struct CommonEvents
	{
		virtual ~CommonEvents() = default;
		Event<ArgMouseMove> MouseMove;
	};
}

#endif