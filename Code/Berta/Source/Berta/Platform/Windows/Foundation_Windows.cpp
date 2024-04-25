/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Berta/Core/Foundation.h"
#include "Berta/Core/Base.h"

#include "Berta/Core/Log.h"

namespace Berta
{
	Foundation Foundation::g_foundation;

	Foundation::Foundation()
	{
		InitializeCore();
		BT_CORE_TRACE << "Foundation init..." << std::endl;
	}

	Foundation::~Foundation()
	{
		BT_CORE_TRACE << "Releasing foundation..." << std::endl;
		ShutdownCore();
	}

	Foundation& Foundation::Instance()
	{
		return g_foundation;
	}
}