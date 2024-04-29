#include "btpch.h"
#include "Interface.h"

#include "Berta/API/WindowAPI.h"

namespace Berta::GUI
{
	BasicWindow* Create_Window(const Rectangle& rectangle)
	{
		auto nativeHandele = API::CreateNativeWindow(rectangle);

		return nullptr;
	}
}