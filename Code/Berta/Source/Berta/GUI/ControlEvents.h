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
	struct MouseButtonState
	{
		bool LeftButton : 1;
		bool RightButton : 1;
		bool MiddleButton : 1;
	};

	struct ArgMouse
	{
		Point Position;
		MouseButtonState ButtonState;
	};

	struct ArgClick
	{
		MouseButtonState ButtonState;
	};

	struct ArgResize
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

	struct ArgDestroy
	{
	};

	struct ArgWheel
	{
		int WheelDelta;
		bool IsVertical;
	};

	struct ArgVisibility
	{
		bool IsVisible{ false };
	};

	struct ControlEvents
	{
		virtual ~ControlEvents() = default;

		Event<ArgMouse>			MouseEnter;
		Event<ArgMouse>			MouseLeave;
		Event<ArgMouse>			MouseDown;
		Event<ArgMouse>			MouseMove;
		Event<ArgWheel>			MouseWheel;
		Event<ArgMouse>			MouseUp;
		Event<ArgClick>			Click;
		Event<ArgClick>			DblClick;
		Event<ArgResize>		Resize;
		Event<ArgFocus>			Focus;
		Event<ArgKeyboard>		KeyChar;
		Event<ArgKeyboard>		KeyPressed;
		Event<ArgKeyboard>		KeyReleased;
		Event<ArgDestroy>		Destroy;
		Event<ArgVisibility>	Visibility;
	};

	struct ArgTextChanged
	{
		//ArgTextChanged(std::wstring& value):NewValue(value){}
		std::wstring NewValue;
	};

	struct InputTextEvents : public ControlEvents
	{
		Event<ArgTextChanged> ValueChanged;
	};

	struct ArgSizeMove
	{
	};

	struct ArgActivated
	{
		bool IsActivated;
	};

	struct ArgClosing
	{
		bool Cancel{ false };
	};

	struct FormEvents : public ControlEvents
	{
		Event<ArgActivated>	Activated;
		Event<ArgSizeMove>	EnterSizeMove;
		Event<ArgSizeMove>	ExitSizeMove;
		Event<ArgClosing>	Closing;
	};

	struct ArgComboBox
	{
		int SelectedIndex;
	};

	struct ComboboxEvents : public ControlEvents
	{
		Event<ArgComboBox>	Selected;
	};

	struct ArgScrollBar
	{
		int Value;
	};

	struct ScrollBarEvents : public ControlEvents
	{
		Event<ArgScrollBar>	ValueChanged;
	};

}

#endif