/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_MESSAGES_HEADER
#define BT_MESSAGES_HEADER

#include <windows.h>
#include <functional>

namespace Berta
{
	struct CustomCallbackMessage
	{
		std::function<void()> Body;
	};
	enum class CustomMessageId : uint32_t
	{
		CustomCallback = WM_USER + 1
	};
}

#endif