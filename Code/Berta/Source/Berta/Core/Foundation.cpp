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