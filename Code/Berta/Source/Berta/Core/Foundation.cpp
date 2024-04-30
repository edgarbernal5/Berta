/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Foundation.h"

namespace Berta
{
	Foundation Foundation::g_foundation;

	Foundation& Foundation::GetInstance()
	{
		return g_foundation;
	}
}