#ifndef BERTA_ASSERT_HEADER
#define BERTA_ASSERT_HEADER

#include "Berta/Core/Log.h"

#if BT_PLATFORM_WINDOWS
	#define BT_DEBUG_BREAK __debugbreak()
#else
	#define BT_DEBUG_BREAK
#endif

#if BT_DEBUG
	#define BT_ASSERT(expression, ...) \
		if (expression){ } \
		else { \
			Berta::Log::PrintAssertMessage(__FILE__, __LINE__, "Assertion failed", __VA_ARGS__); \
			BT_DEBUG_BREAK; \
		}

	#define BT_ASSERT_ERROR(message) \
		{ \
			Berta::Log::PrintAssertMessage(__FILE__, __LINE__, "Assertion failed", message); \
			BT_DEBUG_BREAK; \
		}
#else
	#define BT_ASSERT(expression, ...)
	#define BT_ASSERT_ERROR(message)
#endif

#endif