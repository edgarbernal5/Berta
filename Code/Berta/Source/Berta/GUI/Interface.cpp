#include "btpch.h"
#include "Interface.h"

#include "Berta/Core/Foundation.h"
#include "Berta/API/WindowAPI.h"

namespace Berta::GUI
{
	BasicWindow* CreateBasicWindow(const Rectangle& rectangle)
	{
		auto nativeHandle = API::CreateNativeWindow(rectangle);

		if (nativeHandle.Handle)
		{
			auto& windowManager = Foundation::GetInstance().GetWindowManager();
			BasicWindow* basicWindow = new BasicWindow();
			windowManager.AddWindow(basicWindow);
		}
		return nullptr;
	}
}