/*
* MIT License
* 
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BASE_HEADER
#define BT_BASE_HEADER

constexpr auto BERTA_APPLICATION_DPI = 96.0f;

#include <string>

namespace Berta
{
	void InitializeCore();
	void ShutdownCore();
}

#include "Assert.h"

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