/*
* MIT License
* 
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BASE_HEADER
#define BT_BASE_HEADER

constexpr auto BT_APPLICATION_DPI = 96.0f;

#include <string>

namespace Berta
{
	void InitializeCore();
	void ShutdownCore();
}

#include "Assert.h"

#define BT_DEFINITION_FLAG_FROM_ENUM(Flag) \
	inline constexpr Flag operator |(const Flag selfValue, const Flag inValue) \
	{ \
		return static_cast<Flag>(static_cast<uint8_t>(selfValue) | static_cast<uint8_t>(inValue)); \
	} \
	inline constexpr Flag operator &(const Flag selfValue, const Flag inValue) \
	{ \
		return static_cast<Flag>(static_cast<uint8_t>(selfValue) & static_cast<uint8_t>(inValue)); \
	} \
	inline constexpr Flag operator ~(const Flag selfValue) \
	{ \
		return static_cast<Flag>(~static_cast<uint8_t>(selfValue)); \
	} \
	inline bool HasFlag(Flag value, Flag flag) \
	{ \
		return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) != 0; \
	}

namespace Berta::StringUtils
{
	std::wstring Convert(const std::string& str);
}

namespace Berta::LayoutUtils
{
	float CalculateDPIScaleFactor(uint32_t dpi);
	float CalculateDownwardDPIScaleFactor(uint32_t dpi);
}

#endif