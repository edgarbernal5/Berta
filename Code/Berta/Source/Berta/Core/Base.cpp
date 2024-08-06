/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Base.h"

#define BERTA_BUILD_ID "v0.1"

namespace Berta
{
	void InitializeCore()
	{
		Log::Initialize();

		BT_CORE_TRACE << "Berta Framework " << BERTA_BUILD_ID << std::endl;
		BT_CORE_TRACE << "Initializing..." << std::endl;
	}

	void ShutdownCore()
	{
		BT_CORE_TRACE << "Shutting down... " << std::endl;

		Log::Shutdown();
	}
}

std::wstring Berta::StringUtils::Convert(const std::string& str)
{
	std::wstring wstr(str.begin(), str.end());
	return wstr;
}

float Berta::LayoutUtils::CalculateDPIScaleFactor(uint32_t dpi)
{
	return static_cast<float>(dpi / 96.0f);
}

float Berta::LayoutUtils::CalculateDownwardDPIScaleFactor(uint32_t dpi)
{
	return 96.0f / static_cast<float>(dpi);
}
