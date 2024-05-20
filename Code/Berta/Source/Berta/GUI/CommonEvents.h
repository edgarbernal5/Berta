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
	struct ArgMouse
	{
		Point Position;
		struct MouseButtonState
		{
			bool LeftButton : 1;
			bool RightButton : 1;
			bool MiddleButton : 1;
		}ButtonState;
	};

	struct ArgClick
	{
	};

	struct ArgSize
	{
		Size NewSize;
	};

	struct ArgFocus
	{
		bool Focused;
	};

    struct ArgKeyboard
    {
        wchar_t Key;
        struct KeyboardState
        {
            bool Alt : 1;
            bool Ctrl : 1;
            bool Shift : 1;
        }ButtonState;
    };

	struct CommonEvents
	{
		virtual ~CommonEvents() = default;

		Event<ArgMouse>		MouseEnter;
		Event<ArgMouse>		MouseLeave;
		Event<ArgMouse>		MouseDown;
		Event<ArgMouse>		MouseMove;
		Event<ArgMouse>		MouseUp;
		Event<ArgClick>		Click;
		Event<ArgSize>		Size;
		Event<ArgFocus>		Focus;
		Event<ArgKeyboard>	KeyChar;
		Event<ArgKeyboard>	KeyPressed;
		Event<ArgKeyboard>	KeyReleased;
	};

	struct ArgSizeMove
	{
	};

	struct ArgActivated
	{
		bool IsActivated;
	};

	struct RootEvents : public CommonEvents
	{
		Event<ArgActivated>	Activated;
		Event<ArgSizeMove>	EnterSizeMove;
		Event<ArgSizeMove>	ExitSizeMove;
	};
}

#endif