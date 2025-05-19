/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_D2D_HEADER
#define BT_D2D_HEADER

#ifdef BT_PLATFORM_WINDOWS

#include <d2d1.h>

namespace Berta::DirectX
{
	struct D2DHandle
	{
		ID2D1Factory* m_factory{ nullptr };
	};

	class D2DModule
	{
	public:
		D2DModule();

		D2DHandle& Handle()
		{
			return m_handle;
		}
		
		static D2DModule& GetInstance()
		{
			static D2DModule d2dModule;
			return d2dModule;
		}
	private:
		D2DHandle m_handle;
	};

	//D2DHandle g_d2dHandle{};

	//void InitDirect2D();
}
#endif

#endif