/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_D2D_HEADER
#define BT_D2D_HEADER

#ifdef BT_PLATFORM_WINDOWS

#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "dwrite")

namespace Berta::DirectX
{
	class D2DModule
	{
	public:
		D2DModule();
		
		ID2D1Factory* GetFactory() const
		{
			return m_factory;
		}

		IDWriteFactory* GetWriteFactory() const
		{
			return m_dWriteFactory;
		}
		static D2DModule& GetInstance()
		{
			static D2DModule d2dModule;
			return d2dModule;
		}

	private:
		ID2D1Factory* m_factory{ nullptr };
		IDWriteFactory* m_dWriteFactory{ nullptr };
	};

	//D2DHandle g_d2dHandle{};

	//void InitDirect2D();
}
#endif

#endif